/*
 * 27 october 2015
 *
 * Very simple test program to verify that the counter will total instructions executed by a thread and its children if inherit is set to true.
 *
*/

#include <QDebug>
#include <QVector>
#include <qalgorithms.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <string.h>
#include <omp.h>
#include <sys/syscall.h>

#define ARRAY_SIZE 100000

typedef enum {
    INST,
    HW_CYCLES
} counter_type;

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

/* Open an hardware counter for the current thread */
int perf_open(int inherit, counter_type config)
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

void close_counter(QVector<int> fds)
{
    foreach (int fd, fds)
    {
       close(fd);
    }
}

void make_experiment(int inherit, int nbTh)
{

    printf("Inherit is set to %d.\n", inherit);
    int array[ARRAY_SIZE];

    omp_set_dynamic(0);
    omp_set_num_threads(1);

    u_int64_t inst[2];
    u_int64_t cycles[2];

    QVector<int> fds_inst;
    QVector<int> fds_cycles;
    #pragma omp parallel
    {
        #pragma omp single
        {
            fds_inst.resize(omp_get_num_threads());
            fds_cycles.resize(omp_get_num_threads());
        }
        fds_inst[omp_get_thread_num()] = perf_open(1,counter_type(INST));
        fds_cycles[omp_get_thread_num()] = perf_open(1,counter_type(HW_CYCLES));
    }

    for(int i = 0; i<fds_cycles.size(); i++){
        inst[0] = read_counter(fds_inst);
        cycles[0] = read_counter(fds_cycles);
    }

    write_linear(array, ARRAY_SIZE);

    for(int i = 0; i<fds_cycles.size(); i++){
        inst[1] = read_counter(fds_inst);
        cycles[1] = read_counter(fds_cycles);
    }

    printf("Number of total instruction for %d threads : %ld.\n", omp_get_num_threads(), inst[1]-inst[0]);
    printf("Number of total cycles for %d threads : %ld.\n", omp_get_num_threads(), cycles[1]-cycles[0]);

    double ipc = ((double)(inst[1] - inst[0])) / ((double)(cycles[1] - cycles[0]));

    printf("IPC calculated : %f.\n", ipc);

    close_counter(fds_inst);
    close_counter(fds_cycles);
}


int main(int argc, char** argv)
{
    (void) argc; (void) argv;

    make_experiment(1,2);

    return 0;
}
