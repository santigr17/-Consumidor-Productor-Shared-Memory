#include <stdio.h>
#include "buffershared.h"

int main(int argc, char *argv[])
{
	int err = 0;
	char scberrormsg[BUFFERSHARED_ERRORMSG_MAXSZ + 1] = {'\0'};
	buffershared_ctrl_t inf;
	buffershared_err_t scberr;

	if(argc != 2){
		printf("Usando: %s [NOMBRE_SEMAFORO]\n", argv[0]);
		return(1);
	}

	scberr = scb_getInfo(argv[1], &inf, &err);
	if(scberr != SCB_OK){
		scb_strerror(scberr, err, scberrormsg);
		printf("%s", scberrormsg);
		return(1);
	}

	printf("Nombre del buffer.......: [%s]\n", argv[1]);
	printf("Cabeza..................: [%u]\n", inf.cabeza);
	printf("Cola ...................: [%u]\n", inf.cola);
	printf("Qtd.....................: [%u]\n", inf.qtd);
	printf("Capacidad total.........: [%u]\n", inf.dataTotal);
	printf("Tamaño elemento (bytes): [%lu]\n", inf.dataElementSz);

	return(0);
}
