#include "scsi.h"

#include <memory.h>
#include <stdint.h>

static inline void set_uint16(unsigned char *cdb, int start, uint16_t val)
{
	cdb[start] = (val >> 8) & 0xFF;
	cdb[start+1] = val & 0xFF;
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
