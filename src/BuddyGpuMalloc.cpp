//
// Created by jaken on 18-10-28.
//

#include "BuddyGpuMalloc.hpp"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <math.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <mutex>

#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) ( ((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))



static unsigned fixsize(unsigned size) {
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return size+1;
}

struct buddyAlg {
    unsigned size;
    unsigned longest[1];
};



struct buddyAlg* buddyAlg_new( int size ) {
    struct buddyAlg* self;
    unsigned node_size;
    int i;

    if (size < 1 || !IS_POWER_OF_2(size))
        return NULL;

    self = (struct buddyAlg*)malloc( 2 * size * sizeof(unsigned));
    self->size = size;
    node_size = size * 2;

    for (i = 0; i < 2 * size - 1; ++i) {
        if (IS_POWER_OF_2(i+1))
            node_size /= 2;
        self->longest[i] = node_size;
    }
    return self;
}

void buddyAlg_destroy( struct buddyAlg* self) {
    if(self!=NULL)
        free(self);
}

int buddyAlg_alloc(struct buddyAlg* self, int size) {
    unsigned index = 0;
    unsigned node_size;
    unsigned offset = 0;

    if (self==NULL)
        return -1;

    if (size <= 0)
        size = 1;
    else if (!IS_POWER_OF_2(size))
        size = fixsize(size);

    if (self->longest[index] < size)
        return -1;

    for(node_size = self->size; node_size != size; node_size /= 2 ) {
        if (self->longest[LEFT_LEAF(index)] >= size)
            index = LEFT_LEAF(index);
        else
            index = RIGHT_LEAF(index);
    }

    self->longest[index] = 0;
    offset = (index + 1) * node_size - self->size;

    while (index) {
        index = PARENT(index);
        self->longest[index] =
                MAX(self->longest[LEFT_LEAF(index)], self->longest[RIGHT_LEAF(index)]);
    }

    return offset;
}

void buddyAlg_free(struct buddyAlg* self, int offset) {
    unsigned node_size, index = 0;
    unsigned left_longest, right_longest;

    assert(self && offset >= 0 && offset < self->size);

    node_size = 1;
    index = offset + self->size - 1;

    for (; self->longest[index] ; index = PARENT(index)) {
        node_size *= 2;
        if (index == 0)
            return;
    }

    self->longest[index] = node_size;

    while (index) {
        index = PARENT(index);
        node_size *= 2;

        left_longest = self->longest[LEFT_LEAF(index)];
        right_longest = self->longest[RIGHT_LEAF(index)];

        if (left_longest + right_longest == node_size)
            self->longest[index] = node_size;
        else
            self->longest[index] = MAX(left_longest, right_longest);
    }
}

int buddyAlg_size(struct buddyAlg* self, int offset) {
    unsigned node_size, index = 0;

    assert(self && offset >= 0 && offset < self->size);

    node_size = 1;
    for (index = offset + self->size - 1; self->longest[index] ; index = PARENT(index))
        node_size *= 2;

    return node_size;
}

static struct buddyAlg * buddyAlloctor(NULL);
static std::atomic_bool initCheck(false);
static std::mutex mux;
static void * start_point(NULL);
static int size(0);



int buddy::pool_init(int level) {
    std::lock_guard<std::mutex> lock(mux);


    if(initCheck)
        return 0;

    size = 1 <<level;
    std::cout<<"x: "<<size<<std::endl;
    buddyAlloctor = buddyAlg_new(size);
    if(buddyAlloctor == NULL)
        return -1;

    MEM_MALLOC(start_point,size*BUDDY_BLOCK);

    std::cout<<"start points: "<<start_point<<std::endl;
    if(start_point == NULL) {
        buddyAlg_destroy(buddyAlloctor);
        buddyAlloctor = NULL;
        return -1;
    }
    initCheck.store(true);
    return 0;
}

int buddy::pool_malloc(void **ptr,int size) {
    if(!initCheck)
        return -1;
    std::lock_guard<std::mutex> lock(mux);
    int offset = buddyAlg_alloc(buddyAlloctor,size);
    std::cout<<"offset: "<<offset<<std::endl;
        if(offset < 0)
            return -1;
        else {
            *ptr = (long long *)start_point + BUDDY_BLOCK * offset/(sizeof(long long));
        }
        return 0;
}

void buddy::pool_free(void *ptr) {
    if(ptr != NULL)
    {
        std::lock_guard<std::mutex> lock(mux);

        buddyAlg_free(buddyAlloctor,((u_int8_t *)ptr-(u_int8_t *)start_point)/BUDDY_BLOCK);
    }
}

void buddy::pool_destory() {

    std::lock_guard<std::mutex> lock(mux);
    if(initCheck)
    {
        buddyAlg_destroy(buddyAlloctor);
        buddyAlloctor=NULL;
        if(!start_point)
            MEM_FREE(start_point);
        start_point=NULL;
        initCheck.store(false);
    }
}

