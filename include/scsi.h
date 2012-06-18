#ifndef _LIBSCSI_SCSI_H
#define _LIBSCSI_SCSI_H

#include <stdint.h>
#include <stdbool.h>

void cdb_tur(unsigned char *cdb);
void cdb_inquiry(unsigned char *cdb, bool evpd, char page_code, uint16_t alloc_len);

#endif
