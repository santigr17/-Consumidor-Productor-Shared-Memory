#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "buffercircular.h"
#include "mensaje.h"


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
	scberr = get_buffer(&ctx,argv[1], &err);
	scberr = add_consumidor(&ctx,argv[1], &err);

	

	//SE LIGA PRODUCTOR CON BUFFER USANDO EL ARGUMENTO PASADO POR CONSOLA
	SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, ret, 1);
	
	while (1)
	{
        mensaje msj;
		buffer ctx;
		system("clear");
		scberr = get_buffer( &ctx,argv[1], &err);

		printf("para mostrar que se está jalando el nombre del ctx :\n");
    	char *x ;
		x = ctx.name;
		printf("ctrl..............: [%s]\n", x);

		//Se intenta leer
		scberr = get_msg(&ctx, &msj, copyMessage, BLOCK, &ret);

        printf("Mi buffer es: [%s]\n", argv[1]);
	    printf("Numero mágico: ..............: [%u]\n", msj.numero_magico);
	    printf("Escrito por:................: [%u]\n", msj.pid);
	    printf("A las: .................: [%lu]\n", msj.time);

		
		sleep(4);
	}
}
