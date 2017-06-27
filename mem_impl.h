/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifndef MEM_IMPL
#define MEM_IMPL

#include <cstddef>

/* memset implementation which counters agressive dead-code elimination by some compilers */
volatile void * my_memset(void * ptr, int value, size_t num);

volatile void * my_memclr(void * ptr, size_t num);

#endif // !MEM_IMPL