OBJS="inq.o libtest.a ../src/libscsi.a"
redo-ifchange $OBJS
gcc -o $3 $OBJS
