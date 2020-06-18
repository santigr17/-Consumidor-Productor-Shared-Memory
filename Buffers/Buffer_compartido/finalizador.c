#include <stdio.h>

#include "buffershared.h"

int main(int argc, char *argv[])
{
	int err = 0;
	char buffersharederrormsg[BUFFERSHARED_ERRORMSG_MAXSZ + 1] = {'\0'};
	buffershared_err_t buffersharederr;

	if(argc != 2){
		printf("Usando: %s [NOMBRE_SEMAFORO]\n", argv[0]);
		return(1);
	}

	buffersharederr = buffershared_destroy(argv[1], &err);

	if(buffersharederr != SCB_OK){
		buffershared_strerror(buffersharederr, err, buffersharederrormsg);
		printf("%s", buffersharederrormsg);
		return(1);
	}

	return(0);
}
