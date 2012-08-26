#include "scsi.h"
#include "sg.h"
#include "ata.h"
#include "lib.h"

#include <scsi/scsi.h>
#include <stdio.h>
#include <memory.h>

enum ata_passthrough_len_spec {
	ATA_PT_LEN_SPEC_NONE         = 0,
	ATA_PT_LEN_SPEC_FEATURES     = 1,
	ATA_PT_LEN_SPEC_SECTOR_COUNT = 2,
	ATA_PT_LEN_SPEC_TPSIU        = 3,
};

static inline unsigned char ata_passthrough_flags_2(int offline, int ck_cond, int direction_in, int transfer_block, enum ata_passthrough_len_spec len_spec)
{
	return ((offline & 3) << 6) | (ck_cond&1) | ((direction_in & 1) << 3) | ((transfer_block & 1) << 2) | (len_spec & 3);
}

#include "../structs/ata_inquiry_parse.c.inc"

static int inq_checksum(unsigned char *buf, int buf_len)
{
	char sum;
	int idx;

	if (buf[511] != 0xA5) {
		// Checksum isn't claimed to be valid, nothing to check here
		return 1;
	}

	for (idx = 0, sum = 0; idx < buf_len; idx++) {
		sum += buf[idx];
	}

	return sum == 0;
}

int main(int argc, const char **argv)
{
	sg_t sg;
	unsigned char sense[255];
	unsigned char cdb[16];
	unsigned char buf[512];
	int cdb_len;

	if (!sg_open(&sg, argv[1])) {
		fprintf(stderr, "Error opening sg device '%s': %m\n", argv[1]);
		return 1;
	}

	//cdb_len = cdb_ata_passthrough_12(cdb, false, 0, sizeof(buf));
	cdb_len = 12;
	memset(cdb, 0, cdb_len);
	cdb[0] = 0xA1;
	cdb[1] = 0x4<<1;
	cdb[2] = ata_passthrough_flags_2(0, 0, 1, 1, ATA_PT_LEN_SPEC_SECTOR_COUNT);
	cdb[4] = 1;
	cdb[9] = 0xEC;
	if (!sg_submit(&sg, cdb, cdb_len, SG_DXFER_FROM_DEV, buf, sizeof(buf), sense, sizeof(sense), 30*1000, 0, NULL)) {
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

		hexdump(buf, sizeof(buf));

		if (!inq_checksum(buf, 512)) {
			printf("checksum failed!\n");
		} else {
			printf("checksum passed!\n");
		}

		ata_identify_parse(buf);
	}

Exit:
	sg_close(&sg);
	return 0;
}
