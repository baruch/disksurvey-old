#ifndef _LIBSCSI_TEST_LIB_H
#define _LIBSCSI_TEST_LIB_H

#include <unistd.h>
#include <scsi/sg.h>

void hexdump(unsigned char *buf, int buf_len);
void dump_error(sg_io_hdr_t *hdr);

#endif
