#ifndef _BUFFERCIRCULAR_H
#define _BUFFERCIRCULAR_H

#include <stdint.h>
#include <semaphore.h>

//Se define tamaño máximo para el nombre y los errores

#define TAMAX_NOMBRE (30)
#define TAMAX_MSGERROR (100)

//Se crea struct para controlar el buffer y ver su estado

/*
+-------------+-----------+-----------+-----+-----------+
| Buffer control | espacio 1 | espacio 2 | ... | espacio n |
+-------------+-----------+-----------+-----+-----------+*/


//estructura para el control

typedef struct _buffer_control{
	uint16_t consumidores;
	uint16_t productores;
	sem_t vacio;
	sem_t lleno;
	sem_t con_carrera;
	uint16_t finalizar;
	uint16_t initFinalizado;
	uint16_t cabeza;
	uint16_t cola;
	uint16_t qtd;
	uint16_t capacidad;
	size_t   largo_mensaje; 
	uint16_t maxEspera;
}buffer_control;


// estructura para el total del buffer

typedef struct _buffer{
	char name[TAMAX_NOMBRE];
	buffer_control *ctrl;
	void *mensajes;
}buffer;



// estructura para iterar (revisar)

typedef struct _iterar{
	uint16_t it;
	uint16_t qtd;
}iterar;

// enum para manejo de error



typedef enum{
	SCB_OK = 0,
	SCB_SHMEM,
	SCB_FTRUNC,
	SCB_SEMPH,
	SCB_MMAP,
	SCB_LLENO,
	SCB_VACIO,
	SCB_BLOQUEADO,
	SCB_ITER_FINAL
}errores;

//enum para bloquear el buffer

typedef enum{
	BLOCK = 0,
	UNBLOCK
}bloqueo;

//función para crear un buffer compartido nuevo

errores crear_buffer(char *name, uint16_t totalElementos, size_t tamElementos, buffer *ctx, int *err);

//función para aumentar número de productores

errores add_productor(buffer *ctx, char *name, int espera ,int *err);

//función para aumentar número de consumidores
errores add_consumidor(buffer *ctx, char *name, int espera, int *err);

//Función para indicar que el iniciador está finalizado
errores remove_iniciador(buffer *ctx, char *name, int *err);

//función para disminuir número de productores

errores remove_productor(buffer *ctx, char *name, int *err);

//función para disminuir número de consumidores
errores remove_consumidor(buffer *ctx, char *name, int *err);



//función para obtener la información del buffer

errores get_info_buffer(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera, int *err);

//función para destruir el buffer

errores destruir_buffer(char *name, int *err);

//función para obtener buffer de memoria compartida

errores get_buffer(buffer *ctx, char *name, int *err);


//función para control de errores

void check_error(errores err, int ret, char *msg);

// solicitar condición de carrera en prueba
errores request_sem(buffer *ctx, bloqueo block, int *err);

//funcion para poner mensaje en el buffer
errores put_msg(buffer *ctx, void *mensaje, void *(*copyMessage)(void *dest, const void *src), bloqueo block, int *err);

//funcion para leer mensaje en el buffer

errores get_msg(buffer *ctx, void *mensaje,  void *(*copyMessage)(void *dest, const void *src), bloqueo block, int *err);


/**
 * Distribución exponencial
 * Valor de lambda se relaciona con el maximo valor posible.
 * Se usa time para obtener un valor inicial. 
 * @param lambda Valor de lambda se relaciona con el maximo valor posible.
 */
double ran_expo(double lambda);
/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_small(double mean);
/**
 * Función lambda logarítmica
 * @param xx variable independiente
 */
double lgamma(double xx);
/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_large(double mean);

/**
 * Función de generar números aleatorios 
 * basado en poisson
 */
int poissrnd(double mean);
#endif
