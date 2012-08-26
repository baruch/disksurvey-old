#!/usr/bin/python

import sys
from xml.dom.minidom import parse

def attributes_to_tuple(attrs):
	t = []

	for idx in xrange(attrs.length):
		attr = attrs.item(idx)
		t.append( (attr.name, attr.value) )
	return t

def handle_bit(word_idx, bit):
	name = bit.getAttribute('name')
	print 'fprintf(out, "<%s>%%d</%s>\\n", ata_get_bit(buf, %s, %s));' % (name, name, word_idx, bit.getAttribute('bit'))

def handle_val(word_idx, bit):
	name = bit.getAttribute('name')
	print 'fprintf(out, "<%s>%%d</%s>\\n", ata_get_bits(buf, %s, %s, %s));' % (name, name, word_idx, bit.getAttribute('bit_start'), bit.getAttribute('bit_end'))

def handle_word(word):
	word_idx = word.getAttribute('idx')

	for node in word.childNodes:
		if node.nodeType != node.ELEMENT_NODE:
			continue
		if node.nodeName == 'bit':
			handle_bit(word_idx, node)
		elif node.nodeName == 'val':
			handle_val(word_idx, node)
		else:
			raise 'unknown word node'

def handle_string(s):
	name = s.getAttribute('name')
	print 'fprintf(out, "<%s>%%s</%s>\\n", ata_get_string(buf, %s, %s));' % (name, name, s.getAttribute('word_start'), s.getAttribute('word_end'))

def handle_longword(word):
	name = word.getAttribute('name')
	print 'fprintf(out, "<%s>%%u</%s>\\n", ata_get_longword(buf, %s));' % (name, name, word.getAttribute('idx'))

def handle_struct(struct):
	if struct.nodeName != 'struct':
		print 'Unknown node', struct.nodeName
		return

	print 'void %s_parse(unsigned char *buf, FILE *out) {' % struct.attributes['name'].value

	for node in struct.childNodes:
		if node.nodeType != node.ELEMENT_NODE:
			continue
		#print '// ', node.nodeName, attributes_to_tuple(node.attributes)
		if node.nodeName == 'word':
			handle_word(node)
		elif node.nodeName == 'string':
			handle_string(node)
		elif node.nodeName == 'longword':
			handle_longword(node)
		else:
			raise 'unknown struct element %s' % node.nodeName

	print '}'

def print_header():
	print """#include <stdio.h>
#include "disk-survey.h"
#include "ata.h"
"""

def main():
	print_header()
	dom = parse(sys.stdin)
	handle_struct(dom.childNodes[0])

if __name__ == '__main__':
	main()
