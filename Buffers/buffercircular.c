#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "buffercircular.h"


#define SCB_SAMPLE_CHECK_ERROR(__SCBSAMPLE_RET, __SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, __SCB_SAMPLE_RET) \
{                                                                                                          \
	if(__SCBSAMPLE_RET != __SCB_SAMPLE_APIERR){                                                             \
		char scberrormsg[TAMAX_MSGERROR + 1] = {'\0'};                                                   \
		check_error(__SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, scberrormsg);                                  \
		printf("%s", scberrormsg);                                                                           \
		return(__SCB_SAMPLE_RET);                                                                            \
	}                                                                                                       \
}




//Definición de función que crea el buffer

errores crear_buffer(char *name, uint16_t totalElementos, size_t tamElementos, buffer *ctx, int *err)
{
    
    int fdshmem = 0;
	size_t szshmem = 0;
	void *shmem = NULL;

    //Se pude el tamaño necesario para el bloque de control + los bloques para mensajes
	szshmem = sizeof(buffer) + (totalElementos * tamElementos);
	fdshmem = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}
    //Se verifica que el tamaño de memoria a solicitar y el que se necesita calzen
	if(ftruncate(fdshmem, szshmem) == -1){
		*err = errno;
		shm_unlink(name);
		return(SCB_FTRUNC);
	}

    //Se pide la memoria compartida con mmap
    
	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(name);
		return(SCB_MMAP);
	}

	close(fdshmem);

    //Se asignan los punteros

	ctx->ctrl = (buffer_control *)shmem;

	strncpy(ctx->name, name, TAMAX_NOMBRE);
    //Se setean los valores de los punteros
	ctx->ctrl->cabeza = 0;
	ctx->ctrl->cola = 0;
	ctx->ctrl->qtd  = 0;

	ctx->ctrl->productores = 0;
	ctx->ctrl->consumidores  = 0;

	ctx->ctrl->capacidad     = totalElementos;
	ctx->ctrl->largo_mensaje = tamElementos;

	ctx->mensajes = (void *)(shmem + sizeof(buffer));

    /*Se pide y se crea el semáforo para vacío
    
    Si falla se usa unlink para devolver la mem
    */

	if(sem_init(&(ctx->ctrl->vacio), 1, totalElementos) == -1){
		*err = errno;
		shm_unlink(name);
		return(SCB_SEMPH);
	}

    /*Se pide y se crea el semáforo para lleno
    
    Si falla se destruye el sem anterior y se usa unlink para devolver la mem
    */

	if(sem_init(&(ctx->ctrl->lleno), 1, 0) == -1){
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		shm_unlink(name);
		return(SCB_SEMPH);
	}
    /*Se pide y se crea el semáforo para condición de carrera
    
    Si falla se destruye los sem anteriores y se usa unlink para devolver la mem
    */


	if(sem_init(&(ctx->ctrl->con_carrera), 1, 1) == -1){
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		sem_destroy(&ctx->ctrl->lleno);
		shm_unlink(name);
		return(SCB_SEMPH);
	}



	*err = 0;
	return(SCB_OK);

}

//Función para ligar procesos al espacio de memoria del buffer

errores ligar_buffer(buffer *ctx, char *name, int *err,int *tipo)
{
	int fdshmem = 0;
	int sf = 0, se = 0, sb = 0;
	size_t szshmem = 0;
	void *shmem = NULL;
	buffer_control scbInf;
	errores scberr;

    /*get_info_buffer(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores, int *err);*/
    //se busca la infor de ese buffer
    // se consigue la info actual  de ese espacio de memoria

        printf("Antes del get info ");
    
		int productores_nuevos = 0;
		int consumidores_nuevos = 0;


	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);


    //se calcula lo grande del largo de mem
	szshmem = sizeof(buffer_control) + (scbInf.capacidad * scbInf.largo_mensaje);

	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(name);
		return(SCB_SHMEM);
	}

	//Se modifica estados de productores o consumidores

		if(scberr != SCB_OK) return(scberr);
		buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ|PROT_WRITE, MAP_SHARED, fdshmem, 0);
 		
		  //decirle al puntero de productores que se sume 1 
		productores_nuevos = scbInf.productores +1;
		printf("Info nueva.: [%u]\n", productores_nuevos);
		(*puntero).productores = productores_nuevos;
	 

	close(fdshmem);

    //Se ligan los punteros

	ctx->ctrl = (buffer_control *)shmem;
	ctx->mensajes = (void *)(shmem + sizeof(buffer_control));
	strncpy(ctx->name, name, TAMAX_MSGERROR);


	*err = 0;
	return(SCB_OK);
}

//Función para obtener información del buffer

errores get_info_buffer(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *err)
{
	int fdshmem = 0;
	void *shmem = NULL;

    //similar a create se utiliza la etiqueta name para pedir los datos

	fdshmem = shm_open(name, O_RDONLY, S_IRUSR | S_IWUSR);
	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	shmem = mmap(NULL, sizeof(buffer), PROT_READ, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(name);
		return(SCB_SHMEM);
	}

	memcpy(inf, shmem, sizeof(buffer_control));

	sem_getvalue(&((buffer_control *) shmem)->lleno, semlleno);
	sem_getvalue(&((buffer_control *) shmem)->vacio, semvacio);
	sem_getvalue(&((buffer_control *) shmem)->con_carrera, semcon_carrera);

	close(fdshmem);

	return(SCB_OK);
}

//Función para el manejo de errores 

void check_error(errores err, int ret, char *msg)
{
	char *errat = NULL;
	char errdesc[TAMAX_MSGERROR] = {'\0'};

	switch(err){
		case SCB_SHMEM:
			errat = "Shared Memory";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), TAMAX_MSGERROR);
			break;
		case SCB_FTRUNC:
			errat = "FTruncate";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), TAMAX_MSGERROR);
			break;
		case SCB_SEMPH:
			errat = "Semaphore";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), TAMAX_MSGERROR);
			break;
		case SCB_MMAP:
			errat = "mmap";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), TAMAX_MSGERROR);
			break;
		case SCB_LLENO:
			errat = "SCB FULL";
			strncpy(errdesc, "There is no space", TAMAX_MSGERROR);
			break;
		case SCB_VACIO:
			errat = "SCB EMPTY";
			strncpy(errdesc, "There are no elements", TAMAX_MSGERROR);
			break;
		case SCB_BLOQUEADO:
			errat = "SCB BLOCKED";
			strncpy(errdesc, "Access blocked (in use at operation side or full or empty. Try again)", TAMAX_MSGERROR);
			break;
		default:
			errat = "Success";
			strncpy(errdesc, "no error", TAMAX_MSGERROR);
			break;
	}

	snprintf(msg, TAMAX_MSGERROR, "Error at [%s]: [%s]\n", errat, errdesc);
}







