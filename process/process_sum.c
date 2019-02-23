#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <pthread.h>

// #define OUTPUT_PATH "output.txt"
// #define INPUT_PATH "input.txt"

#define M_MAX     1e32    //整数范围 1 ～ MAX
#define N_MAX     200           //创建N 个子线程求和
#define AVE     (M/N)       //每个子线程处理的整数个数

// long long     *sum = NULL;    //保存各个子线程计算的结果
// long long M;
// int N;

// 获取当前时间
double get_time() {
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec + t.tv_usec/1000000.0;
}

struct struct_zone_data{
    unsigned long start;
    unsigned long end;
};
struct struct_zone_data zone_data[20];

unsigned long *sum = NULL;
pthread_mutex_t mutex;

int sum_work(long N, long M){
    int process = 0;
    while(true){
        pid_t pid = fork();
        if(pid == 0){
            // child process
            // calculate and exit
            //printf("child : %d\n", process);
            //printf("calculate zone : %u ~ %u\n", zone_data[process].start, zone_data[process].end);
            unsigned long local_sum = 0;
            for(unsigned long start = zone_data[process].start; start <= zone_data[process].end; start++){
                local_sum += start;
            }

            // add local sum to sum
            pthread_mutex_lock(&mutex);
            int shmid = shmget((key_t)2333, 8, IPC_CREAT|0666);
            void *p_addr = shmat(shmid, NULL, 0);
            if(p_addr == (void*)-1){
                printf("connect shared memery fail\n");
                exit(1);
            }
            sum = (unsigned long*)p_addr;
            *sum = *sum + local_sum;
            //printf("add: %ld\nnow: %ld\n", local_sum, *sum);
            pthread_mutex_unlock(&mutex);
            exit(0);
        }else{
            // parents process
            //printf("parent process: child pid: %d\n", pid);
            // run until
            ++process;
            if(process > N - 1){
                // wait all child process exit
                while(wait(NULL)!=-1){}
                break;
            }
            
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    long M = 0,N = 0;
    scanf("N = %d\n", &N);
    scanf("M = %lld", &M);
    double      t1, t2;
    // 开始计时
    t1 = get_time();
    // 进行计算
    // long block_num = M/N;
    unsigned long extra  = N == 1 ? M : M%N;
    unsigned start_num = 1;
    for(int i = 0; i < N; i++) {
        if(i < extra) {
            zone_data[i].start = start_num;
            zone_data[i].end = (start_num + AVE) <= M ? (start_num + AVE) : M;
            start_num += AVE + 1;
        } else {
            zone_data[i].start = start_num;
            zone_data[i].end = (start_num + AVE - 1) <= M ? (start_num + AVE -1) : M;
            start_num += AVE;
        }
    }

    pid_t pid = fork();
    if(pid < 0) {
        printf("pid < 0 error!!\n");
        exit(1);
    } else if (pid == 0){
        // printf("child process\n");
        sum_work(N, M);
        exit(0);
    } else {
        // printf("parent process\n");
        // init lock and create shared memery
        pthread_mutex_init(&mutex, NULL);
        /**
         * 共享内存
         * 1. 建立新共享内存对象
         * 2. 新建共享内存大小
         * 3. 如果内核中不存在键值与key相等的共享内存，则新建一个消息队列
         */
        int shmid = shmget((key_t)2333, 8, IPC_CREAT|0666);
        void *p_addr = shmat(shmid, NULL, 0);
        if(p_addr == (void*)-1){
            printf("FAIL to create memory\n");
            exit(1);
        }
        sum = (unsigned long*)p_addr;

        // 父进程执行
        waitpid(pid, NULL, 0);

        // 结束
        t2 = get_time();
        printf("time use: %f\n", t2 - t1);
        printf("result:%ld\n", *sum);

        // destory share memery
        shmdt(sum);
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    return 0;
}
