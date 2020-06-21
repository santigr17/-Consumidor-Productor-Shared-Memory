#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffercircular.h"
#include "mensaje.h"




int main(int argc, char *argv[])
{
	int ret = 0;
	unsigned int sec = 0;
	char scberrormsgcreate[TAMAX_MSGERROR + 1] = {'\0'};
	buffer ctx;
	errores scberr = SCB_OK;


    check_error(scberr, ret, scberrormsgcreate);
	printf("%s", scberrormsgcreate);

	printf("Ligando productor con el buffer: [%s]\n", argv[1]);

    //SE LIGA PRODUCTOR CON BUFFER USANDO EL ARGUMENTO PASADO POR CONSOLA
	scberr = ligar_buffer(&ctx, argv[1], &ret,1);
    SCB_SAMPLE_CHECK_ERROR(SCB_OK, scberr, ret, 1);

    
	
    while(1){
        printf("Soy un productor corriendo \n");
    }
}
