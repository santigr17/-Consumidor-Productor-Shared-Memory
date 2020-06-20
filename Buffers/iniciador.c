#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffercircular.h"
#include "mensaje.h"

int main(int argc, char *argv[])
{
	int ret = 0;

	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;
	mensaje carta;

	if(argc != 3){
		printf("Usage: %s [SEMAPHORE_NAME] [SECONDS]\n", argv[0]);
		return(1);
	}


	printf("Creando Buffer Cicular compartido: [%s]\n", argv[1]);
	scberr = crear_buffer(argv[1], argv[2], sizeof(mensaje), &ctx, &ret);

    //Se prueba la info del buffer creado:


    while(1){
        int err = 0;
	    int semlleno = 0;
    	int semvacio = 0;
        int semconsumidores = 0;
        int semproductores = 0;
    	int semcon_carrera = 0;
	    buffer_control inf;
	    errores scberr;

	if(argc != 2){
		printf("Usage: %s [SEMAPHORE_NAME]\n", argv[0]);
		return(1);
	}

	scberr = (argv[1], &inf, &semlleno, &semvacio, &semcon_carrera,&semconsumidores,&semproductores, &err);
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, err, 1);

	printf("Circular buffer name: [%s]\n", argv[1]);
	printf("Cabeza................: [%u]\n", inf.cabeza);
	printf("COLA................: [%u]\n", inf.cola);
	printf("Qtd.................: [%u]\n", inf.qtd);
	printf("Capaciad total......: [%u]\n", inf.capacidad);
	printf("Tamaño de mensaje: [%lu]\n", inf.largo_mensaje);
	printf("Semaforos..........: Lleno [%d] | Vacio [%d] | Con_carrera [%d] | Consumidores [%d] | Productores [%d]\n", semlleno, semvacio, semcon_carrera,semconsumidores,semproductores);

    }



}