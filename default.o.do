redo-ifchange $2.c
DEP_FILE=$(dirname $2)/.$(basename $2).d
gcc -Wall -Werror -I$PWD/include -MD -MF $DEP_FILE -c -o $3 $2.c
read DEPS <$DEP_FILE
redo-ifchange ${DEPS#*:}
