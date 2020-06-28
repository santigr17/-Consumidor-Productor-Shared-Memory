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
#include <ncurses.h>

//Función para capturar teclas

int kbhit(void)
{
	int ch = getch();

	if (ch != ERR)
	{
		ungetch(ch);
		return 1;
	}
	else
	{
		return 0;
	}
}

//Función especial para finalizar
void finalizar(int sentMessages, double tiempoBloqueo, double tiempoEspera, pid_t pid)
{
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	printf(ANSI_RED_BACKGROUND "\rFinalizando %d debido a variable global fue establecida como TRUE..." ANSI_COLOR_RESET "\n", pid);
	printf("\rMensajes enviados........." ANSI_GRAY_BACKGROUND
		   " %d " ANSI_COLOR_RESET
		   "\n",
		   sentMessages);
	printf("\rTiempo total bloqueado...." ANSI_GRAY_BACKGROUND
		   " %g " ANSI_COLOR_RESET
		   "\n",
		   tiempoBloqueo);
	printf("\rTiempo total de espera...." ANSI_GRAY_BACKGROUND
		   " %g " ANSI_COLOR_RESET
		   "\n",
		   tiempoEspera);
	printf("\rTiempo de CPU:............" ANSI_GRAY_BACKGROUND
		   " %ld.%06ld  " ANSI_COLOR_RESET
		   " segundos en USER. "
		   "\n",
		   (long int)usage.ru_utime.tv_sec, (long int)usage.ru_utime.tv_usec);

	printf("\rTiempo de CPU:............" ANSI_GRAY_BACKGROUND
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

//Función para finalizar con número mágico
void finalizar_magico(int magic, int myMagic)
{
	printf(ANSI_MAGENTA_GREEN_BACKGROUND "Finalizando debido a variable número mágico: %d es igual a mi num mágico %d...\n" ANSI_COLOR_RESET, magic, myMagic);
}

int main(int argc, char *argv[])
{
	int receivedMessages = 0;
	double tiempoBloqueo = 0;
	double tiempoEspera = 0;
	double start;
	pid_t processId = getpid();
	int myMagic = processId % 6;
	srand((unsigned)time(NULL));
	srand48((unsigned)time(NULL));
	int err = 0;
	int ret = 0;
	unsigned int media;

	//Se verifica entradas
	if (argc < 3 || argc > 4)
	{
		printf("ERROR:  Por favor indique:\n\
	-> Nombre del buffer\n\
	-> Tipo de Consumidor m (manual) o a (automatico) \n\
	-> Media de Tiempo*\n*Modo automatico\n");
		return 1;
	}
	if (argc == 3 && strcmp(argv[2], "manual"))
	{
		printf("ERROR:  Por favor para modo manual indique:\n\
	-> Nombre del buffer\n\
	-> Tipo de Consumidor m (manual) o a (automatico) \n");
		return 1;
	}
	if (argc == 4 && strcmp(argv[2], "automatico"))
	{
		printf("ERROR:  Por favor para modo automatico indique:\n\
	-> Nombre del buffer\n\
	-> Tipo de Consumidor m (manual) o a (automatico) \n\
	-> Media de Tiempo\n");
		return 1;
	}

	// unsigned int sec = 0;
	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;
	printf("Creando Consumidor...\n");
	check_error(scberr, ret, scberrormsgcreate);
	printf("%s", scberrormsgcreate);

	scberr = get_buffer(&ctx, argv[1], &err);

	//Funciones para la captura de teclas
	WINDOW *win = initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	//SE LIGA PRODUCTOR CON BUFFER USANDO EL ARGUMENTO PASADO POR CONSOLA
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, ret, 1);

	if (!strcmp(argv[2], "manual"))
	{
		int espera = 1;
		scberr = add_consumidor(&ctx, argv[1], espera, &err);
		while (1)
		{

			mensaje msj;
			buffer ctx;
			// se pide el buffer a memoria compartida
			scberr = get_buffer(&ctx, argv[1], &err);

			/* errores get_info(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores, int *err);*/
			errores scberr2;
			//se pide info del controller
			int err = 0;
			int semlleno = 0;
			int semvacio = 0;
			int semcon_carrera = 0;
			buffer_control inf;
			scberr2 = get_info_buffer(argv[1], &inf, &semlleno, &semvacio, &semcon_carrera, &err);
			printf("Finalizador.........: [%u] \n\r", inf.finalizar);
			//Se tiene que verificar finalizador
			if (inf.finalizar)
			{
				finalizar(receivedMessages, tiempoBloqueo, tiempoEspera, processId);

				return -1;
			}
			if (kbhit())
			{
				printw("Validando char \n");
				int enter = getch();

				//Se verifica algun error
				SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr2, err, 1);

				if (enter == 10)
				{
					start = clock();
					scberr = get_msg(&ctx, &msj, copyMessage, BLOCK, &ret);
					tiempoBloqueo += clock() - start;
					printf(ANSI_COLOR_RESET "\rMi buffer es:......." ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %s \n\r " ANSI_COLOR_RESET, argv[1]);
					printf(ANSI_COLOR_RESET "\rNumero mágico:......" ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %u \n\r " ANSI_COLOR_RESET, msj.numero_magico);
					printf(ANSI_COLOR_RESET "\rEscrito por:........" ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %u \n\r " ANSI_COLOR_RESET, msj.pid);
					struct tm *info;
					info = localtime(&(msj.time));
					printf(ANSI_COLOR_RESET "\rHora:..............." " %s \n\r " , asctime(info));
					printf(ANSI_COLOR_RESET);
					if (scberr == SCB_OK)
					{
						receivedMessages = receivedMessages + 1;
					}
					if (msj.numero_magico == myMagic)
					{
						finalizar_magico(msj.numero_magico, myMagic);
						break;
					}
					sleep(4);
				}
			}
			else
			{
				printf(ANSI_COLOR_MAGENTA"Por favor presionar ENTER para intentar consumir mensaje \n"ANSI_COLOR_RESET);
				sleep(2);
			}
			system("clear");
		}
	}
	else if (!strcmp(argv[2], "automatico"))
	{

		media = atoi(argv[3]);
		// printf("maDato ingresado:%s\n\r",argv[3]);
		if (media <= 0)
		{
			printf("ERROR:  Por favor para modo automatico:\n\r\
		El tiempo ingresado es invalido\n\r\
		Por favor ingrese un valor mayor a 0\n\r");
			return 1;
		}
		unsigned int TEspera;
		TEspera = poissrnd(media + 0.0) + 1;
		scberr = add_consumidor(&ctx, argv[1], TEspera, &err);
		printf("Dato ingresado:%s y tiempo de espera generado %d\n\r", argv[3], TEspera);
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
			printf("Finalizador.........: [%u]\n\r", inf.finalizar);

			//Se tiene que verificar finalizador
			if (inf.finalizar)
			{
				finalizar(receivedMessages, tiempoBloqueo, tiempoEspera, processId);
				break;
			}
			mensaje msj;
			buffer ctx;

			// se pide el buffer a memoria compartida
			scberr = get_buffer(&ctx, argv[1], &err);
			//Se intenta leer
			start = clock();
			scberr = get_msg(&ctx, &msj, copyMessage, BLOCK, &ret);
			tiempoBloqueo += clock() - start;
			printf(ANSI_COLOR_RESET "Mi buffer es:......." ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %s \n\r" ANSI_COLOR_RESET, argv[1]);
			printf(ANSI_COLOR_RESET "Numero mágico:......" ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %u \n\r" ANSI_COLOR_RESET, msj.numero_magico);
			printf(ANSI_COLOR_RESET "Tiempo Espera:......" ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %d \n\r" ANSI_COLOR_RESET, TEspera);
			printf(ANSI_COLOR_RESET "Escrito por:........" ANSI_LIGHT_GREEN_BACKGROUND ANSI_COLOR_BLACK " %u \n\r" ANSI_COLOR_RESET, msj.pid);
			struct tm *info;
			info = localtime(&(msj.time));
			printf(ANSI_COLOR_RESET "Hora:..............." " %s \n\r " , asctime(info));
			if (scberr == SCB_OK)
			{
				receivedMessages = receivedMessages + 1;
			}
			if (msj.numero_magico == myMagic)
			{
				finalizar_magico(msj.numero_magico, myMagic);
				break;
			}
			sleep(TEspera);
			tiempoEspera += TEspera;
			system("clear");
		}
	}
	else
	{
		printf("ERROR: Por favor indique nombre del buffer y el tipo de consumidor m (manual) o a (automatico) \n\r");
	}

	endwin();
	return 0;
}
