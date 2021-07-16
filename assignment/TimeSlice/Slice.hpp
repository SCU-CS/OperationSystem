#ifndef __TIME_SLICE_
#define __TIME_SLICE_ 1
#include<iostream>
#include<chrono>
#include<thread>
#include<atomic>

size_t singleThreadSliceMeasure(size_t times);
#ifdef _WIN32
    #include<windows.h>
#endif

class semaphore{
        // bool hold;
        std::atomic_bool hold;
        semaphore():semaphore(true){}
        semaphore(bool have):hold(have){}
    public:
        static semaphore& object(){
            static semaphore signal;
            return signal;
        }
        // busy wait
        static void get(){
            bool temp=false;
            while(!object().hold.exchange(temp));
        }
        static void release(){
            object().hold=true;
        }
};



/**
 * @brief return the measured OS time slice
 * @return the average time of the time slice in nanosecs
 * @warning only support Windows and Linux/Unix 
 * maybe?
*/
size_t TimeSliceMeasure(size_t times=64,bool sout=false){
    // const unsigned int threads=std::thread::hardware_concurrency()
    //     +(std::thread::hardware_concurrency()>>1);
    const unsigned int threads=std::thread::hardware_concurrency()+1;
    // std::cout<<"threads:"<<threads<<std::endl;
    // const unsigned int testThreads=threads+threads>1;
    size_t sum=0,time=0;
    #ifdef _WIN32
        /**
         * @brief return the measured windows time slice
         * @return the average time of the time slice in nanosecs
        */
        std::chrono::_V2::system_clock::time_point beg,end;
        while(times--){
            beg=std::chrono::system_clock::now();
            // `Sleep` only guarantees that the thread will sleep for at least that length of time.
            Sleep(1);
            end=std::chrono::system_clock::now();
            sum+=(end-beg).count();
        }
        time=sum/times;
    // #elif __linux__
    #else
        // race condition maybe
        using milli=std::chrono::duration<long double, std::milli>;
        int actual=0;
        const auto &&second=std::chrono::duration<long double>{1};
        const auto &&milsec=std::chrono::duration<long double, std::milli>{1};
        auto fn=[&](){
            // std::this_thread::sleep_for(second);
            std::chrono::_V2::system_clock::time_point stl=std::chrono::system_clock::now();
            std::chrono::_V2::system_clock::time_point beg=std::chrono::system_clock::now();
            std::chrono::_V2::system_clock::time_point end=std::chrono::system_clock::now();
            while((end-stl)<second){
                end=std::chrono::system_clock::now();
                if((end-beg)>milsec){
                    semaphore::get();
                    sum+=(end-beg).count();
                    ++actual;
                    semaphore::release();
                    beg=std::chrono::system_clock::now();
                }else{
                    beg=end;
                }
            }
        };
        for(int i=0;i<threads;++i){
            std::thread tr(fn);
            tr.detach();
        }
        std::this_thread::sleep_for(std::chrono::duration<long double>{2});
        time=sum/actual;
    #endif
    if(sout){
        std::cout<<"Time Slice:"<<time/1000000<<"ms"<<std::endl;
    }
    return time;
}

size_t singleThreadSliceMeasure(size_t times){
    const auto &&milsec=std::chrono::duration<long double, std::milli>{1};
    size_t temp=times,sum=0;
    std::chrono::_V2::system_clock::time_point beg=std::chrono::system_clock::now();
    std::chrono::_V2::system_clock::time_point end=std::chrono::system_clock::now();
    while(temp){
        end=std::chrono::system_clock::now();
        if((end-beg)>milsec){
            semaphore::get();
            sum+=(end-beg).count();
            --temp;
            semaphore::release();
            // puts("Yep");
            beg=std::chrono::system_clock::now();
        }else{
            beg=end;
        }
    }
    return sum/times;
}
#endif//__TIME_SLICE_