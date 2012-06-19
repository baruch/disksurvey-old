#ifndef _LIBSCSI_SG_H
#define _LIBSCSI_SG_H

#include <stdbool.h>
#include <stdlib.h>
#include <scsi/sg.h>

typedef struct sg sg_t;

bool sg_open(sg_t *sg, const char *path);
void sg_close(sg_t *sg);
int sg_fd(sg_t *sg);
bool sg_submit(sg_t *sg, unsigned char *cdb, char cdb_len, int dxfer_dir, void *buf, unsigned int buf_len,
               unsigned char *sense, char sense_len, unsigned int timeout, int pack_id,
               void *usr_ptr);
bool sg_read(sg_t *sg, sg_io_hdr_t *hdr);

#include "sg_private.h"

#endif
