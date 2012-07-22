#ifndef _LIBSCSI_PARSE_H
#define _LIBSCSI_PARSE_H

static inline uint16_t get_uint16(unsigned char *buf, int start)
{
	uint16_t val;
	val = (uint16_t)buf[start] << 8 |
	      (uint16_t)buf[start+1];
	return val;
}

static inline uint32_t get_uint32(unsigned char *buf, int start)
{
	uint32_t val;
	val = (uint32_t)buf[start]   << 24 |
              (uint32_t)buf[start+1] << 16 |
	      (uint32_t)buf[start+2] << 8  |
	      (uint32_t)buf[start+3];
	return val;
}

static inline uint64_t get_uint64(unsigned char *buf, int start)
{
	uint64_t val;

	val = (uint64_t)buf[start]   << 56 |
              (uint64_t)buf[start+1] << 48 |
	      (uint64_t)buf[start+2] << 40 |
	      (uint64_t)buf[start+3] << 32 |
              (uint64_t)buf[start+4] << 24 |
	      (uint64_t)buf[start+5] << 16 |
	      (uint64_t)buf[start+6] <<  8 |
	      (uint64_t)buf[start+7];

	return val;
}

#endif
