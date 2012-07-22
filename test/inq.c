#include "scsi.h"
#include "sg.h"
#include "lib.h"

#include <scsi/scsi.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
	sg_t sg;
	unsigned char sense[255];
	unsigned char cdb[16];
	unsigned char buf[128];

	if (!sg_open(&sg, argv[1])) {
		fprintf(stderr, "Error opening sg device '%s': %m\n", argv[1]);
		return 1;
	}

	cdb_inquiry(cdb, false, 0, sizeof(buf));
	if (!sg_submit(&sg, cdb, 6, SG_DXFER_FROM_DEV, buf, sizeof(buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit INQUIRY\n");
		goto Exit;
	}

	sg_io_hdr_t hdr;
	if (!sg_read(&sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		goto Exit;
	}

	if (hdr.status) {
		printf("Reply received with error %d\n", hdr.status);
	} else {
		printf("Reply received\n");

		scsi_vendor_t vendor;
		scsi_model_t model;
		scsi_fw_revision_t revision;
		scsi_serial_t serial;
		int device_type;
		parse_inquiry(buf, sizeof(buf), &device_type, vendor, model, revision, serial);
		if (device_type != TYPE_DISK)
			printf("Device is not a disk (%d), bad device!!!\n", device_type);
		else
			printf("Device is a disk, phew!\n");
		printf("Vendor: '%s'\nModel: '%s'\nRevision: '%s'\nSerial: '%s'\n", vendor, model, revision, serial);

		hexdump(buf, sizeof(buf));
	}

	cdb_read_capacity_16(cdb, sizeof(buf));
	if (!sg_submit(&sg, cdb, 16, SG_DXFER_FROM_DEV, buf, sizeof(buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit read capacity 16\n");
		goto Exit;
	}

	if (!sg_read(&sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		goto Exit;
	}

	if (hdr.status) {
		printf("Reply received with error %d\n", hdr.status);
	} else {
		printf("Reply received\n");

		uint64_t num_lbas;
		uint32_t block_size;
		int prot_type;
		bool prot_en;
		parse_read_capacity_16(buf, sizeof(buf), &num_lbas, &block_size, &prot_type, &prot_en);
		printf("Num LBAs: %llu\nBlock Size: %u\nProtection Type: %d\nProtection Enabled: %s\n",
			(unsigned long long)num_lbas, block_size, prot_type, prot_en ? "Yes" : "No");
	}

Exit:
	sg_close(&sg);
	return 0;
}
