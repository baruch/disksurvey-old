#include "disk-survey.h"
#include "scsi.h"

#include <string.h>

static inline char to_hex_char(char ch)
{
	if (ch >= 0 && ch <= 9)
		return '0' + ch;
	else
		return 'a' + ch - 10;
}

char *hex_encode(unsigned char *buf, size_t buf_len, int strip_zeros)
{
	static char hex[256*1024];
	int first_zero = 0;
	int j = 0;
	int i;

	memset(hex, 0, sizeof(hex));

	for (i = 0; i < buf_len; i++) {
		hex[j++] = to_hex_char((buf[i] >> 4) & 0xF);
		hex[j++] = to_hex_char(buf[i] & 0xF);

		if (buf[i])
			first_zero = j;
	}

	if (strip_zeros)
		hex[first_zero] = 0;
	else
		hex[j] = 0;

	return hex;
}

int cdb_execute(sg_t *sg, unsigned char *cdb, size_t cdb_len, unsigned char *buf, size_t buf_len, unsigned char *sense, size_t sense_len,
                sg_io_hdr_t *hdr, FILE *out)
{
	fprintf(out, "<cdb>%s</cdb>", hex_encode(cdb, cdb_len, 0));

	if (!sg_submit(sg, cdb, cdb_len, SG_DXFER_FROM_DEV, buf, buf_len, sense, sense_len, 30*1000, 0, NULL)) {
		fprintf(stderr, "Failed to submit cmd\n");
		return 0;
	}

	if (!sg_read(sg, hdr)) {
		fprintf(stderr, "Failed to read reply\n");
		return 0;
	}

	fprintf(out, "<reply");
	fprintf(out, " duration=\"%u\"", hdr->duration);
	fprintf(out, " status=\"%d\"", hdr->status);
	fprintf(out, " masked_status=\"%d\"", hdr->masked_status);
	fprintf(out, " msg_status=\"%d\"", hdr->msg_status);
	fprintf(out, " host_status=\"%d\"", hdr->host_status);
	fprintf(out, " driver_status=\"%d\"", hdr->driver_status);
	fprintf(out, " sense=\"%s\"", hex_encode(hdr->sbp, hdr->sb_len_wr, 0));
	fprintf(out, ">\n");

	if (hdr->status) {
		fprintf(stderr, "Reply is an error: %d\n", hdr->status);
		fprintf(stderr, "CDB: %s\n", hex_encode(cdb, cdb_len, 0));
		fprintf(stderr, "Sense: %s\n", hex_encode(hdr->sbp, hdr->sb_len_wr, 0));
		bool current;
		uint8_t sense_key, asc, ascq;
		parse_sense(hdr->sbp, hdr->sb_len_wr, &current, &sense_key, &asc, &ascq);
		fprintf(stderr, "Sense: %s/%02X/%02X/%02X\n", current ? "current" : "deferred", sense_key, asc, ascq);
		return 0;
	}

	return 1;
}
