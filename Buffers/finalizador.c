#include <stdio.h>

#include "buffercircular.h"
#include "mensaje.h"

int main(int argc, char *argv[])
{
	int err = 0;
	errores scberr;

	if(argc != 2){
		printf("Usage: %s [SEMAPHORE_NAME]\n", argv[0]);
		return(1);
	}

	scberr = destruir_buffer(argv[1], &err);
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, err, 1);

	return(0);
}