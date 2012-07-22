OBJS="main.o survey_read_perf.o survey_vpds.o survey_timestamp.o survey_read_diagnostics.o survey_mode_pages.o common.o ../src/libscsi.a"
redo-ifchange $OBJS
gcc $(cat ../flags) -static -o $3 $OBJS
