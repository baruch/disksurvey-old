#include "scsi.h"
#include "sg.h"
#include "disk-survey.h"

#include <memory.h>

void survey_mode_pages(sg_t *sg, FILE *out)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[16*1024];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	fprintf(out, "\t<mode_pages>\n");
	cdb_mode_sense_10(cdb, LLBA_YES, DBD_YES, MODE_PC_CURRENT, 0x3F, 0xFF, sizeof(buf));
	if (cdb_execute(sg, cdb, 6, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		fprintf(out, "\t\t<raw_all>%s</raw_all>\n", hex_encode(buf, sizeof(buf), 1));
	}
	fprintf(out, "\t</mode_pages>\n");
}
