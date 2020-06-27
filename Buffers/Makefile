# C flags:
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
LIBS = -lbuffercircular -lpthread -lrt -lncurses -lm
LIBPATH = -L./ 


##Se crea la biblioteca 
lbuffercircular:
	@echo
	@echo "=== libbuffercircular ================="
	$(CC) -o buffercircular.o -c buffercircular.c -lpthread -lrt $(CFLAGS)
	$(AR) rc libbuffercircular.a buffercircular.o
	$(RANLIB) libbuffercircular.a
	-$(RM) buffercircular.o

##compilar prueba
sample: lbuffercircular
	@echo
	@echo "=== Compilando =============="
	$(CC) -o iniciador iniciador.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o productor productor.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o consumidor  consumidor.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)
	$(CC) -o finalizador  finalizador.c $(CFLAGS) $(INCLUDEPATH) $(LIBPATH) $(LIBS)


clean:
	@echo
	@echo "=== Limpiando datos =============="
	-$(RM) iniciador productor consumidor finalizador libbuffercircular.a *.o core