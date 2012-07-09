#include "scsi.h"
#include "parse.h"

#include <string.h>
#include <stdio.h>

bool parse_inquiry(unsigned char *buf, unsigned buf_len, int *device_type, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial)
{
	*device_type = -1;
	vendor[0] = 0;
	model[0] = 0;
	revision[0] = 0;
	serial[0] = 0;

	if (buf_len < 44)
		return false;

	unsigned char fmt = buf[3] & 0xf; 
	if (fmt != 2 && fmt != 1) {
		fprintf(stderr, "Data not in standard format (%d but expected 2)\n", fmt);
		return false;
	}

	int valid_len = buf[4] + 4;

	*device_type = buf[0] & 0x1f;

	if (valid_len >= 8 + SCSI_VENDOR_LEN) {
		strncpy(vendor, (char*)buf+8, SCSI_VENDOR_LEN);
		vendor[SCSI_VENDOR_LEN] = 0;
	}

	if (valid_len >= 16 + SCSI_MODEL_LEN) {
		strncpy(model, (char*)buf+16, SCSI_MODEL_LEN);
		model[SCSI_MODEL_LEN] = 0;
	}

	if (valid_len >= 32 + SCSI_FW_REVISION_LEN) {
		strncpy(revision, (char*)buf+32, SCSI_FW_REVISION_LEN);
		revision[SCSI_FW_REVISION_LEN] = 0;
	}

	if (valid_len >= 44 && fmt == 2) {
		strncpy(serial, (char*)buf+36, SCSI_SERIAL_LEN);
		serial[SCSI_SERIAL_LEN] = 0;
	}

	return true;
}

bool parse_read_capacity_16(unsigned char *buf, unsigned buf_len, uint64_t *num_lbas, uint32_t *block_size, int *protection_type, bool *protection_enabled)
{
	if (buf_len < 32)
		return false;

	*num_lbas = get_uint64(buf, 0);	
	*block_size = get_uint32(buf, 8);
	*protection_type = (buf[12] >> 1) & 0x7;
	*protection_enabled = buf[12] & 1;

	return true;
}

bool parse_sense(unsigned char *buf, unsigned buf_len, bool *current, uint8_t *sense_key, uint8_t *asc, uint8_t *ascq)
{
	uint8_t resp_code = buf[0] & 0x7F;
	*current = resp_code == 0x70 || resp_code == 0x72;
	if (resp_code == 0x70 || resp_code == 0x71) {
		*sense_key = buf[2] & 0xF;
		*asc = buf[12];
		*ascq = buf[13];
	} else {
		*sense_key = *asc = *ascq = 0xff;
		return false;
	}
	return true;
}
