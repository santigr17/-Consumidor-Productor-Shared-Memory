#ifndef _BUFFERCIRCULAR_H
#define _BUFFERCIRCULAR_H

# include <stdint.h>
# include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>


/*

Código hecho por Esteban Ferarios , basado en: https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/

*/

//Estructura de un buffer circular
typedef struct circular_buf_t circular_buf_t;

/*Para evitar problemas con el puntero
 Handle type, Manera en que los usuarios interacturán con el API*/
typedef circular_buf_t* cbuf_handle_t;

/*
Primero, debemos pensar en cómo los usuarios interactuarán con un búfer circular:

    -Necesitan inicializar el contenedor de búfer circular con un búfer y tamaño
    -Necesitan destruir un contenedor de búfer circular
    -Necesitan restablecer el contenedor de búfer circular
    -Necesitan poder agregar datos al búfer
    -Deben poder obtener el siguiente valor del búfer
    -Necesitan saber si el búfer está lleno o vacío
    -Necesitan saber el número actual de elementos en el búfer
    -Necesitan saber la capacidad máxima del búfer

*/

//Se utilizara uint8_t. Este es un tipo entero con un ancho de exactamente 
//8, 16, 32 o 64 bits. Para los tipos con signo, los valores negativos 
//se representan usando el complemento de 2. Sin partes de relleno.


/// Pide el tamaño del buffer
/// Retorna un manejadore de buffer(handle)
cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size);

/// Liberar la estructura buffer circular.
/// No libera la data el buffer; el dueño es el responsable de esto.
void circular_buf_free(cbuf_handle_t cbuf);

/// Resete el buffer a vacio, cabeza == cola
void circular_buf_reset(cbuf_handle_t cbuf);

/// Poner versión 1 continúa agregando datos si el búfer está lleno
/// Dato viejos son sobreescritos
void circular_buf_put(cbuf_handle_t cbuf, uint8_t data);

/// Put Versión 2 rechaza datos nuevos si el búfer está lleno
/// Retorna 0 si es exitoso, -1 si el buffer está lleno
int circular_buf_put2(cbuf_handle_t cbuf, uint8_t data);

/// Devuelve el valor del buffer
/// Retorna 0 si fue un exito, -1 si el buffer está vacío

int circular_buf_get(cbuf_handle_t cbuf, uint8_t * data);

/// Retorna true si el buffer está vacío
bool circular_buf_empty(cbuf_handle_t cbuf);

/// Retorna true si el buffer está lleno
bool circular_buf_full(cbuf_handle_t cbuf);

/// Retorna la capacidad máxima del buffer
size_t circular_buf_capacity(cbuf_handle_t cbuf);

/// Retorna el número actual de elementos en el buffer
size_t circular_buf_size(cbuf_handle_t cbuf);


/*
Tanto los casos "lleno" como "vacío" del búfer circular se ven iguales: 
el puntero de la cabeza y la cola son iguales. 
Hay dos enfoques para diferenciar entre lleno y vacío:



1-gastar un slot del buffer:

Estado lleno es cola + 1 == cabeza;
Estado vacío es cabeza == cola;

2-Usar una bandera boleana y lógica adicional para diferentes estados:
 
 Estado lleno is lleno
 Estado vacío es (cabeza ==cola) y !lleno

 En lugar de desperdiciar un espacio de datos potencialmente valioso, 
 la implementación a continuación utiliza el indicador bool. 
 El uso del indicador requiere lógica adicional en las rutinas get y put 
 para actualizar el indicador. 


*/



#endif