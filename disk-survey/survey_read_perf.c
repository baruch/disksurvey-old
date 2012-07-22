#include "scsi.h"
#include "sg.h"
#include "disk-survey.h"

#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>

typedef struct dev_stats {
	char sg_dev[256];
	char stats_filename[256];
} dev_stats_t;

static void sample_disk_stats(FILE *out, dev_stats_t *stats)
{
	char buf[8192];
	int fd;
	int ret;

	fd = open(stats->stats_filename, O_RDONLY);
	if (fd < 0) {
		fprintf(out, "<disk_stats_sample_failed errno=\"%d\"/>\n", errno);
		return;
	}

	ret = read(fd, buf, sizeof(buf)-1);
	if (ret < 0) {
		fprintf(out, "<disk_stats_sample_read_failed errno=\"%d\"/>\n", errno);
		goto Exit;
	}

	buf[ret] = 0;
	fprintf(out, "<disk_stats_sample>%s</disk_stats_sample>\n", buf);

Exit:
	close(fd);
}

static void do_perf_read(sg_t *sg, FILE *out, uint64_t lba, uint32_t block_size, uint32_t num_blocks)
{
	unsigned char cdb[16];
	unsigned char buf[block_size*num_blocks];
	unsigned char sense[128];
	sg_io_hdr_t hdr;

	fprintf(out, "\t\t\t<trace lba=\"%llu\" nblocks=\"%u\"\\>", (unsigned long long)lba, num_blocks);
	cdb_read_16(cdb, lba, num_blocks, true, false);
	cdb_execute(sg, cdb, 16, buf, sizeof(buf), sense, sizeof(sense), &hdr, out);
}

static void test_small_random(sg_t *sg, FILE *out, uint64_t num_blocks, uint32_t block_size, dev_stats_t *stats)
{
	struct drand48_data rand_buf;
	long int r;
	int i;
	
	memset(&rand_buf, 0, sizeof(rand_buf));

	fprintf(out, "\t\t<small_random>\n");
	sample_disk_stats(out, stats);
	for (i = 0; i < 1000; i++) {
		uint64_t lba;

		lrand48_r(&rand_buf, &r);
		lba = r % num_blocks;

		do_perf_read(sg, out, lba, block_size, 1);
	}
	sample_disk_stats(out, stats);
	fprintf(out, "\t\t</small_random>\n");
}

static void device_path_for_stats_sg(int sg_num, dev_stats_t *stats)
{
	// /sys/class/scsi_generic/sg0/device iorequest_cnt iodone_cnt ioerr_cnt
	snprintf(stats->sg_dev, sizeof(stats->sg_dev), "/sys/class/scsi_generic/sg%d/device", sg_num);

	// .. block/sda/stat
	char glob_pat[1024];
	snprintf(glob_pat, sizeof(glob_pat), "%s/block/*/stat", stats->sg_dev);

	glob_t globbuf;
	int ret = glob(glob_pat, 0, NULL, &globbuf);
	if (ret == 0) {
		strncpy(stats->stats_filename, globbuf.gl_pathv[0], sizeof(stats->stats_filename)-1);
	}
	globfree(&globbuf);
}

static void device_path_for_stats(const char *path, dev_stats_t *stats)
{
	int sg_num;
	int cnt;

	memset(stats, 0, sizeof(*stats));

	cnt = sscanf(path, "/dev/sg%d", &sg_num);
	if (cnt == 1) {
		device_path_for_stats_sg(sg_num, stats);
	}
}

void survey_read_performance(sg_t *sg, FILE *out, uint64_t num_blocks, uint32_t block_size, const char *path)
{
	dev_stats_t stats;

	device_path_for_stats(path, &stats);

	fprintf(out, "\t<read_performance>\n");
	test_small_random(sg, out, num_blocks, block_size, &stats);
	fflush(out); // flush between tests
	fprintf(out, "\t</read_performance>\n");
}
