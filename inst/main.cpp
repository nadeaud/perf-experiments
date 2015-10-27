#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <string.h>
#include <omp.h>
#include <sys/syscall.h>

#define ARRAY_SIZE 1000

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



int main(int argc, char** argv) {
    
    int array[ARRAY_SIZE];

    perf_event_attr attr;
    memset(&attr,0,sizeof(attr));
    attr.size = sizeof(attr);
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    attr.inherit = 1;
    attr.disabled = 0;

    pid_t pid = getpid();
    int fd = sys_perf_event_open(&attr, pid, -1, -1, 0);

    long v1,v2;
    int ret;

    omp_set_dynamic(0);
    omp_set_num_threads(1);
    ret = read(fd, &v1, sizeof(v1));
    write_linear(array,ARRAY_SIZE);
    ret = read(fd, &v2, sizeof(v2));
    printf("Number of total instruction : %ld.\n", v2-v1);

    omp_set_num_threads(2);
    ret = read(fd, &v1, sizeof(v1));
    write_linear(array,ARRAY_SIZE);
    ret = read(fd, &v2, sizeof(v2));
    // It should be twice as big as there are two threads
    printf("Number of total instruction : %ld.\n", v2-v1);

    return 0;
}
