#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "buffercircular.h"
#include "mensaje.h"
#include "colors.h"
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
void finalizar(int sentMessages, double tiempoBloqueo, double tiempoEspera, pid_t pid)
{
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	printf(ANSI_RED_BACKGROUND "Finalizando %d debido a variable global fue establecida como TRUE..." ANSI_COLOR_RESET "\n", pid);
	printf("Mensajes enviados........." ANSI_GRAY_BACKGROUND
		   " %d " ANSI_COLOR_RESET
		   "\n",
		   sentMessages);
	printf("Tiempo total bloqueado...." ANSI_GRAY_BACKGROUND
		   " %g " ANSI_COLOR_RESET
		   "\n",
		   tiempoBloqueo);
	printf("Tiempo total de espera...." ANSI_GRAY_BACKGROUND
		   " %g " ANSI_COLOR_RESET
		   "\n",
		   tiempoEspera);
	printf("Tiempo de CPU:............" ANSI_GRAY_BACKGROUND
		   " %ld.%06ld  " ANSI_COLOR_RESET
		   " segundos en USER. "
		   "\n",
		   (long int)usage.ru_utime.tv_sec, (long int)usage.ru_utime.tv_usec);

	printf("Tiempo de CPU:............" ANSI_GRAY_BACKGROUND
		   " %ld.%06ld  " ANSI_COLOR_RESET
		   " segundos en KERNEL. "
		   "\n",
		   (long int)usage.ru_stime.tv_sec, (long int)usage.ru_stime.tv_usec);

	// printf("Tiempo CPU: %ld.%06ld segundos en USER, %ld.%06ld segundos en KERNEL\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
	//    usage.ru_utime.tv_sec,
	//    usage.ru_utime.tv_usec,
	//    usage.ru_stime.tv_sec,
	//    usage.ru_stime.tv_usec);
}

int main(int argc, char *argv[])
{
	srand((unsigned)time(NULL));
	int sentMessages;
	sentMessages = 0;
	int err = 0;
	int ret = 0;
	unsigned int media;
	media = atoi(argv[2]);
	int TEspera = 0;
	//TEspera = ran_expo(1 / media) + 1;
	TEspera = 5;
	pid_t processId = getpid();
	// unsigned int sec = 0;
	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;

	check_error(scberr, ret, scberrormsgcreate);
	printf("%s", scberrormsgcreate);
	scberr = get_buffer(&ctx, argv[1], &err);
	scberr = add_productor(&ctx, argv[1], TEspera, &err);

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
			finalizar(sentMessages, tiempoBloqueo, tiempoEspera, processId);
			remove_productor(&ctx, argv[1], &err);
			break;
		}
		buffer ctx;
		//system("clear");
		//se refresca el buffer
		scberr = get_buffer(&ctx, argv[1], &err);

		//se crea un mensaje
		generate_message(&msj);

		start = clock();
		//Se intenta escribir
		scberr = put_msg(&ctx, &msj, copyMessage, UNBLOCK, &ret);
		tiempoBloqueo += clock() - start;
		if (scberr == SCB_OK)
		{
			sentMessages = sentMessages + 1;
		}
		struct tm *info;
		printf(ANSI_GREEN_BACKGROUND ANSI_COLOR_BLACK " Soy un productor corriendo  PID: %i " ANSI_COLOR_RESET "\n", msj.pid);
		printf("Generé el número mágico: " ANSI_PINK_BACKGROUND " %i " ANSI_COLOR_RESET "\n", msj.numero_magico);
		printf("A las : " ANSI_CYAN_BACKGROUND ANSI_COLOR_BLACK " %s " ANSI_COLOR_RESET, asctime(info));
		info = localtime(&(msj.time));
		sleep(TEspera); // Val
		tiempoEspera += TEspera;
	}
}
