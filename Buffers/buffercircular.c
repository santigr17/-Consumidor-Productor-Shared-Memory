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

	ctx->ctrl = (buffer *)shmem;

	strncpy(ctx->name, name, TAMAX_NOMBRE);
    //Se setean los valores de los punteros
	ctx->ctrl->cabeza = 0;
	ctx->ctrl->cola = 0;
	ctx->ctrl->qtd  = 0;

    ctx->ctrl->consumidores = 0;
    ctx->ctrl->productores = 0;

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

     /*Se pide y se crea el semáforo para finalizar todo
    
    Si falla se destruye los sem anteriores y se usa unlink para devolver la mem
    */


	if(sem_init(&(ctx->ctrl->finalizar), 1, 0) == -1){
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		sem_destroy(&ctx->ctrl->lleno);
        sem_destroy(&ctx->ctrl->con_carrera);
		shm_unlink(name);
		return(SCB_SEMPH);
	}


	*err = 0;
	return(SCB_OK);

}


//Función para obtener información del buffer

errores scb_getInfo(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores,int *err)
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







