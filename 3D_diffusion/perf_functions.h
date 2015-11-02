#ifndef PERF_FUNCTIONS_H
#define PERF_FUNCTIONS_H

#include <QVector>

typedef enum {
    INST,
    HW_CYCLES
} counter_config;

int perf_open(int inherit, counter_config config);

u_int64_t read_counter(QVector<int> fds);

int close_counter(QVector<int> fds);

#endif // PERF_FUNCTIONS_H
