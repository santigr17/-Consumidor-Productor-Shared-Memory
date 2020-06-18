#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "buffershared.h"
#include "mensaje.h"

int main(int argc, char *argv[])
{
	int ret = 0;
	unsigned int sec = 0;
	char buffersharederrormsg[SCB_ERRORMSG_MAXSZ + 1] = {'\0'};
	buffershared_t ctx;
	buffershared_err_t buffersharederr;
	element_t e;
	pid_t p;

	if(argc != 3){
		printf("Usando: %s [NOMBRE_SEMAFORO] [SEGUNDOS]\n", argv[0]);
		return(1);
	}

	sec = atoi(argv[2]);
	p = getpid();

	buffersharederr = buffershared_attach(&ctx, argv[1], &ret);
	if(buffersharederr != SCB_OK){
		buffershared_strerror(buffersharederr, ret, buffersharederrormsg);
		printf("%s", buffersharederrormsg);
		return(1);
	}

	while(1){
		memset(&e, 0, sizeof(element_t));

		buffersharederr = scb_get(&ctx, &e, copyElement, SCB_BLOCK);
		if(buffersharederr != SCB_OK){
			buffershared_strerror(buffersharederr, ret, buffersharederrormsg);
			printf("%s", buffersharederrormsg);
			return(1);
		}

		printf("PID [%d] obtuvo [%d][%f][%s]\n", p, e.a, e.b, e.c);

		sleep(sec);
	}

	/* scb_destroy(&ctx, &ret); */

	return(0);
}