#include "scsi.h"

#include <string.h>

static inline uint32_t get_uint32(unsigned char *buf, int start)
{
	uint32_t val;
	val = (uint32_t)buf[start]   << 24 |
              (uint32_t)buf[start+1] << 16 |
	      (uint32_t)buf[start+2] << 8  |
	      (uint32_t)buf[start+3];
	return val;
}

static inline uint64_t get_uint64(unsigned char *buf, int start)
{
	uint64_t val;

	val = (uint64_t)buf[start]   << 56 |
              (uint64_t)buf[start+1] << 48 |
	      (uint64_t)buf[start+2] << 40 |
	      (uint64_t)buf[start+3] << 32 |
              (uint64_t)buf[start+4] << 24 |
	      (uint64_t)buf[start+5] << 16 |
	      (uint64_t)buf[start+6] <<  8 |
	      (uint64_t)buf[start+7];

	return val;
}

bool parse_inquiry(unsigned char *buf, unsigned buf_len, int *device_type, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial)
{
	if (buf_len < 44)
		return false;

	if ((buf[3] & 0xf) != 2) {
		// Data not in standard format
		return false;
	}

	int valid_len = buf[4] + 4;
	if (valid_len < 44)
		return false;

	*device_type = buf[0] & 0x1f;

	strncpy(vendor, (char*)buf+8, SCSI_VENDOR_LEN);
	vendor[SCSI_VENDOR_LEN] = 0;

	strncpy(model, (char*)buf+16, SCSI_MODEL_LEN);
	model[SCSI_MODEL_LEN] = 0;

	strncpy(revision, (char*)buf+32, SCSI_FW_REVISION_LEN);
	revision[SCSI_FW_REVISION_LEN] = 0;

	strncpy(serial, (char*)buf+36, SCSI_SERIAL_LEN);
	serial[SCSI_SERIAL_LEN] = 0;

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
