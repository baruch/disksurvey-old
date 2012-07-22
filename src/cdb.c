#include "scsi.h"

#include <memory.h>
#include <stdint.h>

static inline void set_uint16(unsigned char *cdb, int start, uint16_t val)
{
	cdb[start] = (val >> 8) & 0xFF;
	cdb[start+1] = val & 0xFF;
}

static inline void set_uint32(unsigned char *cdb, int start, uint32_t val)
{
	cdb[start]   = (val >> 24) & 0xFF;
	cdb[start+1] = (val >> 16) & 0xFF;
	cdb[start+2] = (val >> 8) & 0xFF;
	cdb[start+3] = val & 0xFF;
}

static inline void set_uint64(unsigned char *cdb, int start, uint64_t val)
{
	cdb[start]   = (val >> 56) & 0xFF;
	cdb[start+1] = (val >> 48) & 0xFF;
	cdb[start+2] = (val >> 40) & 0xFF;
	cdb[start+3] = (val >> 32) & 0xFF;
	cdb[start+4] = (val >> 24) & 0xFF;
	cdb[start+5] = (val >> 16) & 0xFF;
	cdb[start+6] = (val >>  8) & 0xFF;
	cdb[start+7] = val & 0xFF;
}

void cdb_tur(unsigned char *cdb)
{
	memset(cdb, 0, 6);
}

void cdb_inquiry(unsigned char *cdb, bool evpd, char page_code, uint16_t alloc_len)
{
	cdb[0] = 0x12;
	cdb[1] = evpd ? 1 : 0;
	cdb[2] = page_code;
	set_uint16(cdb, 3, alloc_len);
	cdb[5] = 0;
}

void cdb_read_capacity_16(unsigned char *cdb, uint32_t alloc_len)
{
	memset(cdb, 0, 16);
	cdb[0] = 0x9E;
	cdb[1] = 0x10;
	set_uint32(cdb, 10, alloc_len);
}

void cdb_read_16(unsigned char *cdb, uint64_t lba, uint32_t num_lbas, bool fua, bool dpo)
{
	memset(cdb, 0, 16);
	cdb[0] = 0x88;
	cdb[1] = dpo<<4 | fua<<3;
	set_uint64(cdb, 2, lba);
	set_uint32(cdb, 10, num_lbas);
}

void cdb_read_10(unsigned char *cdb, uint32_t lba, uint16_t num_lbas, bool fua, bool dpo)
{
	memset(cdb, 0, 10);
	cdb[0] = 0x28;
	cdb[1] = dpo<<4 | fua<<3;
	set_uint32(cdb, 2, lba);
	set_uint16(cdb, 7, num_lbas);
}

void cdb_log_sense(unsigned char *cdb, uint8_t page_code, uint8_t sub_page_code, uint16_t alloc_len)
{
	memset(cdb, 0, 10);
	cdb[0] = 0x4D;
	cdb[2] = page_code;
	cdb[3] = sub_page_code;
	set_uint16(cdb, 7, alloc_len);
}

void cdb_mode_sense_10(unsigned char *cdb, mode_llba_e llba, mode_dbd_e dbd, mode_pc_e pc, uint8_t page_code, uint8_t sub_page_code, uint16_t alloc_len)
{
	cdb[0] = 0x5A;
	cdb[1] = llba << 4 | dbd << 3;
	cdb[2] = (pc << 6) | (page_code & 0x3F);
	cdb[3] = sub_page_code;
	cdb[4] = cdb[5] = cdb[6] = 0;
	set_uint16(cdb, 7, alloc_len);
	cdb[9] = 0;
}

void cdb_receive_diagnostics(unsigned char *cdb, diag_pcv_e pcv, uint8_t page_code, uint16_t alloc_len)
{
	cdb[0] = 0x1C;
	cdb[1] = pcv;
	cdb[2] = page_code;
	set_uint16(cdb, 3, alloc_len);
	cdb[5] = 0;
}

void cdb_report_timestamp(unsigned char *cdb, uint32_t alloc_len)
{
	memset(cdb, 0, 12);
	cdb[0] = 0xA3;
	cdb[1] = 0xF;
	set_uint32(cdb, 6, alloc_len);
}
