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

typedef enum {
	LLBA_NO = 0,
	LLBA_YES = 1,
} mode_llba_e;

typedef enum {
	DBD_NO = 0,
	DBD_YES = 1,
} mode_dbd_e;

typedef enum {
	MODE_PC_CURRENT = 0,
	MODE_PC_CHANGEABLE = 1,
	MODE_PC_DEFAULT = 2,
	MODE_PC_SAVED = 3,
} mode_pc_e;

typedef enum {
	PCV_NO = 0,
	PCV_YES = 1,
} diag_pcv_e;

void cdb_tur(unsigned char *cdb);
void cdb_inquiry(unsigned char *cdb, bool evpd, char page_code, uint16_t alloc_len);
void cdb_read_capacity_16(unsigned char *cdb, uint32_t alloc_len);
void cdb_read_16(unsigned char *cdb, uint64_t lba, uint32_t num_lbas, bool fua, bool dpo);
void cdb_read_10(unsigned char *cdb, uint32_t lba, uint16_t num_lbas, bool fua, bool dpo);
void cdb_log_sense(unsigned char *cdb, uint8_t page_code, uint8_t sub_page_code, uint16_t alloc_len);
void cdb_mode_sense_10(unsigned char *cdb, mode_llba_e llba, mode_dbd_e dbd, mode_pc_e, uint8_t page_code, uint8_t sub_page_code, uint16_t alloc_len);
void cdb_receive_diagnostics(unsigned char *cdb, diag_pcv_e pcv, uint8_t page_code, uint16_t alloc_len);

void cdb_ata_identify_device(unsigned char *cdb);

bool parse_inquiry(unsigned char *buf, unsigned buf_len, int *device_type, scsi_vendor_t vendor, scsi_model_t model, scsi_fw_revision_t revision, scsi_serial_t serial);
bool parse_read_capacity_16(unsigned char *buf, unsigned buf_len, uint64_t *num_lbas, uint32_t *block_size, int *protection_type, bool *protection_enabled);
bool parse_sense(unsigned char *buf, unsigned buf_len, bool *current, uint8_t *sense_key, uint8_t *asc, uint8_t *ascq);

#endif
