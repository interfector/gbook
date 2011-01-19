XML2LIB = -lxml2
XML2INC = /usr/include/libxml2

INCS = -I/usr/include -I${XML2INC} -Iinclude
LIBS = ${XML2LIB}

CFLAGS = ${INCS}
LDFLAGS = -s ${LIBS}

all:
	gcc -o gbook src/main.c src/xml.c ${INCS} ${LIBS}
install:
	cp gbook /usr/local/bin/
clean:
	rm gbook
