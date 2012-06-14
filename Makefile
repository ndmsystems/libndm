LIB = libndm.so
OBJS = iface.o feedback.o log.o parse.o socket.o time.o utils.o

CFLAGS += -Iinclude
all: ${LIB}

${LIB}: ${OBJS}
	${CC} ${CFLAGS } -shared -o $@ $^

clean:
	-rm *.o ${LIB}
