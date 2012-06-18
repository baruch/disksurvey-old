OBJS="tur.o ../src/libscsi.a"
redo-ifchange $OBJS
gcc -o $3 $OBJS
