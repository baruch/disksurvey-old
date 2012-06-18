#include "scsi.h"
#include "sg.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
	sg_t sg;
	unsigned char sense[255];
	unsigned char tur[6];

	if (!sg_open(&sg, argv[1])) {
		fprintf(stderr, "Error opening sg device '%s': %m\n", argv[1]);
		return 1;
	}

	cdb_tur(tur);
	if (!sg_submit(&sg, tur, 6, SG_DXFER_NONE, NULL, 0, sense, sizeof(sense), 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit TUR\n");
		goto Exit;
	}

	sg_io_hdr_t hdr;
	if (!sg_read(&sg, &hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		goto Exit;
	}

	printf("Reply received\n");

Exit:
	sg_close(&sg);
	return 0;
}
