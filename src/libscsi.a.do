OBJ="cdb.o sg.o parse_misc.o parse_log_sense.o"

redo-ifchange $OBJ
ar crs $3 $OBJ
