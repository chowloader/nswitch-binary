#include "chowjs.h"

// Semaphore Synchronization for Multithreading Allocation

atomic_int_fast64_t semaphore = 1;

void semaphore_wait(){
  while(!semaphore){};
  semaphore--;
}

void semaphore_signal(){
  semaphore++;
}

void *qjs_malloc(JSMallocState *state, size_t size){
  semaphore_wait();

  void *ptr = native_qjs_malloc(state, size);

  semaphore_signal();

  return ptr;
}

void qjs_free(JSMallocState *state, void *ptr){
  semaphore_wait();

  native_qjs_free(state, ptr);
  
  semaphore_signal();
}

void *qjs_realloc(JSMallocState *state, void *ptr, size_t size){
  semaphore_wait();

  void *nptr = native_qjs_realloc(state, ptr, size);

  semaphore_signal();

  return nptr;
}
