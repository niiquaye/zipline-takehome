#ifndef __POOL_ALLOC_H__
#define __POOL_ALLOC_H__

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUMBER_OF_BLOCK_SIZES 10  // max number of block sizes will be 10
                                      // 10 different sizez is a sufficent amount sizes to allocate
                                      // i.e. 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 

#ifdef __cplusplus
extern "C"
{
#endif

// Return true on success, false on failue
bool pool_init(const size_t block_sizes[], size_t block_size_count);

// Allocates n bytes
// Returns pointer to allocate memory on success, NULL on failure
void* pool_malloc(size_t n);

// Release allocation to by ptr
void pool_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /*__POOL_ALLOC_H__*/
