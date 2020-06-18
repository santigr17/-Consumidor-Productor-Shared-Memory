/*
Basado en el codigo de Augusto Giannotti con licencia MIT
*/


#ifndef __LIB_BUFFERSHARED__
#define __LIB_BUFFERSHARED__

#include <stdint.h>
#include <semaphore.h>

#define BUFFERSHARED_NAME_MAXSZ (30)
#define SCB_ERRORMSG_MAXSZ (100)


/*
Struct que define el buffer circular 
*/
typedef struct _buffershared_ctrl_t{
    sem_t vacio;
    sem_t lleno;
    sem_t buffCtrl;

    uint16_t cabeza;
    uint16_t cola;
    uint16_t qtd;

    uint16_t dataTotal; /*Total de elementos*/
    size_t dataElementSz; /*Tamaño de un elemento*/
}buffershared_ctrl_t;


typedef struct  _buffershared_t{
    char nombre[BUFFERSHARED_NAME_MAXSZ];
    buffershared_ctrl_t *ctrl;
    void *data;

}buffershared_t;

typedef struct _buffershared_iter_t{
	uint16_t it;
}buffershared_iter_t;

/*enum para el manejo de errores*/

typedef enum{
    SCB_OK = 0,
	SCB_SHMEM,
	SCB_FTRUNC,
	SCB_SEMPH,
	SCB_MMAP,
	SCB_FULL,
	SCB_EMPTY,
	SCB_BLOCKED
}buffershared_err_t;


typedef enum{
	SCB_BLOCK = 0,
	SCB_UNBLOCK
}buffershared_block_t;

/*Creacion del buffer*/
buffershared_err_t buffershared_create(char *nombre, uint16_t totalElements, size_t sizeElements, buffershared_t *ctx, int *err);
/*Ligar el  buffer*/
buffershared_err_t buffershared_attach(buffershared_t *ctx, char *nombre, int *err);



/*Obtener dato del  buffer*/
buffershared_err_t buffershared_get(buffershared_t *ctx, void *element, void *(*copyElement)(void *dst, const void *src), buffershared_block_t block);
/*Meter dato en el   buffer*/
buffershared_err_t buffershared_put(buffershared_t *ctx, void *element, void *(*copyElement)(void *dst, const void *src), buffershared_block_t block);
/*Obtener datos del  buffer*/
buffershared_err_t buffershared_getInfo(char *nombre, buffershared_ctrl_t *inf, int *err);
/*Crear un iterador*/
buffershared_err_t buffershared_iterator_create(buffershared_t *ctx, buffershared_iter_t *ctxIter);
/*Obtener datos con iterador*/
buffershared_err_t buffershared_iterator_get(buffershared_t *ctx, buffershared_iter_t *ctxIter, void *data);
/*Destruccion del buffer*/
buffershared_err_t buffershared_destroy(char *nombre, int *err);
/*manejo de errores*/
void buffershared_strerror(buffershared_err_t err, int ret, char *msg);

#endif