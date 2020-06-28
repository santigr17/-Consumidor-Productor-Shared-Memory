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
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "buffercircular.h"

enum subscription
{
	NEW_CONSUMER,
	NEW_PRODUCER,
	ALREADY_REGISTERED
};
#define SCB_SAMPLE_CHECK_ERROR(__SCBSAMPLE_RET, __SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, __SCB_SAMPLE_RET) \
	{                                                                                                      \
		if (__SCBSAMPLE_RET != __SCB_SAMPLE_APIERR)                                                        \
		{                                                                                                  \
			char scberrormsg[TAMAX_MSGERROR + 1] = {'\0'};                                                 \
			check_error(__SCB_SAMPLE_APIERR, __SCB_SAMPLE_ERRNO, scberrormsg);                             \
			printf("%s", scberrormsg);                                                                     \
			return (__SCB_SAMPLE_RET);                                                                     \
		}                                                                                                  \
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

	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}
	//Se verifica que el tamaño de memoria a solicitar y el que se necesita calzen
	if (ftruncate(fdshmem, szshmem) == -1)
	{
		*err = errno;
		shm_unlink(name);
		return (SCB_FTRUNC);
	}

	//Se pide la memoria compartida con mmap

	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if (shmem == MAP_FAILED)
	{
		*err = errno;
		shm_unlink(name);
		return (SCB_MMAP);
	}

	//close(fdshmem);

	//Se asignan los punteros

	ctx->ctrl = (buffer_control *)shmem;

	strncpy(ctx->name, name, TAMAX_NOMBRE);
	//Se setean los valores de los punteros
	ctx->ctrl->cabeza = 0;
	ctx->ctrl->cola = 0;
	ctx->ctrl->qtd = 0;

	ctx->ctrl->productores = 0;
	ctx->ctrl->consumidores = 0;
	ctx->ctrl->maxEspera = 1;

	ctx->ctrl->capacidad = totalElementos;
	ctx->ctrl->largo_mensaje = tamElementos;
	ctx->ctrl->initFinalizado = 0;
	ctx->ctrl->finalizar = 0;

	ctx->mensajes = (void *)(shmem + sizeof(buffer));

	/*Se pide y se crea el semáforo para vacío
    
    Si falla se usa unlink para devolver la mem
    */
	// si se puede bloquear, entra al if
	// if(block == BLOCK){
	// 	//se intenta tomar control de
	// }

	if (sem_init(&(ctx->ctrl->vacio), 1, totalElementos) == -1)
	{
		*err = errno;
		shm_unlink(name);
		return (SCB_SEMPH);
	}

	/*Se pide y se crea el semáforo para lleno
    
    Si falla se destruye el sem anterior y se usa unlink para devolver la mem
    */

	if (sem_init(&(ctx->ctrl->lleno), 1, 0) == -1)
	{
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		shm_unlink(name);
		return (SCB_SEMPH);
	}
	/*Se pide y se crea el semáforo para condición de carrera
    
    Si falla se destruye los sem anteriores y se usa unlink para devolver la mem
    */

	if (sem_init(&(ctx->ctrl->con_carrera), 1, 1) == -1)
	{
		*err = errno;
		sem_destroy(&ctx->ctrl->vacio);
		sem_destroy(&ctx->ctrl->lleno);
		shm_unlink(name);
		return (SCB_SEMPH);
	}

	*err = 0;
	return (SCB_OK);
}

//Función para aumentar número de productores en el buffer
/**
 * TIPO 0 : Consumidor inical
 * TIPO 1 : Productor inicial
 * TIPO 2 : Consumidor / Productos ya creado
*/
errores add_productor(buffer *ctx, char *name, int espera, int *err)
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
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estados de productores
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);

	int productores_nuevos = scbInf.productores;
	productores_nuevos = productores_nuevos + 1;
	(*puntero).productores = productores_nuevos;
	int maxEspera = scbInf.maxEspera;
	// if(maxEspera <  espera){
	// }
	(*puntero).maxEspera += espera;
	*err = 0;

	close(fdshmem);
	return (SCB_OK);
}

//Función para indicar que el iniciador está finalizado
errores remove_iniciador(buffer *ctx, char *name, int *err)
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
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estados de productores
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	(*puntero).initFinalizado = 1;
	*err = 0;

	close(fdshmem);
	return (SCB_OK);
}

//Función para aumentar el número de consumidores en el buffer

errores add_consumidor(buffer *ctx, char *name, int espera, int *err)
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
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estados de productores
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);

	int consumidores_nuevos = scbInf.consumidores;
	consumidores_nuevos = consumidores_nuevos + 1;
	(*puntero).consumidores = consumidores_nuevos;

	*err = 0;
	int maxEspera = scbInf.maxEspera;
	// if(maxEspera <  espera){
	// }
	(*puntero).maxEspera += espera;
	close(fdshmem);
	return (SCB_OK);
}

//Función para remover un productor.
errores remove_productor(buffer *ctx, char *name, int *err)
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
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estados de productores
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);

	// int productores_nuevos = scbInf.productores;
	// productores_nuevos = productores_nuevos - 1;
	(*puntero).productores -= 1;

	*err = 0;

	close(fdshmem);
	return (SCB_OK);
}

//Función para disminuir el número de consumidores en el buffer

errores remove_consumidor(buffer *ctx, char *name, int *err)
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
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estados de productores
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);

	// int consumidores_nuevos = scbInf.consumidores;
	// consumidores_nuevos = consumidores_nuevos - 1;
	(*puntero).consumidores -= 1;

	*err = 0;

	close(fdshmem);
	return (SCB_OK);
}

//Función para obtener información del buffer

errores get_info_buffer(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera, int *err)
{
	int fdshmem = 0;
	void *shmem = NULL;

	//similar a create se utiliza la etiqueta name para pedir los datos

	fdshmem = shm_open(name, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	shmem = mmap(NULL, sizeof(buffer), PROT_READ, MAP_SHARED, fdshmem, 0);
	if (shmem == MAP_FAILED)
	{
		*err = errno;
		shm_unlink(name);
		return (SCB_SHMEM);
	}

	memcpy(inf, shmem, sizeof(buffer_control));

	sem_getvalue(&((buffer_control *)shmem)->lleno, semlleno);
	sem_getvalue(&((buffer_control *)shmem)->vacio, semvacio);
	sem_getvalue(&((buffer_control *)shmem)->con_carrera, semcon_carrera);

	close(fdshmem);

	return (SCB_OK);
}

//Función para el manejo de errores

void check_error(errores err, int ret, char *msg)
{
	char *errat = NULL;
	char errdesc[TAMAX_MSGERROR] = {'\0'};

	switch (err)
	{
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
	if (errat == "Success")
		snprintf(msg, TAMAX_MSGERROR, "\n");
	else
		snprintf(msg, TAMAX_MSGERROR, "Error at [%s]: [%s]\n", errat, errdesc);
}

//función que retorna un buffer de mem compartida

errores get_buffer(buffer *ctx, char *name, int *err)
{
	int fdshmem = 0;
	int sf = 0, se = 0, sb = 0;
	size_t szshmem = 0;
	void *shmem = NULL;
	buffer_control scbInf;
	errores scberr;

	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	if (scberr != SCB_OK)
		return (scberr);

	szshmem = sizeof(buffer_control) + (scbInf.capacidad * scbInf.largo_mensaje);

	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	shmem = mmap(NULL, szshmem, PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);
	if (shmem == MAP_FAILED)
	{
		*err = errno;
		shm_unlink(name);
		return (SCB_SHMEM);
	}

	close(fdshmem);

	ctx->ctrl = (buffer_control *)shmem;
	ctx->mensajes = (void *)(shmem + sizeof(buffer_control));
	strncpy(ctx->name, name, TAMAX_NOMBRE);

	*err = 0;
	return (SCB_OK);
}

// Intentar bloquear el recurso para hacer un uso de forma segura test
errores request_sem(buffer *ctx, bloqueo block, int *err)
{
	//printf(" Buffer circular: Request Context: %p  \n", (ctx->ctrl));
	if (ctx->ctrl == NULL)
	{
		return SCB_SEMPH;
	}
	// si se puede bloquear, entra al if
	if (block == BLOCK)
	{
		//se intenta tomar control de
		printf("se hace wait de lleno\n");
		if (sem_wait(&(ctx->ctrl->lleno)) == -1)
		{
			printf("se entra primer if\n");
			*err = errno;
			return (SCB_SEMPH);
		}
		printf("ctx->ctrl->vacio\n");

		if (sem_wait(&(ctx->ctrl->vacio)) == -1)
		{
			*err = errno;
			sem_post(&(ctx->ctrl->lleno));
			return (SCB_SEMPH);
		}
		printf("ctx->ctrl->con_carrera\n");

		if (sem_wait(&(ctx->ctrl->con_carrera)) == -1)
		{
			*err = errno;
			sem_post(&(ctx->ctrl->lleno));
			sem_post(&(ctx->ctrl->vacio));
			return (SCB_SEMPH);
		}
	}
	else
	{
		if (sem_post(&(ctx->ctrl->vacio)) == -1)
		{
			*err = errno;
			return (SCB_SEMPH);
		}

		if (sem_post(&(ctx->ctrl->lleno)) == -1)
		{
			*err = errno;
			sem_wait(&(ctx->ctrl->vacio));
			return (SCB_SEMPH);
		}

		if (sem_post(&(ctx->ctrl->con_carrera)) == -1)
		{
			*err = errno;
			sem_wait(&(ctx->ctrl->lleno));
			sem_wait(&(ctx->ctrl->vacio));
			return (SCB_SEMPH);
		}
	}
	*err = 0;
	return (SCB_OK);
}

//función que escribe 1 mensaje en el buffer
errores put_msg(buffer *ctx, void *mensaje, void *(*copyMessage)(void *dest, const void *src), bloqueo block, int *err)

{
	errores ret = SCB_OK;

	if (block == UNBLOCK)
	{
		if (sem_trywait(&(ctx->ctrl->vacio)) == -1)
			return (BLOCK);
	}
	else
	{
		if (sem_wait(&(ctx->ctrl->vacio)) == -1)
		{
			*err = errno;
			return (SCB_SEMPH);
		}
	}

	if (sem_wait(&(ctx->ctrl->con_carrera)) == -1)
	{
		*err = errno;
		sem_post(&(ctx->ctrl->lleno));
		return (SCB_SEMPH);
	}

	if (ctx->ctrl->qtd == ctx->ctrl->capacidad)
		ret = SCB_LLENO;
	else
	{
		copyMessage(ctx->mensajes + (ctx->ctrl->cabeza * ctx->ctrl->largo_mensaje), mensaje);

		ctx->ctrl->cabeza = (ctx->ctrl->cabeza + 1) % ctx->ctrl->capacidad;
		ctx->ctrl->qtd++;
	}

	sem_post(&(ctx->ctrl->con_carrera));
	sem_post(&(ctx->ctrl->lleno));

	*err = 0;
	return (ret);
}

// función que lee un mensaje , usada para consumidor

errores get_msg(buffer *ctx, void *mensaje, void *(*copyMessage)(void *dest, const void *src), bloqueo block, int *err)
{
	errores ret = SCB_OK;

	if (block == UNBLOCK)
	{
		if (sem_trywait(&(ctx->ctrl->lleno)) == -1)
			return (SCB_BLOQUEADO);
	}
	else
	{
		if (sem_wait(&(ctx->ctrl->lleno)) == -1)
		{
			*err = errno;
			return (SCB_SEMPH);
		}
	}

	if (sem_wait(&(ctx->ctrl->con_carrera)) == -1)
	{
		*err = errno;
		sem_post(&(ctx->ctrl->lleno));
		return (SCB_SEMPH);
	}

	if (ctx->ctrl->qtd == 0)
		ret = SCB_VACIO;
	else
	{
		copyMessage(mensaje, ctx->mensajes + (ctx->ctrl->cola * ctx->ctrl->largo_mensaje));

		ctx->ctrl->cola = (ctx->ctrl->cola + 1) % ctx->ctrl->capacidad;
		ctx->ctrl->qtd--;
	}
	//Se libera condicion de carrera y se disminuye los vacios
	sem_post(&(ctx->ctrl->con_carrera));
	sem_post(&(ctx->ctrl->vacio));

	*err = 0;
	return (ret);
}

//Función para destruir todo

errores destruir_buffer(char *name, int *err)
{
	int ret = 0;
	buffer ctx;
	errores scberr;

	int fdshmem = 0;
	int sf = 0, se = 0, sb = 0;
	size_t szshmem = 0;
	void *shmem = NULL;
	buffer_control scbInf;

	/*get_info_buffer(char *name, buffer_control *inf, int *semlleno, int *semvacio, int *semcon_carrera,int *semconsumidores,int *semproductores, int *err);*/
	//se busca la infor de ese buffer
	// se consigue la info actual  de ese espacio de memoria
	scberr = get_info_buffer(name, &scbInf, &sf, &se, &sb, err);
	//se crea el file descriptor con mmap
	fdshmem = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
	if (fdshmem == -1)
	{
		*err = errno;
		return (SCB_SHMEM);
	}

	//Se modifica estado de finalizador
	if (scberr != SCB_OK)
	{
		return (scberr);
	}
	//se trae el control
	buffer_control *puntero = mmap(0, sizeof(buffer_control), PROT_READ | PROT_WRITE, MAP_SHARED, fdshmem, 0);

	int finalizar = 1;
	(*puntero).finalizar = finalizar;
	sleep((*puntero).maxEspera + 3);
	if ((*puntero).initFinalizado)
	{
		*err = 0;
		//se obtiene el buffer
		scberr = get_buffer(&ctx, name, err);
		if (scberr != SCB_OK)
			return (scberr);
		// se destruyen los semaforos
		ret = sem_destroy(&(ctx.ctrl->con_carrera)) | sem_destroy(&(ctx.ctrl->vacio)) | sem_destroy(&(ctx.ctrl->lleno));

		if (ret != 0)
		{
			*err = errno;
			return (SCB_SEMPH);
		}

		if (shm_unlink(ctx.name) == -1)
		{
			*err = errno;
			return (SCB_SHMEM);
		}
	}

	return (SCB_OK);
}

/**
 * Distribución exponencial
 * Valor de lambda se relaciona con el maximo valor posible.
 * Se usa time para obtener un valor inicial. 
 * @param lambda Valor de lambda se relaciona con el maximo valor posible.
 */
double ran_expo(double lambda)
{

	// long idum = rand(); // TODO used var
	double u;
	u = rand() / (RAND_MAX + 1.0);
	return -log(1 - u) / lambda;
}
/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_small(double mean)
{

	double L = exp(-mean);
	double p = 1;
	int result = 0;
	do
	{
		result++;
		p *= drand48();
	} while (p > L);
	result--;
	return result;
}

/**
 * Función lambda logarítmica
 * @param xx variable independiente
 */
double lgamma(double xx)
{
	double pi = 3.14159265358979;
	double xx2 = xx * xx;
	double xx3 = xx2 * xx;
	double xx5 = xx3 * xx2;
	double xx7 = xx5 * xx2;
	double xx9 = xx7 * xx2;
	double xx11 = xx9 * xx2;
	return xx * log(xx) - xx - 0.5 * log(xx / (2 * pi)) +
		   1 / (12 * xx) - 1 / (360 * xx3) + 1 / (1260 * xx5) - 1 / (1680 * xx7) +
		   1 / (1188 * xx9) - 691 / (360360 * xx11);
}

/**
 * Tomado de https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/
 * @param mean La media de las ocurrencias
 */
int poissrnd_large(double mean)
{
	double r;
	double x, m;
	double pi = 3.14159265358979;
	double sqrt_mean = sqrt(mean);
	double log_mean = log(mean);
	double g_x;
	double f_m;

	do
	{
		do
		{
			x = mean + sqrt_mean * tan(pi * (drand48() - 1 / 2.0));
		} while (x < 0);
		g_x = sqrt_mean / (pi * ((x - mean) * (x - mean) + mean));
		m = floor(x);
		f_m = exp(m * log_mean - mean - lgamma(m + 1));
		r = f_m / g_x / 2.4;
	} while (drand48() > r);
	return (int)m;
}

/**
 * Función de generar números aleatorios 
 * basado en poisson
 */
int poissrnd(double mean)
{
	if (mean < 60)
	{
		return poissrnd_small(mean);
	}
	else
	{
		return poissrnd_large(mean);
	}
}
