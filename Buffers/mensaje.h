#ifndef _MENSAJE_H
#define _MENSAJE_H
#include <time.h>
typedef struct _mensaje{
	int numero_magico;
	pid_t pid; // El process id del productor
	time_t time; //EPOCH el momento cuando se crea el mensaje cantidad de segundos desde  1 de enero de 1970
}mensaje;

#define SCB_SAMPLE_CHECK_ERROR(__SCBSAMPLE_RET, __SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, __SCB_SAMPLE_RET) \
{                                                                                                          \
	if(__SCBSAMPLE_RET != __SCB_SAMPLE_APIERR){                                                             \
		char scberrormsg[TAMAX_MSGERROR + 1] = {'\0'};                                                   \
		check_error(__SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, scberrormsg);                                  \
		printf("%s", scberrormsg);                                                                           \
		return(__SCB_SAMPLE_RET);                                                                            \
	}                                                                                                       \
}

//Funcion para leer o escribir mensajes directamente en memoria con un src y un destino

#include <string.h>
void * copyElement(void *dst, const void *src)
{
	return(memcpy(dst, src, sizeof(mensaje)));
}

#endif