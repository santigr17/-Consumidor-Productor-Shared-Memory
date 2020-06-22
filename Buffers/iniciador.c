#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffercircular.h"
#include "mensaje.h"

int main(int argc, char *argv[])
{
	int ret = 0;
    unsigned int cantidad = 0;
	// char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr1 = SCB_OK;
	// mensaje carta;
	if(argc != 3){
		printf("Por favor indique nombre del buffer y cantidad de mensajes\n");
		printf("Argumento 0: [%s]\n", argv[0]);
        printf("Argumento 1: [%s]\n", argv[1]);
        printf("Argumento 2: [%s]\n", argv[2]);
        return(1);
	}
    //Se convierte el segundo argumento capturado a unsigned int
    cantidad = atoi(argv[2]);
	printf("Creando Buffer Cicular compartido: [%s]\n", argv[1]);
	scberr1 = crear_buffer(argv[1], cantidad, sizeof(mensaje), &ctx, &ret);
    //Se prueba la info del buffer creado:

        int err = 0;
	    int semlleno = 0;
    	int semvacio = 0;
    	int semcon_carrera = 0;
	    buffer_control inf;
	    errores scberr2;
    while(1){
    system("clear");
    /* errores get_info(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores, int *err);*/
    scberr2 = get_info_buffer(argv[1], &inf, &semlleno, &semvacio, &semcon_carrera, &err);
	//Se verifica algun error
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr2, err, 1);
	// se printea
	printf("Nombre: [%s]\n", argv[1]);
	printf("Cabeza..............: [%u]\n", inf.cabeza);
	printf("COLA................: [%u]\n", inf.cola);
	printf("Qtd.................: [%u]\n", inf.qtd);
	printf("Capaciad total......: [%u]\n", inf.capacidad);
	printf("Productores.........: [%u]\n", inf.productores);
	printf("Consumidores........: [%u]\n", inf.consumidores);
	printf("Semaforos..........: Espacios Ocupados [%d] | Espacios Vacios [%d]\n", semlleno, semvacio);
	printf("Request Context: %p  \n",&(ctx.ctrl));
    sleep(2);
    }



}
