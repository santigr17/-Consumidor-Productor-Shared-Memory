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

int main(int argc, char *argv[])
{
	int ret = 0;
	// unsigned int sec = 0;
	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;

	check_error(scberr, ret, scberrormsgcreate);
	printf("%s", scberrormsgcreate);

	printf("Ligando productor con el buffer: [%s]\n", argv[1]);

	//SE LIGA PRODUCTOR CON BUFFER USANDO EL ARGUMENTO PASADO POR CONSOLA
	scberr = ligar_buffer(&ctx, argv[1], &ret, 1);
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, ret, 1);
	mensaje msj;
	while (1)
	{
		system("clear");
		generate_message(&msj);
		struct tm *info;
		info = localtime(&(msj.time));
		printf("Soy un productor corriendo  PID: %i, Magic: %i, Date UTC: %s",
			   msj.pid,
			   msj.numero_magico,
			   asctime(info));
		sleep(1);
	}
}
