/*
 * 27 october 2015
 *
 * Very simple test program to verify that the counter will total instructions executed by a thread and its children if inherit is set to true.
 *
*/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <string.h>
#include <omp.h>
#include <sys/syscall.h>

#define ARRAY_SIZE 100000

/* perf_event_open syscall wrapper */
static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

void write_linear(int * array, int size) {

    int i;
    volatile int sum;
    #pragma omp parallel private(sum,i)
    {
        for(i=0; i<size; i++){
            array[i] = i*2;
            sum = array[i];
        }
        printf("Thread %d, sum = %d.\n", omp_get_thread_num(), sum);
    }
}

void make_experiment(int inherit=0) {

    printf("Inherit is set to %d.\n", inherit);
    int array[ARRAY_SIZE];

    perf_event_attr attr;
    memset(&attr,0,sizeof(attr));
    attr.size = sizeof(attr);
    attr.exclude_kernel = 1;
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    attr.inherit = inherit;
    attr.disabled = 0;

    pid_t pid = getpid();
    int fd = sys_perf_event_open(&attr, pid, -1, -1, 0);

    long v1,v2;

    omp_set_dynamic(0);
    omp_set_num_threads(1);
    write_linear(array, ARRAY_SIZE);
    read(fd, &v1, sizeof(v1));
    write_linear(array, ARRAY_SIZE);
    read(fd, &v2, sizeof(v2));
    printf("Number of total instruction : %ld.\n", v2-v1);

    omp_set_num_threads(2);
    write_linear(array, ARRAY_SIZE);
    read(fd, &v1, sizeof(v1));
    write_linear(array, ARRAY_SIZE);
    read(fd, &v2, sizeof(v2));
    // It should be twice as big as there are two threads
    printf("Number of total instruction : %ld.\n", v2-v1);

    close(fd);
}

int main(int argc, char** argv)
{
    (void) argc; (void) argv;

    make_experiment(1);

    return 0;
}
