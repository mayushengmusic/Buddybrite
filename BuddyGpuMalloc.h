//
// Created by jaken on 18-10-28.
//

#ifndef BUDDYBRITE_BUDDYGPUMALLOC_H
#define BUDDYBRITE_BUDDYGPUMALLOC_H

#include <iostream>
#define DEFAULT_LEVEL 10
#define MEM_MALLOC( ptr, size ) do{ \
      (ptr) = (void *)malloc((size_t)( size )); \
        }while(0);

#define MEM_FREE free

#define BUDDY_BLOCK 4194304

namespace buddy{
    int pool_init(int level = DEFAULT_LEVEL); //the buddy alloctor memory size will 2^level * 4MiB on the CUDA GPU
    int pool_malloc(void **ptr,int size);//malloc will allocate 2^(x-1) * 4MiB (2^(x-1)<=size<2^(x))
    void pool_free(void *ptr);
    void pool_destory();
}


#endif //BUDDYBRITE_BUDDYGPUMALLOC_H
