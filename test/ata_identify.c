#include "scsi.h"
#include "sg.h"
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

static inline uint16_t ata_inq_word(unsigned char *buf, int word)
{
	uint16_t val = (uint16_t)(buf[word*2+1])<<8 | buf[word*2];
	return val;
}

static inline uint16_t ata_inq_bits(unsigned char *buf, int word, int start_bit, int end_bit)
{
	uint16_t val = ata_inq_word(buf, word);
	uint16_t shift = start_bit;
	uint16_t mask = 0;

	switch (end_bit - start_bit + 1) {
		case 1: mask = 1; break;
		case 2: mask = 3; break;
		case 3: mask = 7; break;
		case 4: mask = 0xF; break;
		case 5: mask = 0x1F; break;
		case 6: mask = 0x3F; break;
		case 7: mask = 0x7F; break;
		case 8: mask = 0xFF; break;
		case 9: mask = 0x1FF; break;
		case 10: mask = 0x3FF; break;
		case 11: mask = 0x7FF; break;
		case 12: mask = 0xFFF; break;
		case 13: mask = 0x1FFF; break;
		case 14: mask = 0x3FFF; break;
		case 15: mask = 0x7FFF; break;
		case 16: mask = 0xFFFF; break;
	}

	return (val >> shift) & mask;
}

static inline uint16_t ata_inq_bit(unsigned char *buf, int word, int bit)
{
	return ata_inq_bits(buf, word, bit, bit);
}

static inline unsigned char *ata_string(unsigned char *buf, int word_start, int word_end)
{
	static unsigned char str[128];
	int word;
	int i;

	/* Need to reverse the characters in the string as per "ATA string conventions" of ATA/ATAPI command set */
	for (i = 0, word = word_start; word <= word_end; word++) {
		str[i++] = buf[word*2+1];
		str[i++] = buf[word*2];
	}
	str[i] = 0;

	return str;
}

static inline uint32_t ata_longword(unsigned char *buf, int word)
{
	uint32_t longword = (uint32_t)ata_inq_word(buf, word+1) << 16 | ata_inq_word(buf, word);
	return longword;
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
