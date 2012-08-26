OBJS="ata_identify.o ata_identify_parse.o libtest.a ../src/libscsi.a"
redo-ifchange $OBJS
gcc $(cat ../flags) -o $3 $OBJS
