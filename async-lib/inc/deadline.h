//
// Created by maksprotsyk on 05.06.22.
//

#ifndef SERVER_DEADLINE_H
#define SERVER_DEADLINE_H

#include <chrono>
#include <functional>

struct Deadline {
    std::chrono::system_clock::time_point deadline;
    int fd{};
};

bool operator<(const Deadline& deadline1, const Deadline& deadline2);

#endif //SERVER_DEADLINE_H
