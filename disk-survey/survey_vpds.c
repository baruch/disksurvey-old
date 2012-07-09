#include "disk-survey.h"
#include "scsi.h"

#include <alloca.h>
#include <memory.h>

static void parse_vpd_ascii(FILE *out, unsigned char *buf, size_t buf_len)
{
	char data[buf[4]+1];
	memcpy(data, buf+5, buf[4]);
	data[buf[4]] = 0;
	fprintf(out, "\t\t\t<ascii>%s</ascii>\n", data);
}

static void parse_vpd_serial(FILE *out, unsigned char *buf, size_t buf_len)
{
	unsigned char *start = buf+4;
	while (start && *start == ' ')
		start++;
	fprintf(out, "\t\t\t<serial>%s</serial>\n", start);
}

static void parse_vpd_block_dev_characteristics(FILE *out, unsigned char *buf, size_t buf_len)
{
	if (buf[1] != 0xB1) {
		fprintf(stderr, "Failed to parse VPD page 0xB1\n");
		return;
	}

	uint16_t rotate_rate = buf[4]<<8 | buf[5];
	if (rotate_rate == 0x0001)
		fprintf(out, "\t\t\t<non_rotating/>\n");
	else if (rotate_rate >= 0x0401 && rotate_rate <= 0xFFFE)
		fprintf(out, "\t\t\t<rotate_rate>%d</rotate_rate>\n", rotate_rate);

	static const char *form_factors[16] = {
		[1] = "5.25",
		[2] = "3.5",
		[3] = "2.5",
		[4] = "1.8",
		[5] = "smaller than 1.8",
	};

	const char *form_factor = form_factors[buf[7] & 0xF];
	if (form_factor)
		fprintf(out, "\t\t\t<form_factor>%s</form_factor>\n", form_factor);
}

typedef void (*parse_buf_func)(FILE *out, unsigned char *buf, size_t buf_len);
static parse_buf_func vpd_funcs[256] = {
	[0x80] = parse_vpd_serial,
	[0xB1] = parse_vpd_block_dev_characteristics,
};

static void survey_vpd(sg_t *sg, FILE *out, uint8_t page)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[4096];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	cdb_inquiry(cdb, true, page, sizeof(buf));

	fprintf(out, "\t\t<vpd_%u>\n", page);
	if (cdb_execute(sg, cdb, 6, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		if (vpd_funcs[page])
			vpd_funcs[page](out, buf, sizeof(buf));
		else if (page >= 0x01 && page <= 0x7F)
			parse_vpd_ascii(out, buf, sizeof(buf));
		fprintf(out, "\t\t\t<raw>%s</raw>\n", hex_encode(buf, sizeof(buf), 1));
	}
	fprintf(out, "\t\t</vpd_%u>\n", page);
}

void survey_vpds(sg_t *sg, FILE *out)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[256];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	cdb_inquiry(cdb, true, 0, sizeof(buf));

	fprintf(out, "\t<vpds>\n");
	fprintf(out, "\t\t<vpd_0>\n");
	if (cdb_execute(sg, cdb, 6, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {

		int i;
		for (i = 0; i < buf[3]; i++) {
			fprintf(out, "\t\t\t<supported_page>%d</supported_page>\n", buf[4+i]);
		}

		fprintf(out, "\t\t\t<raw>%s</raw>\n"
				"\t\t</vpd_0>\n",
				hex_encode(buf, sizeof(buf), 1));

		for (i = 0; i < buf[3]; i++) {
			if (buf[4+i]) // Skip page 0
				survey_vpd(sg, out, buf[4+i]);
		}
	} else {
		fprintf(out, "\t\t</vpd_0>\n");
	}

	fprintf(out, "\t</vpds>\n");
}
