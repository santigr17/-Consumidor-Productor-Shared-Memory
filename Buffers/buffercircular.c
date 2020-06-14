# include <stdint.h>
# include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "buffercircular.h"





// definición de la structura del buffer


struct circular_buf_t {
	uint8_t * buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
};



cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size)
{
	assert(buffer && size);

	cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
	assert(cbuf);

	cbuf->buffer = buffer;
	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

	return cbuf;
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}


void circular_buf_free(cbuf_handle_t cbuf)
{
	assert(cbuf);
	free(cbuf);
}


bool circular_buf_full(cbuf_handle_t cbuf)
{
	assert(cbuf);

    return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf)
{
	assert(cbuf);

    return (!cbuf->full && (cbuf->head == cbuf->tail));
}



size_t circular_buf_size(cbuf_handle_t cbuf)
{
	assert(cbuf);

	size_t size = cbuf->max;

	if(!cbuf->full)
	{
		if(cbuf->head >= cbuf->tail)
		{
			size = (cbuf->head - cbuf->tail);
		}
		else
		{
			size = (cbuf->max + cbuf->head - cbuf->tail);
		}
	}

	return size;
}


static void advance_pointer(cbuf_handle_t cbuf)
{
	assert(cbuf);

	if(cbuf->full)
   	{
		cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	}

	cbuf->head = (cbuf->head + 1) % cbuf->max;
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
	assert(cbuf);

	cbuf->full = false;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}


void circular_buf_put(cbuf_handle_t cbuf, uint8_t data)
{
	assert(cbuf && cbuf->buffer);

    cbuf->buffer[cbuf->head] = data;

    advance_pointer(cbuf);
}


int circular_buf_put2(cbuf_handle_t cbuf, uint8_t data)
{
    int r = -1;

    assert(cbuf && cbuf->buffer);

    if(!circular_buf_full(cbuf))
    {
        cbuf->buffer[cbuf->head] = data;
        advance_pointer(cbuf);
        r = 0;
    }

    return r;
}


int circular_buf_get(cbuf_handle_t cbuf, uint8_t * data)
{
    assert(cbuf && data && cbuf->buffer);

    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->tail];
        retreat_pointer(cbuf);

        r = 0;
    }

    return r;
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
	assert(cbuf);

	return cbuf->max;
}

//probando ejemplo

int main() {

//Se pide espacio y se inicia buffer
   uint8_t * buffer  = malloc(10 * sizeof(uint8_t));
    cbuf_handle_t cbuf = circular_buf_init(buffer, 10);

// Se setean banderas full y empty
   
    bool full = circular_buf_full(cbuf);
    bool empty = circular_buf_empty(cbuf);

    printf("Está el buffer vacío?: %i\n", empty);
    printf("Está el buffer lleno?: %i\n", full);
    printf("Tamaño actual del buffer: %zu\n", circular_buf_size(cbuf));
    printf("Capacidad máxima del buffer: %zu\n", circular_buf_capacity(cbuf));





    //Se crea un dato
     uint8_t data = 8;
     //se inserta el dato
    circular_buf_put( cbuf, data);
    printf("Se insertó un dato \n");
  
    printf("Está el buffer vacío?: %i\n", empty);
    printf("Está el buffer lleno?: %i\n", full);
    printf("Tamaño actual del buffer: %zu\n", circular_buf_size(cbuf));
    printf("Capacidad máxima del buffer: %zu\n", circular_buf_capacity(cbuf));



   
   
   
   //Se libera memoria 
   free(buffer);
   circular_buf_free(cbuf);
   return 0;
}













