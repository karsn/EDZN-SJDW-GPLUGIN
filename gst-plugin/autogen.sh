#!/bin/sh
# you can either set the environment variables AUTOCONF, AUTOHEADER, AUTOMAKE,
# ACLOCAL, AUTOPOINT and/or LIBTOOLIZE to the right versions, or leave them
# unset and get the defaults

autoreconf --verbose --force --install --make || {
 echo 'autogen.sh failed';
 exit 1;
}

CFLAGS="-Wall -ggdb3"
CXXFLAGS="-Wall -ggdb3 -std=c++11 -I/home/wangsh/EDZN-SJDW-ZXING/zxing/out/include/" \
LDFLAGS=-L/home/wangsh/EDZN-SJDW-ZXING/zxing/out/ \
LIBS="-Wl,-Bstatic -lzxing -Wl,-Bdynamic -lpthread -lstdc++" \
./configure  || {
 echo 'configure failed';
 exit 1;
}

echo
echo "Now type 'make' to compile this module."
echo
