#include "perf_functions.h"
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>

static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int perf_open(int inherit, counter_config config)
{
    perf_event_attr attr;
    memset(&attr,0,sizeof(attr));
    attr.size = sizeof(attr);
    attr.exclude_kernel = 1;
    attr.type = PERF_TYPE_HARDWARE;

    switch(config){
        case INST :
            attr.config = PERF_COUNT_HW_INSTRUCTIONS;
            break;
        case HW_CYCLES :
            attr.config = PERF_COUNT_HW_CPU_CYCLES;
            break;
        default :
            fprintf(stderr, "Invalid counter configuration.\n");
            exit(EXIT_FAILURE);
            break;
    }

    attr.inherit = inherit;
    attr.disabled = 0;
    return sys_perf_event_open(&attr, 0, -1, -1, 0);
}

u_int64_t read_counter(QVector<int> fds)
{
    u_int64_t sum = 0;
    u_int64_t val;
    for(int i = 0; i < fds.size(); i++)
    {
        read(fds[i], &val, sizeof(u_int64_t));
        sum += val;
    }
    return sum;
}

int close_counter(QVector<int> fds)
{
    foreach (int fd, fds)
    {
       close(fd);
    }
}
