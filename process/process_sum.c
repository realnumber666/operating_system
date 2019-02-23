#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <pthread.h>

// #define OUTPUT_PATH "output.txt"
// #define INPUT_PATH "input.txt"

#define M_MAX     1e32    //整数范围 1 ～ MAX
#define N_MAX     200           //创建N 个子线程求和
#define AVE     (M/N)       //每个子线程处理的整数个数

unsigned long *sum = NULL;
pthread_mutex_t mutex;

// 获取当前时间
double get_time() {
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec + t.tv_usec/1000000.0;
}

struct Data_segment{
    unsigned long start;
    unsigned long end;
};
struct Data_segment data_segment[20];




int sum_work(long N, long M){
    int process = 0;
    while(true){
        pid_t pid = fork();
        if(pid == 0){
            unsigned long local_sum = 0;
            for(unsigned long start = data_segment[process].start; start <= data_segment[process].end; start++){
                local_sum += start;
            }

            // 通过共享内存求和
            pthread_mutex_lock(&mutex);
            int shmid = shmget((key_t)2333, 8, IPC_CREAT|0666);
            void *p_addr = shmat(shmid, NULL, 0);
            if(p_addr == (void*)-1){
                printf("connect shared memery fail\n");
                exit(1);
            }
            sum = (unsigned long*)p_addr;
            *sum = *sum + local_sum;
            pthread_mutex_unlock(&mutex);
            exit(0);
        }else{
            // 父进程执行
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
    unsigned long extra  = N == 1 ? M : M%N;
    unsigned start_num = 1;
    for(int i = 0; i < N; i++) {
        if(i < extra) {
            data_segment[i].start = start_num;
            data_segment[i].end = (start_num + AVE) <= M ? (start_num + AVE) : M;
            start_num += AVE + 1;
        } else {
            data_segment[i].start = start_num;
            data_segment[i].end = (start_num + AVE - 1) <= M ? (start_num + AVE -1) : M;
            start_num += AVE;
        }
    }

    pid_t pid = fork();
    if(pid < 0) {
        printf("FAIL to create pid!!\n");
        exit(1);
    } else if (pid == 0){
        sum_work(N, M);
        exit(0);
    } else {
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

        // 释放共享内存
        shmdt(sum);
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    return 0;
}
