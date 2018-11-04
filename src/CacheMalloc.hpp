#include "BuddyGpuMalloc.hpp"

#define CACHE_BLOCK 512 //cache block size is 512byte


namespace cache{
    int cache_malloc(void ** ptr, size_t size);
    int cache_free(void *ptr);
}
