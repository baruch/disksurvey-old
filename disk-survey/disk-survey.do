OBJS="main.o survey_vpds.o survey_read_diagnostics.o survey_mode_pages.o common.o ../src/libscsi.a"
redo-ifchange $OBJS
gcc -o $3 $OBJS
