OBJ="cdb.o sg.o parse_misc.o"

redo-ifchange $OBJ
ar rs $3 $OBJ
