redo-ifchange $2.c flags
DEP_FILE=$(dirname $2)/.$(basename $2).d
gcc $(cat flags) -g -Wall -Werror -I$PWD/include -MD -MF $DEP_FILE -c -o $3 $2.c
read DEPS <$DEP_FILE
redo-ifchange ${DEPS#*:}
