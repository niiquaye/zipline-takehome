#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <error.h>
#include <limits.h>
#include <string.h>

#include "pool_alloc.h"

#define SIXTY_FOUR_BIT_PAGE (1 << 16)                           // 65536 or 2^16 - 64kB

/*
a struct that will contain the memory address and the start and end of each block
*/
typedef struct{
    void* ptr;                                                   // pointer to start of memory block
    size_t start;                                                // start index of memory block
    size_t end;                                                  // end index of memory block
}memory_block;

static uint8_t g_pool_heap[SIXTY_FOUR_BIT_PAGE];                 // 64KB of memory - heap memory
static size_t block_sizes_array[MAX_NUMBER_OF_BLOCK_SIZES];      // max number of sizes will be 10 - thus there will be a max of 10 different types of sizes
static size_t position;                                          // a pointer to keep track of current position in the heap
static memory_block allocated_memory_array[SIXTY_FOUR_BIT_PAGE]; // a list to keep track of allocated memory
static memory_block freed_memory_array[SIXTY_FOUR_BIT_PAGE];     // a list to keep track of all freed memory

/*
helper function to easily determine which argument is the min and return it
*/
inline static size_t min(const size_t a, const size_t b) {
    return (a < b)? a : b; 
}

/********************************************************************
 *  Name: 
 *      pool_init
 *  
 *  Parameters: 
 *      block_sizes - an array that contains the different sizes 
 *                    to be allocated 
 *      
 *      block_size_count - the number of elements the block_size array
 *                         contains - Note: this must not be greater
 *                         than the macro: MAX_NUMBER_OF_BLOCK_SIZES
 * 
 * 
 *  Return:
 *      returns true if pool intialized successfully, 
 *      else return false
 * 
 * 
 *  Description:
 *      intializes the pool of memory to be used
 * 
 * *****************************************************************/
bool pool_init(const size_t block_sizes[], size_t block_size_count){
    // error checking
    if(!block_sizes){
        fprintf(stderr, "There were no block sizes provided\n");
        return false;
    }
    if(block_size_count > MAX_NUMBER_OF_BLOCK_SIZES){
        fprintf(stderr, "The number of block sizes exceeds the maximum allowable size of %i\n", MAX_NUMBER_OF_BLOCK_SIZES);
        return false;
    }

    // intialize heap pointer to 0
    position = 0;

    // intialize all arrays
    memset(g_pool_heap, 0, sizeof(g_pool_heap));
    memset(block_sizes_array, 0, sizeof(block_sizes_array));
    memset(allocated_memory_array, 0, sizeof(allocated_memory_array));
    memset(freed_memory_array, 0, sizeof(freed_memory_array));

    //memcpy((void*)block_sizes_array, (const void*)block_sizes, block_size_count);

    // copy block sizes into block_sizes_array to be used later on in the program
    size_t i;
    for(i = 0; i < block_size_count; i++){
        block_sizes_array[i] = block_sizes[i];
    }
    
    // if the number of block sizes given is less than the max number of block sizes, 
    // fill the rest of the unused positions with ULONG_MAX
    if(block_size_count < MAX_NUMBER_OF_BLOCK_SIZES){
        for(i; i < MAX_NUMBER_OF_BLOCK_SIZES; i++){
            block_sizes_array[i] = ULONG_MAX;
        }
    }

    return true;
}

/********************************************************************
 *  Name:
 *      pool_malloc
 * 
 *  Parameters:
 *      n - the number of bytes to allocate
 * 
 *  Returns:
 *      returns a void pointer on success,
 *      else it return NULL
 * 
 *  Description:
 *      Allocates the at least n bytes of memory,
 *      this function first checks if any recently freed 
 *      memory is available to allocate if the heap pointer 
 *      is at the end of the heap. When memory is allocated
 *      it gets added to an allocated memory array that keeps
 *      track of all allocated memory. If the situation arises
 *      where freed memory gets reallocated it will return the 
 *      freed memory and update its contents in the allocated memory
 *      array.    
 * 
 * *****************************************************************/

void* pool_malloc(size_t n){

    // trivially return NULL if zero bytes requested - do not waste time on computations
    if(n == 0) return NULL;

    size_t closest_size_available = ULONG_MAX; 

    // linearly determine the closest blocksize of bytes to allocate
    for(size_t i = 0; i < MAX_NUMBER_OF_BLOCK_SIZES; i++){
        int res = block_sizes_array[i] - n;
        if(res == 0){ // block size requested is within the block size array

            closest_size_available = block_sizes_array[i];
            break;

        }else if(res > 0){ // block size requested is not within the block size array

            // find smallest block size closest to the requested size and update the closest_size_available
            closest_size_available = min(closest_size_available, block_sizes_array[i]);

        }else{
            continue;
        }
    }
    
    // trivial error checking 
    if(closest_size_available == ULONG_MAX){
        fprintf(stderr, "The requested memory size could not be properly determined\n");
        return NULL;
    }

    // check if there is enough memory to allocate for the requested block size
    if(position + closest_size_available - 1 > SIXTY_FOUR_BIT_PAGE ){

        size_t free_memory_index = ULONG_MAX;

        // first check lineraly if there is any freed memory from the freed_memory_array - run a check to ensure it is the smallest possible block

        for(size_t i = 0; i < SIXTY_FOUR_BIT_PAGE; i++){

            // calculate block size of freed memory 
            size_t freed_memory_size_block = freed_memory_array[i].end - freed_memory_array[i].start + 1; 

            if( freed_memory_size_block >= closest_size_available){

                // we have found the smallest block of freed memory that can be reallocated 
                // return this memory and remove from freed_memory_array and add it to the allocated_memory_array
                free_memory_index = min(free_memory_index, i);

            }
        }

        // failed to find any freed memory to reallocate and there is not enough memory on the heap that was requested - thus no more space
        if(free_memory_index == ULONG_MAX){
            fprintf(stderr, "Not enough memory present in the heap");
            return NULL;
        }

        // get a pointer to the freed memory that will be returned
        void* memory_to_return = (void*)freed_memory_array[free_memory_index].ptr;


        // add the freed memory to the allocated memory array
        allocated_memory_array[free_memory_index].ptr = memory_to_return;
        allocated_memory_array[free_memory_index].start = freed_memory_array[free_memory_index].start;
        allocated_memory_array[free_memory_index].end = freed_memory_array[free_memory_index].end;

        // reset the freed memory contents in the freed_memory array
        freed_memory_array[free_memory_index].ptr = 0;
        freed_memory_array[free_memory_index].start = 0;
        freed_memory_array[free_memory_index].end = 0;

        // return the newly allocated memory
        return memory_to_return;
        
    }

    
    // allocate memory from heap
    void* memory_to_return = (void*)&g_pool_heap[position];

    // put allocated memory in free table
    allocated_memory_array[position].ptr = memory_to_return;
    allocated_memory_array[position].start = position;
    allocated_memory_array[position].end = position + closest_size_available - 1; // subtract 1 because of 0 start index

    // update current position on the heap
    position += closest_size_available - 1; // subtract 1 because of 0 start index

    return memory_to_return;
}

/********************************************************************
 *  Name:
 *      pool_free
 * 
 *  Parameters:
 *      ptr - void pointer to be freed
 * 
 *  Returns:
 *      nothing
 * 
 *  Description:
 *      This reallocates any allocated memory to the freed memory list
 *      which will allow it to be used in the future by the 'pool_malloc' 
 *      function
 * 
 * *****************************************************************/
void pool_free(void* ptr){

    bool is_memory_found = false;

    // check if pointer is null
    if(!ptr){
        fprintf(stderr, "Pointer is Null\n");
        return;
    }

    // iterate linearly through the allocated memory array, find the memory and then reallocate
    size_t i;
    for(i = 0; i < SIXTY_FOUR_BIT_PAGE; i++){
        if(allocated_memory_array[i].ptr == ptr){
            is_memory_found = true;
            break; // we have found the memory that needs to be freed, simply add the memory to the freed list
        }
    }

    // check that the memory to be freed has been found
    if(is_memory_found){
        // add memory to freed list 
        freed_memory_array[i].ptr = allocated_memory_array[i].ptr;
        freed_memory_array[i].start = allocated_memory_array[i].start;
        freed_memory_array[i].end = allocated_memory_array[i].end;


        // printf("size of freed block: %lu\n", allocated_memory_array[i].end - allocated_memory_array[i].start + 1);

        // reset contents in the allocated_memory_array index that was just "freed"
        allocated_memory_array[i].ptr = 0;
        allocated_memory_array[i].start = 0;
        allocated_memory_array[i].end = 0;

        return;

    }else{
        fprintf(stderr, "Memory requested to be freed does not exist\n");
        return;
    }
    

}
