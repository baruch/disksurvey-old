#include "scsi.h"
#include "sg.h"

#include <scsi/scsi.h>
#include <stdio.h>
#include <alloca.h>
#include <memory.h>
#include <time.h>

typedef enum {
	READ_OK,
	READ_ERROR,
	READ_MEDIUM_ERR,
} read_scrub_code_e;

static bool do_inquiry(sg_t *sg, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial, int *dev_type)
{
	unsigned char cdb[16];
	unsigned char buf[256];
	unsigned char sense[255];
	sg_io_hdr_t hdr;

	cdb_inquiry(cdb, false, 0, sizeof(buf));
	if (!sg_submit(sg, cdb, 6, SG_DXFER_FROM_DEV, buf, sizeof(buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
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

	return parse_inquiry(buf, sizeof(buf), dev_type, vendor, model, revision, serial);
}

static bool do_read_capacity(sg_t *sg, uint64_t *num_blocks, uint32_t *block_size)
{
	unsigned char cdb[16];
	unsigned char buf[256];
	unsigned char sense[255];
	sg_io_hdr_t hdr;

	cdb_read_capacity_16(cdb, sizeof(buf));
	if (!sg_submit(sg, cdb, 16, SG_DXFER_FROM_DEV, buf, sizeof(buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit READ CAPACITY 16\n");
		return false;
	}

	if (!sg_read(sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		return false;
	}

	if (hdr.status) {
		fprintf(stderr, "Error while reading inquiry: %d\n", hdr.status);
		return false;
	}

	int prot_type;
	bool prot_en;
	return parse_read_capacity_16(buf, sizeof(buf), num_blocks, block_size, &prot_type, &prot_en);
}

static read_scrub_code_e do_scrub_read(sg_t *sg, uint64_t offset, uint32_t num_blocks, uint32_t block_size)
{
	unsigned char *buf = alloca(num_blocks * block_size);
	unsigned char cdb[16];
	unsigned char sense[255];
	sg_io_hdr_t hdr;

#if 0
	cdb_read_16(cdb, offset, num_blocks, false, true);
	if (!sg_submit(sg, cdb, 16, SG_DXFER_FROM_DEV, buf, num_blocks * block_size, sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit READ(16): %m\n");
		return READ_ERROR;
	}
#else
	cdb_read_10(cdb, offset, num_blocks, false, true);
	if (!sg_submit(sg, cdb, 10, SG_DXFER_FROM_DEV, buf, num_blocks * block_size, sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit READ(10): %m\n");
		return READ_ERROR;
	}
#endif

	if (!sg_read(sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		return READ_ERROR;
	}

	if (hdr.status) {
		fprintf(stderr, "Error while reading data: %d\n", hdr.status);
		return READ_ERROR;
	}

	return READ_OK;
}

typedef struct {
	time_t start_time;
	uint64_t io_operations;
} progress_t;

static void progress_init(progress_t *progress)
{
	memset(progress, 0, sizeof(*progress));
	progress->start_time = time(NULL);
}

static void progress_update(progress_t *progress, uint64_t current, uint64_t total)
{
	time_t now = time(NULL);
	time_t diff = now - progress->start_time;

	progress->io_operations++;
	uint64_t iops = progress->io_operations/(diff+1);

	double fcur = current;
	double ftotal = total;
	double percent = fcur * 100.0 / ftotal;
	double percent_per_sec = percent / (diff+1);
	time_t time_to_100 = now + (time_t)((100.0 - percent) / percent_per_sec);
	printf("\rProgress %3.1f%%: IOPS %3llu estimated end: %s\r", percent, (unsigned long long)iops, ctime(&time_to_100));
	fflush(stdout);
}

static void find_num_blocks_to_read(sg_t *sg, uint32_t *blocks_to_read, uint32_t block_size)
{
	read_scrub_code_e ret = READ_ERROR;

	while (ret != READ_OK && *blocks_to_read > 1) {
		printf("Trying to read %u blocks\n", *blocks_to_read);
		ret = do_scrub_read(sg, 0, *blocks_to_read, block_size);
		if (ret != READ_OK) {
			*blocks_to_read /= 2;
		}
	}
	printf("Settled on %u blocks to read in a request\n", *blocks_to_read);
}

static void do_scrub(sg_t *sg, uint64_t num_blocks, uint32_t block_size)
{
	uint64_t block_offset;
	uint32_t blocks_to_read = 1024*1024/block_size;
	progress_t progress;

	find_num_blocks_to_read(sg, &blocks_to_read, block_size);

	progress_init(&progress);
	for (block_offset = 0; block_offset < num_blocks; block_offset += blocks_to_read) {
		progress_update(&progress, block_offset, num_blocks);
		if (num_blocks - block_offset < blocks_to_read)
			blocks_to_read = num_blocks - block_offset;

		switch (do_scrub_read(sg, block_offset, blocks_to_read, block_size)) {
		case READ_OK: break;
		case READ_ERROR: fprintf(stderr, "Error while reading data\n"); return;
		case READ_MEDIUM_ERR:
			fprintf(stderr, "Medium error reading at offset %llu\n", (unsigned long long)block_offset);
			break;
		}
	}

	progress_update(&progress, num_blocks, num_blocks);
	printf("\n");
}

static void scrub_disk(const char *path)
{
	sg_t sg;
	int dev_type;
	scsi_vendor_t vendor;
	scsi_model_t model;
	scsi_fw_revision_t revision;
	scsi_serial_t serial;
	uint64_t num_blocks;
	uint32_t block_size;

	printf("Scrubbing disk %s\n", path);
	
	if (!sg_open(&sg, path)) {
		fprintf(stderr, "Error opening disk %s: %m\n", path);
		return;
	}

	if (!do_inquiry(&sg, vendor, model, revision, serial, &dev_type)) {
		fprintf(stderr, "Error while reading inquiry\n");
		goto Exit;
	}

	if (dev_type != TYPE_DISK) {
		fprintf(stderr, "Device is not a disk, scrubbing makes no sense\n");
		goto Exit;
	}

	if (!do_read_capacity(&sg, &num_blocks, &block_size)) {
		fprintf(stderr, "Error while reading capacity\n");
		goto Exit;
	}

	printf("Device attributes:\n"
	       "\tVendor: %s\n"
	       "\tModel: %s\n"
	       "\tRevision: %s\n"
	       "\tSerial: %s\n"
	       "\tNum blocks: %llu\n"
	       "\tBlock size: %u\n"
	       "\tSize: %llu GB\n"
	       , vendor, model, revision, serial, (unsigned long long)num_blocks, block_size, (unsigned long long)num_blocks*block_size/1000/1000/1000);

	do_scrub(&sg, num_blocks, block_size);

Exit:
	sg_close(&sg);
}

int main(int argc, char **argv)
{
	if (argc > 1)
		scrub_disk(argv[1]);
	else
		scrub_disk("/dev/sg0");
	return 0;
}
