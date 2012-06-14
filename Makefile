LIB = libndm.so
OBJS = iface.o log.o feedback.o parse.o socket.o time.o utils.o

CFLAGS += -Iinclude -fPIC
all: ${LIB}

${LIB}: ${OBJS}
	${CC} ${CFLAGS } -shared -o $@ $^

clean:
	-rm *.o ${LIB}
