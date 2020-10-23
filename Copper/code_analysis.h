//
//  code_analysis.h
//  CopperXcode
//
//  Created by Niels Valentin Abildskov on 10/09/2020.
//  Copyright © 2020 Niels Valentin Abildskov. All rights reserved.
//
#include <chrono>
#include <iostream>
#ifndef code_analysis_h
#define code_analysis_h


#endif /* code_analysis_h */

//#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
std::printf("'%s' - Failed\n",#n); \
std::printf("On %s \n",__DATE__); \
std::printf("At %s \n",__TIME__); \
std::printf("In File %s \n",__FILE__); \
std::printf("At Line %d\n",__LINE__); \
exit(1);}
#endif



class Timer { // Will start a timer from initialization until end of code block
public:
    Timer() {
        m_Starttime = std::chrono::high_resolution_clock::now();
    }
    ~Timer() {
        m_Endtime = std::chrono::high_resolution_clock::now();
        Stop();
    }

    void Stop() {
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Starttime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_Endtime).time_since_epoch().count();
        double elapsed = end - start;
        std::cout << "Elapsed: " << elapsed << " [us], (" << elapsed / 1000 << " [ms])" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Starttime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Endtime;

};
