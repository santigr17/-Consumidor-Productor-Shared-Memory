CFLAGS_OPTIMIZATION = -g
#CFLAGS_OPTIMIZATION = -O3
CFLAGS_VERSION = -std=c11
CFLAGS_WARNINGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter
CFLAGS_DEFINES = -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_POSIX_SOURCE=1 -D_DEFAULT_SOURCE=1 -D_GNU_SOURCE=1
CFLAGS = $(CFLAGS_OPTIMIZATION) $(CFLAGS_VERSION) $(CFLAGS_WARNINGS) $(CFLAGS_DEFINES)

# System shell utilities
CC = gcc
RM = rm -fr
CP = cp
AR = ar
RANLIB = ranlib
CPPCHECK = cppcheck

INCLUDEPATH = -I./
LIBS = -lscb -lpthread -lrt
LIBPATH = -L./

all: clean lbuffershared

lbuffershared:
	@echo
	@echo "=== libbuffershared ================="
	$(CC) -o buffershared.o -c buffershared.c -lpthread -lrt $(CFLAGS)
	$(AR) rc libbuffershared.a buffershared.o
	$(RANLIB) libbuffershared.a
	-$(RM) buffershared.o

sample: lbuffershared
	@echo
	@echo "=== Compilando =============="
	$(CC) -o productor productor.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o consumidor consumidor.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o mensaje mensaje.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o finalizador finalizador.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)

clean:
	@echo
	@echo "=== Limpiar_datos =============="
	-$(RM) prodoductor consumidor mensaje libbuffershared.a *.o core
