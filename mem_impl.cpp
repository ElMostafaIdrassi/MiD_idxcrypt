/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#include "mem_impl.h"

volatile void * my_memset(void * ptr, int value, size_t num) {
	volatile unsigned char * buf;
	buf = (volatile unsigned char *)ptr;

	while (num)
		buf[--num] = (unsigned char)value;

	return (volatile void *)ptr;
}

volatile void * my_memclr(void * ptr, size_t num) {
	return (my_memset(ptr, 0, num));
}

