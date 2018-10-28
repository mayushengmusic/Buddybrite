#include "BuddyGpuMalloc.h"
#include <random>
#include <thread>
#include <assert.h>

void cell(){
    std::random_device gen;
    for(int i=0;i<10;++i)
    {

        void * ptr;
        int size = gen()%32;
        std::cout<<"size: "<<size<<std::endl;
        if(buddy::pool_malloc(&ptr,size)<0)
            std::cout<<"error"<<std::endl;

        u_long flag = gen();
        u_int64_t * cusor = (u_int64_t *)ptr;
        for(u_int64_t j=0;j<size*BUDDY_BLOCK/sizeof(u_int64_t);++j)
        {
            (*cusor) = flag;
            ++cusor;
        }
        cusor = (u_int64_t *)ptr;
        std::this_thread::sleep_for(std::chrono::seconds(gen()%3));
        for(u_int64_t j=0;j<size*BUDDY_BLOCK/sizeof(u_int64_t);++j)
        {
            assert(*cusor==flag);
            ++cusor;
        }

        //buddy::pool_free(ptr);
    }
}


int main() {
    std::cout<<sizeof(void *)<<std::endl;
    std::cout<<buddy::pool_init(10)<<std::endl;
    cell();
    /*
    std::vector<std::thread> ths;
    ths.reserve(36);

    for(int i=0;i<32;++i) {
        ths.emplace_back(cell);
    }

    for(int i=0;i<32;++i) {
        ths[i].join();
    }
    */
    buddy::pool_destory();

    return 0;
}