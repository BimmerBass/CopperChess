#include <chrono>
#include <iostream>


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
        auto start = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_Starttime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_Endtime).time_since_epoch().count();
        double elapsed = end - start;
        std::cout << "Elapsed: " << elapsed << " [ns], (" << elapsed / 1000 << " [us])" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Starttime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Endtime;

};
