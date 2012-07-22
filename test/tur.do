OBJS="tur.o ../src/libscsi.a"
redo-ifchange $OBJS
gcc $(cat ../flags) -o $3 $OBJS
