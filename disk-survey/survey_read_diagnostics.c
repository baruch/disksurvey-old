#include "disk-survey.h"
#include "scsi.h"

#include <memory.h>

void survey_read_diagnostics(sg_t *sg, FILE *out)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[4096];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	cdb_receive_diagnostics(cdb, PCV_YES, 0, sizeof(buf));

	fprintf(out, "\t<receive_diagnostics>\n");
	if (cdb_execute(sg, cdb, 6, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		fprintf(out, "\t\t<raw_0>%s</raw_0>\n",
				hex_encode(buf, sizeof(buf), 1));
	}
	fprintf(out, "\t</receive_diagnostics>\n");
}
