#ifndef _LIBSCSI_SCSI_H
#define _LIBSCSI_SCSI_H

#include <stdint.h>
#include <stdbool.h>

#define SCSI_VENDOR_LEN 8
#define SCSI_MODEL_LEN 16
#define SCSI_FW_REVISION_LEN 4
#define SCSI_SERIAL_LEN 8

typedef char scsi_vendor_t[SCSI_VENDOR_LEN+1];
typedef char scsi_model_t[SCSI_MODEL_LEN+1];
typedef char scsi_fw_revision_t[SCSI_FW_REVISION_LEN+1];
typedef char scsi_serial_t[SCSI_SERIAL_LEN+1];

void cdb_tur(unsigned char *cdb);
void cdb_inquiry(unsigned char *cdb, bool evpd, char page_code, uint16_t alloc_len);
void cdb_read_capacity_16(unsigned char *cdb, uint32_t alloc_len);
void cdb_read_16(unsigned char *cdb, uint64_t lba, uint32_t num_lbas, bool fua, bool dpo);
void cdb_read_10(unsigned char *cdb, uint32_t lba, uint16_t num_lbas, bool fua, bool dpo);

bool parse_inquiry(unsigned char *buf, unsigned buf_len, int *device_type, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial);
bool parse_read_capacity_16(unsigned char *buf, unsigned buf_len, uint64_t *num_lbas, uint32_t *block_size, int *protection_type, bool *protection_enabled);

#endif
