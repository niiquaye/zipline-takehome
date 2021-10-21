#include <stdio.h>
#include <stdlib.h>

/*
Test program
*/

#include "pool_alloc.h"

int main(int argc, char* argv[]){
    (void)argc;
    (void)argv;


    size_t array[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024};

    if(!pool_init(array, sizeof(array)/sizeof(array[0]) )){
        return EXIT_FAILURE;
    }

    uint8_t* newG =(uint8_t*)pool_malloc(97);
    uint8_t* newF = (uint8_t*)pool_malloc(25);

    if(newG == NULL){
        printf("ooooh");
    }

    for(int i = 0; i < 97; i++){
        printf("%u\n", newG[i]);
    }

    for(int i = 0; i < 25; i++){
        printf("%u\n", newF[i]);
    }

    printf("%p\n", &newG[0]);
    
    pool_free((void*)newG);
    pool_free((void*)newF);

    printf("hello world\n");
}
