STRUCT=../structs/ata_inquiry.xml
redo-ifchange $STRUCT ../tools/xml2survey.py
../tools/xml2survey.py < $STRUCT > $3
