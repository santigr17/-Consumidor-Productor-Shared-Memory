#ifndef __BUFFERSHARED_MENSAJE_SAMPLE__
#define __BUFFERSHARED_MENSAJE_SAMPLE__


#include <stdio.h>
#include <string.h>

typedef struct _element_t{
	int a;
	float b;
	char c[10];
}element_t;

void * copyElement(void *dst, const void *src)
{
	return(memcpy(dst, src, sizeof(element_t)));
}

#endif
