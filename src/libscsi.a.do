OBJ="cdb.o sg.o parse_misc.o"

redo-ifchange $OBJ
ar crs $3 $OBJ
