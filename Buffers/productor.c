#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "buffercircular.h"
#include "mensaje.h"
/**
 * Genera un mensaje.
 */
void generate_message(mensaje *msj)
{
	int magic;
	time_t time_created;
	pid_t pid;
	time(&time_created);
	/* Initializes random number generator */
	srand((unsigned)time(&time_created));
	magic = rand() % 7;
	pid = getpid();
	msj->numero_magico = magic;
	msj->pid = pid;
	msj->time = time_created;
}
//Función especial para finalizar
void finalizar()
{
	printf("Finalizando debido a variable global fue establecida como TRUE...\n");
}

int main(int argc, char *argv[])
{
	int err = 0;
	int ret = 0;
	// unsigned int sec = 0;
	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;

	check_error(scberr, ret, scberrormsgcreate);
	printf("%s", scberrormsgcreate);
	scberr = get_buffer(&ctx, argv[1], &err);
	scberr = add_productor(&ctx, argv[1], &err);

	// MEDICION de TIEMPOS
	double tiempoEspera, tiempoBloqueo, start, start2;

	//SE LIGA PRODUCTOR CON BUFFER USANDO EL ARGUMENTO PASADO POR CONSOLA
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, ret, 1);
	mensaje msj;
	while (1)
	{
		//se pide info del controller

		int err = 0;
		int semlleno = 0;
		int semvacio = 0;
		int semcon_carrera = 0;
		buffer_control inf;
		errores scberr2;

		/* errores get_info(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores, int *err);*/
		scberr2 = get_info_buffer(argv[1], &inf, &semlleno, &semvacio, &semcon_carrera, &err);
		//Se verifica algun error
		SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr2, err, 1);
		printf("Finalizador.........: [%u]\n", inf.finalizar);

		//Se tiene que verificar finalizador
		if (inf.finalizar)
		{
			finalizar();
			break;
		}
		buffer ctx;
		system("clear");
		//se refresca el buffer
		scberr = get_buffer(&ctx, argv[1], &err);

		//se crea un mensaje
		generate_message(&msj);

		struct tm *info;
		info = localtime(&(msj.time));
		printf("Soy un productor corriendo  PID: %i,Generé el número mágico: %i, a las : %s",
			   msj.pid,
			   msj.numero_magico,
			   asctime(info));

		start = clock();
		//Se intenta escribir
		scberr = put_msg(&ctx, &msj, copyMessage, UNBLOCK, &ret);
		tiempoBloqueo += clock() - start;
		sleep(5); // Val
		tiempoEspera += 5;
	}
}
