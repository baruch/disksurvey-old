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
	unsigned char buf[4096];

	if (!sg_open(&sg, argv[1])) {
		fprintf(stderr, "Error opening sg device '%s': %m\n", argv[1]);
		return 1;
	}

	cdb_log_sense(cdb, 0, 0, sizeof(buf));
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
		dump_error(&hdr);
	} else {
		printf("Reply received\n");

		hexdump(buf, sizeof(buf));
	}
Exit:
	sg_close(&sg);
	return 0;
}
