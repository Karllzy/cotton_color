//
// Created by zjc on 24-11-18.
//

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
template <typename Func>
// Time measurement function
void measure_execution_time(Func func) {
    std::chrono::time_point<std::chrono::steady_clock> start;
    start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Function execution time: " << duration.count() << " milliseconds" << std::endl;
}

#endif //UTILS_H
