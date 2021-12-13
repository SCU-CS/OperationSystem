#ifndef _CONQUER_HPP
#define _CONQUER_HPP 1
#include<chrono>
#include<thread>
#include<iostream>
// make CPU at least ratio percent full
using milli=std::chrono::duration<long double, std::milli>;
class ConquerFactory{
    public:
    constexpr static milli shared_time=milli{20};
    constexpr static int st=20;
    class Conquer{
        public:
        static void exec(milli rt,milli slp,const milli&ms){
            auto&& beg=std::chrono::steady_clock::now();
            auto&& cur=std::chrono::steady_clock::now();
            auto pos=cur;
            while((cur-beg)<ms){
                pos=cur;
                while((cur-pos)<rt){
                    cur=std::chrono::steady_clock::now();
                }
                std::this_thread::sleep_for(slp);
                cur=std::chrono::steady_clock::now();
            }
        }
        void start(int percent,const milli&ms=milli{10000},bool multiple=false)const noexcept{
            milli shared_time=milli{20};
            int m=percent/5;
            const auto slp=milli{st-m};
            const auto rt=shared_time-slp;
            if(multiple){
                for(int i=0;i<std::thread::hardware_concurrency();++i){
                    std::thread td(exec,rt,slp,ms);
                    td.detach();
                }
            }
        }
    };
    static Conquer& build(){
        static Conquer cq;
        return cq;
    }
};
#endif//CONQUER_HPP
