#include <stdio.h>
#include "disk-survey.h"
#include "ata.h"
#include <memory.h>

static int cdb_ata_identify(unsigned char *cdb)
{
	int cdb_len = 12;
	memset(cdb, 0, cdb_len);
	cdb[0] = 0xA1;
	cdb[1] = 0x4<<1;
	cdb[2] = ata_passthrough_flags_2(0, 0, 1, 1, ATA_PT_LEN_SPEC_SECTOR_COUNT);
	cdb[4] = 1;
	cdb[9] = 0xEC;
	return cdb_len;
}

void survey_ata_identify(sg_t *sg, FILE *out)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[512];
	sg_io_hdr_t hdr;
	int cdb_len;

	fprintf(out, "<ata_identify>\n");
	cdb_len = cdb_ata_identify(cdb);
	if (cdb_execute(sg, cdb, cdb_len, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		ata_identify_parse(buf, out);
	}
	fprintf(out, "</ata_identify>\n");
}
