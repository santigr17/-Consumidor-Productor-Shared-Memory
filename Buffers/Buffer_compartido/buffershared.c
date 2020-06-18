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

#include "buffershared.h"

buffershared_err_t buffershared_create(char *nombre,uint16_t totalElements, size_t sizeElements, buffershared_t *ctx, int *err){

    int fdshmem = 0;
	size_t szshmem = 0;
	void *shmem = NULL;
    /*Tamaño que se pedira para el buffer*/
	szshmem = sizeof(buffershared_ctrl_t) + (totalElements * sizeElements);

    fdshmem = shm_open(nombre, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	if(ftruncate(fdshmem, szshmem) == -1){
		*err = errno;
		shm_unlink(nombre);
		return(SCB_FTRUNC);
	}

	/* Se hace uso de mmap para la memoria comparida */
	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(nombre);
		return(SCB_MMAP);
	}

	close(fdshmem);

	ctx->ctrl = (buffershared_ctrl_t *)shmem;

	strncpy(ctx->nombre, nombre, BUFFERSHARED_NAME_MAXSZ);
    
    /*Se setena punteros del buffer circular*/

	ctx->ctrl->cabeza = 0;
	ctx->ctrl->cola = 0;
	ctx->ctrl->qtd  = 0;

	ctx->ctrl->dataTotal     = totalElements;
	ctx->ctrl->dataElementSz = sizeElements;

	ctx->data = (void *)(shmem + sizeof(buffershared_ctrl_t));

	if(sem_init(&(ctx->ctrl->vacio), 1, totalElements) == -1){
		*err = errno;
		shm_unlink(nombre);
		return(SCB_SEMPH);
	}

	if(sem_init(&(ctx->ctrl->lleno), 1, 0) == -1){
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		shm_unlink(nombre);
		return(SCB_SEMPH);
	}

	if(sem_init(&(ctx->ctrl->buffCtrl), 1, 1) == -1){
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		sem_destroy(&ctx->ctrl->lleno);
		shm_unlink(nombre);
		return(SCB_SEMPH);
	}

	*err = 0;
	return(SCB_OK);

}


buffershared_err_t buffershared_attach(buffershared_t *ctx, char *nombre, int *err)
{
	int fdshmem = 0;
	size_t szshmem = 0;
	void *shmem = NULL;
	buffershared_ctrl_t buffersharedInf;
	buffershared_err_t buffersharederr;

	buffersharederr = buffershared_getInfo(nombre, &buffersharedInf, err);
	if(buffersharederr != SCB_OK) return(buffersharederr);

    /*tamaño de la memoria*/

	szshmem = sizeof(buffershared_ctrl_t) + (buffersharedInf.dataTotal * buffersharedInf.dataElementSz);

	fdshmem = shm_open(nombre, O_RDWR, S_IRUSR | S_IWUSR);
	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(nombre);
		return(SCB_SHMEM);
	}

	close(fdshmem);

	ctx->ctrl = (buffershared_ctrl_t *)shmem;
	ctx->data = (void *)(shmem + sizeof(buffershared_ctrl_t));
	strncpy(ctx->nombre, nombre, BUFFERSHARED_NAME_MAXSZ);

	*err = 0;
	return(SCB_OK);
}



buffershared_err_t buffershared_get(buffershared_t *ctx, void *element,  void *(*copyElement)(void *dest, const void *src), buffershared_block_t block)
{
	buffershared_err_t ret = SCB_OK;

	if(block == SCB_UNBLOCK){
		if(sem_trywait(&(ctx->ctrl->lleno)) == -1) return(SCB_BLOCKED);
	}else sem_wait(&(ctx->ctrl->lleno));

	sem_wait(&(ctx->ctrl->buffCtrl));

	if(ctx->ctrl->qtd == 0) ret = SCB_EMPTY;
	else{
		copyElement(element, ctx->data + (ctx->ctrl->cola * ctx->ctrl->dataElementSz));

		ctx->ctrl->cola = (ctx->ctrl->cola + 1) % ctx->ctrl->dataTotal;
		ctx->ctrl->qtd--;
	}


    /*Se modifica el estado de los semaforos , esto para evitar que se intente hacer un get a un buffer vacio*/
	sem_post(&(ctx->ctrl->buffCtrl));
	sem_post(&(ctx->ctrl->vacio));

	return(ret);
}


/*
 * block - 0 unblock | 1 block
 */
buffershared_err_t buffershared_put(buffershared_t *ctx, void *element, void *(*copyElement)(void *dest, const void *src), buffershared_block_t block)
{
	buffershared_err_t ret = SCB_OK;
	/*Se verifica si lo que va a hacer es sacar algo de un buffer vacio*/

	if(block == SCB_UNBLOCK){
		if(sem_trywait(&(ctx->ctrl->vacio)) == -1) return(SCB_BLOCKED);
	}else sem_wait(&(ctx->ctrl->vacio));

	sem_wait(&(ctx->ctrl->buffCtrl));

	if(ctx->ctrl->qtd == ctx->ctrl->dataTotal) ret = SCB_FULL;
	else{
		copyElement(ctx->data + (ctx->ctrl->cabeza * ctx->ctrl->dataElementSz), element);

		ctx->ctrl->cabeza = (ctx->ctrl->cabeza + 1) % ctx->ctrl->dataTotal;
		ctx->ctrl->qtd++;
	}

	sem_post(&(ctx->ctrl->buffCtrl));
	sem_post(&(ctx->ctrl->lleno));

	return(ret);
}

buffershared_err_t buffershared_iterator_create(buffershared_t *ctx, buffershared_iter_t *ctxIter)
{
	ctxIter->it = ctx->ctrl->cola;

	return(SCB_OK);
}

buffershared_err_t buffershared_iterator_get(buffershared_t *ctx, buffershared_iter_t *ctxIter, void *data)
{
	if((ctx->ctrl->cabeza < ctxIter->it) || (ctx->ctrl->cola > ctxIter->it)){
		ctxIter->it = ctx->ctrl->cola;
	}

	return(SCB_OK);
}

buffershared_err_t buffershared_destroy(char *nombre, int *err)
{
	int ret = 0;
	buffershared_t ctx;
	buffershared_err_t buffersharederr;

	buffersharederr = buffershared_attach(&ctx, nombre, err);
	if(buffersharederr != SCB_OK) return(buffersharederr);

	ret = sem_destroy(&(ctx.ctrl->buffCtrl)) | sem_destroy(&(ctx.ctrl->vacio)) | sem_destroy(&(ctx.ctrl->lleno));

	if(ret != 0){
		*err = errno;
		return(SCB_SEMPH);
	}

	if(shm_unlink(ctx.nombre) == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	return(SCB_OK);
}

void buffershared_strerror(buffershared_err_t err, int ret, char *msg)
{
	char *errat = NULL;
	char errdesc[SCB_ERRORMSG_MAXSZ] = {'\0'};


	switch(err){
		case SCB_SHMEM:
			errat = "Shared Memory";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_FTRUNC:
			errat = "FTruncate";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_SEMPH:
			errat = "Semaphore";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_MMAP:
			errat = "mmap";
			/* TODO strerror_r(ret, errdesc, SCB_ERRORMSG_MAXSZ); */
			strncpy(errdesc, strerror(ret), SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_FULL:
			errat = "SCB FULL";
			strncpy(errdesc, "There is no space", SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_EMPTY:
			errat = "SCB EMPTY";
			strncpy(errdesc, "There are no elements", SCB_ERRORMSG_MAXSZ);
			break;
		case SCB_BLOCKED:
			errat = "SCB BLOCKED";
			strncpy(errdesc, "Access blocked (in use at operation side or full or empty. Try again)", SCB_ERRORMSG_MAXSZ);
			break;
		default:
			errat = "Success";
			strncpy(errdesc, "no error", SCB_ERRORMSG_MAXSZ);
			break;
	}

	snprintf(msg, SCB_ERRORMSG_MAXSZ, "Error at [%s]: [%s]\n", errat, errdesc);
}

buffershared_err_t buffershared_getInfo(char *nombre, buffershared_ctrl_t *inf, int *err)
{
	int fdshmem = 0;
	void *shmem = NULL;

	fdshmem = shm_open(nombre, O_RDONLY, S_IRUSR | S_IWUSR);
	if(fdshmem == -1){
		*err = errno;
		return(SCB_SHMEM);
	}

	shmem = mmap(NULL, sizeof(buffershared_t), PROT_READ, MAP_SHARED, fdshmem, 0);
	if(shmem == MAP_FAILED){
		*err = errno;
		shm_unlink(nombre);
		return(SCB_SHMEM);
	}

	memcpy(inf, shmem, sizeof(buffershared_ctrl_t));

	close(fdshmem);

	return(SCB_OK);
}
