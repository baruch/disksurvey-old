#include "scsi.h"
#include "sg.h"

#include "disk-survey.h"

#include <scsi/scsi.h>
#include <stdio.h>
#include <string.h>

unsigned char inquiry_buf[256];

static char *strtrim(char *str)
{
	char *ptr = str + strlen(str) - 1;
	while (ptr >= str && *ptr == ' ') {
		*ptr = '\0';
		ptr--;
	}
	return str;
}

static bool disk_is_ata(scsi_vendor_t vendor)
{
	// According to the SCSI ATA Translating Standard, only "ATA" is expected,
	// In fact, other responses can be seen such as "SATA", so be all encompassing.
	return strstr(vendor, "ATA") != NULL;
}

static bool do_inquiry(sg_t *sg, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial, int *dev_type)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	sg_io_hdr_t hdr;

	memset(inquiry_buf, 0, sizeof(inquiry_buf));

	cdb_inquiry(cdb, false, 0, sizeof(inquiry_buf));
	if (!sg_submit(sg, cdb, 6, SG_DXFER_FROM_DEV, inquiry_buf, sizeof(inquiry_buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit INQUIRY\n");
		return false;
	}

	if (!sg_read(sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		return false;
	}

	if (hdr.status) {
		printf("Error while reading inquiry: %d\n", hdr.status);
		return false;
	}

	return parse_inquiry(inquiry_buf, sizeof(inquiry_buf), dev_type, vendor, model, revision, serial);
}

static void survey_disk_capacity(sg_t *sg, FILE *out, uint64_t *num_blocks, uint32_t *block_size)
{
	unsigned char cdb[16];
	unsigned char buf[256];
	unsigned char sense[255];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	fprintf(out, "\t<capacity>\n");

	cdb_read_capacity_16(cdb, sizeof(buf));
	if (cdb_execute(sg, cdb, 16, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		int prot_type;
		bool prot_en;
		parse_read_capacity_16(buf, sizeof(buf), num_blocks, block_size, &prot_type, &prot_en);
		fprintf(out, "\t\t<num_blocks>%llu</num_blocks>\n"
			     "\t\t<block_size>%u</block_size>\n"
			     "\t\t<prot_type>%d</prot_type>\n"
			     "\t\t<prot_enable>%d</prot_enable>\n"
			     "\t\t<raw>%s</raw>\n"
			     , (unsigned long long)*num_blocks, *block_size, prot_type, prot_en, hex_encode(buf, sizeof(buf), 1));
	}

	fprintf(out, "\t</capacity>\n");
}

static void survey_disk(const char *path)
{
	sg_t sg;
	FILE *out = NULL;

	printf("Scrubbing disk %s\n", path);
	
	if (!sg_open(&sg, path)) {
		fprintf(stderr, "Error opening disk %s: %m\n", path);
		return;
	}

	int dev_type;
	scsi_vendor_t vendor;
	scsi_model_t model;
	scsi_fw_revision_t revision;
	scsi_serial_t serial;
	if (!do_inquiry(&sg, vendor, model, revision, serial, &dev_type)) {
		fprintf(stderr, "Error while reading inquiry\n");
		goto Exit;
	}

	if (dev_type != TYPE_DISK) {
		fprintf(stderr, "Device is not a disk (%d), bailing out.\n", dev_type);
		goto Exit;
	}

	char filename[256];
	snprintf(filename, sizeof(filename), "disk-survey-%s-%s-%s.xml", strtrim(vendor), strtrim(model), strtrim(serial));
	out = fopen(filename, "w");
	if (!out) {
		fprintf(stderr, "Failed to open log file '%s' for output\n", filename);
		goto Exit;
	}

	char largebuf[1024*1024];
	setbuffer(out, largebuf, sizeof(largebuf));

	fprintf(out, "<survey>\n"
		     "\t<inquiry>\n"
	             "\t\t<vendor>%s</vendor>\n"
		     "\t\t<model>%s</model>\n"
		     "\t\t<revision>%s</revision>\n"
		     "\t\t<serial>%s</serial>\n"
		     "\t\t<raw>%s</raw>\n"
		     "\t</inquiry>\n"
		     , vendor, model, revision, serial, hex_encode(inquiry_buf, sizeof(inquiry_buf), 1));

	uint64_t num_blocks;
	uint32_t block_size;
	survey_disk_capacity(&sg, out, &num_blocks, &block_size);
	survey_vpds(&sg, out);
	survey_timestamp(&sg, out);
	survey_mode_pages(&sg, out);
	survey_read_diagnostics(&sg, out);

	if (disk_is_ata(vendor)) {
		survey_ata_identify(&sg, out);
	}

	// It woud be preferable to have no other IO during the read
	// performance test, so flush the pending output buffer to keep all of
	// the space for the next test
	fflush(out);
	survey_read_performance(&sg, out, num_blocks, block_size, path);

Exit:
	if (out) {
		fprintf(out, "</survey>\n");
		fclose(out);
	}
	sg_close(&sg);
}

int main(int argc, char **argv)
{
	if (argc > 1)
		survey_disk(argv[1]);
	else {
		printf("Defaulting to /dev/sg0\n");
		survey_disk("/dev/sg0");
	}
	return 0;
}
