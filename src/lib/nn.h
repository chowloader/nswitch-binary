#ifndef __NN_H
#define __NN_H

#include <stdint.h>
#include <stdbool.h>

extern void *nn_os_GetCurrentThread();
extern void nn_os_SetThreadCoreMask(void* thread, int core, uint64_t mask);

#endif
