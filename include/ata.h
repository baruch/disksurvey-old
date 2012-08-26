#ifndef _LIBSCSI_ATA_H
#define _LIBSCSI_ATA_H

#include <stdint.h>
#include <stdbool.h>

static inline uint16_t ata_get_word(unsigned char *buf, int word)
{
	uint16_t val = (uint16_t)(buf[word*2+1])<<8 | buf[word*2];
	return val;
}

static inline uint16_t ata_get_bits(unsigned char *buf, int word, int start_bit, int end_bit)
{
	uint16_t val = ata_get_word(buf, word);
	uint16_t shift = start_bit;
	uint16_t mask = 0;

	switch (end_bit - start_bit + 1) {
		case 1: mask = 1; break;
		case 2: mask = 3; break;
		case 3: mask = 7; break;
		case 4: mask = 0xF; break;
		case 5: mask = 0x1F; break;
		case 6: mask = 0x3F; break;
		case 7: mask = 0x7F; break;
		case 8: mask = 0xFF; break;
		case 9: mask = 0x1FF; break;
		case 10: mask = 0x3FF; break;
		case 11: mask = 0x7FF; break;
		case 12: mask = 0xFFF; break;
		case 13: mask = 0x1FFF; break;
		case 14: mask = 0x3FFF; break;
		case 15: mask = 0x7FFF; break;
		case 16: mask = 0xFFFF; break;
	}

	return (val >> shift) & mask;
}

static inline uint16_t ata_get_bit(unsigned char *buf, int word, int bit)
{
	return ata_get_bits(buf, word, bit, bit);
}

static inline unsigned char *ata_get_string(unsigned char *buf, int word_start, int word_end)
{
	static unsigned char str[128];
	int word;
	int i;

	/* Need to reverse the characters in the string as per "ATA string conventions" of ATA/ATAPI command set */
	for (i = 0, word = word_start; word <= word_end; word++) {
		str[i++] = buf[word*2+1];
		str[i++] = buf[word*2];
	}
	str[i] = 0;

	return str;
}

static inline uint32_t ata_get_longword(unsigned char *buf, int word)
{
	uint32_t longword = (uint32_t)ata_get_word(buf, word+1) << 16 | ata_get_word(buf, word);
	return longword;
}

#endif
