#ifndef INIT_MSG
#define INIT_MSG

#include <iostream>
#include <string>
#include <chrono>

// class that prints out a message when the object is instantiated,
// and the same message with suffix " Done!" when exiting scope
// it also calculates time span
struct MyWatch
{
    MyWatch(std::string function_name)
        : _msg(std::move(function_name)), _start(std::chrono::high_resolution_clock::now())
    {
        std::cout << "Initializing " << _msg << "..." << std::endl;
    }

    ~MyWatch()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - _start).count();
        std::cout << "Initializing " << _msg << "... Done! ("
                  << duration << " ms)" << std::endl;
    }

    std::string _msg;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

#endif
