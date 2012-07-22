#include "scsi.h"
#include "sg.h"
#include "disk-survey.h"

#include <memory.h>

static const char *dev_clock_to_str(device_clock_e dev_clock)
{
	switch (dev_clock) {
	case DEVICE_CLOCK_POWER_ON: return "POWER_ON";
	case DEVICE_CLOCK_RESERVED_1: return "RESERVED_1";
	case DEVICE_CLOCK_SET_TIMESTAMP: return "SET_TIMESTAMP";
	case DEVICE_CLOCK_UNKNOWN: return "UNKNOWN";
	case DEVICE_CLOCK_RESERVED_2: return "RESERVED_2";
	case DEVICE_CLOCK_RESERVED_3: return "RESERVED_3";
	case DEVICE_CLOCK_RESERVED_4: return "RESERVED_4";
	case DEVICE_CLOCK_RESERVED_5: return "RESERVED_5";
	}

	return "NA";
}

void survey_timestamp(sg_t *sg, FILE *out)
{
	unsigned char cdb[16];
	unsigned char sense[255];
	unsigned char buf[16*1024];
	sg_io_hdr_t hdr;

	memset(buf, 0, sizeof(buf));

	fprintf(out, "\t<timestamp>\n");
	cdb_report_timestamp(cdb, sizeof(buf));
	if (cdb_execute(sg, cdb, 12, buf, sizeof(buf), sense, sizeof(sense), &hdr, out)) {
		device_clock_e dev_clock;
		uint64_t ts_msec;

		fprintf(out, "\t\t<raw_all>%s</raw_all>\n", hex_encode(buf, sizeof(buf), 1));
		parse_report_timestamp(buf, sizeof(buf), &dev_clock, &ts_msec);
		fprintf(out, "\t\t<device_clock val=\"%d\">%s</device_clock>\n", dev_clock, dev_clock_to_str(dev_clock));
		fprintf(out, "\t\t<timestamp_msec>%llu</timestamp_msec>\n", (unsigned long long)ts_msec);
	}
	fprintf(out, "\t</timestamp>\n");
}
