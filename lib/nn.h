#ifndef __NN_H
#define __NN_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

void *nn_os_GetCurrentThread();
void nn_os_SetThreadCoreMask(void* thread, int core, uint64_t mask);

#endif
