#ifndef _DISK_SURVEY_H
#define _DISK_SURVEY_H

#include <sg.h>
#include <stdio.h>

char *hex_encode(unsigned char *buf, size_t buf_len, int strip_zeros);
void survey_mode_pages(sg_t *sg, FILE *out);
void survey_vpds(sg_t *sg, FILE *out);
void survey_read_diagnostics(sg_t *sg, FILE *out);

int cdb_execute(sg_t *sg, unsigned char *cdb, size_t cdb_len, unsigned char *buf, size_t buf_len, unsigned char *sense, size_t sense_len,
                sg_io_hdr_t *hdr, FILE *out);

#endif
