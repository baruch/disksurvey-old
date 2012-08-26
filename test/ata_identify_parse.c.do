STRUCT=../structs/ata_inquiry.xml

redo-ifchange $STRUCT ../tools/xmlstruct.py
../tools/xmlstruct.py < $STRUCT
