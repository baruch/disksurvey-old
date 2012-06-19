#include "sg.h"
#include "scsi.h"

#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool sg_open(sg_t *sg, const char *path)
{
	memset(sg, 0, sizeof(*sg));
	sg->fd = open(path, O_RDWR);
	return sg->fd >= 0;
}

void sg_close(sg_t *sg)
{
	close(sg->fd);
}

int sg_fd(sg_t *sg)
{
	return sg->fd;
}

bool sg_submit(sg_t *sg, unsigned char *cdb, char cdb_len, int dxfer_dir, void *buf, unsigned int buf_len,
              unsigned char *sense, char sense_len, unsigned int timeout, int pack_id,
              void *usr_ptr)
{
	sg_io_hdr_t hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.interface_id = 'S';
	hdr.dxfer_direction = dxfer_dir;
	hdr.cmd_len = cdb_len;
	hdr.mx_sb_len = sense_len;
	hdr.dxfer_len = buf_len;
	hdr.dxferp = buf;
	hdr.cmdp = cdb;
	hdr.sbp = sense;
	hdr.timeout = timeout;
	hdr.flags = SG_FLAG_LUN_INHIBIT;
	hdr.pack_id = pack_id;
	hdr.usr_ptr = usr_ptr;
	
	ssize_t ret = write(sg->fd, &hdr, sizeof(hdr));
	return ret == sizeof(hdr);
}

bool sg_read(sg_t *sg, sg_io_hdr_t *hdr)
{
	return read(sg->fd, hdr, sizeof(*hdr)) == sizeof(*hdr);
}
