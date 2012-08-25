OBJS="ata_identify.o libtest.a ../src/libscsi.a"
redo-ifchange ../structs/ata_inquiry_parse.c.inc $OBJS
gcc $(cat ../flags) -o $3 $OBJS
