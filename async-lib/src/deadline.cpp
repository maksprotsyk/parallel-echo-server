//
// Created by maksprotsyk on 05.06.22.
//
#include "deadline.h"

bool operator<(const Deadline& deadline1, const Deadline& deadline2) {
    return deadline1.deadline >= deadline2.deadline;
}