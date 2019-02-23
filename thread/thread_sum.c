#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

#define M_MAX     1e32    //整数范围 1 ～ MAX
#define N_MAX     200           //创建N 个子线程求和
#define AVE     (M/N)       //每个子线程处理的整数个数

long long     *sum = NULL;    //保存各个子线程计算的结果
long long M;
int N;
//获取当前时间
double get_time()
{
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec + t.tv_usec/1000000.0;
}

//求和子线程
void* sum_work(void* arg)
{
    int i=*((int*)arg);   //第i个线程
    long long start;
    long long end;
    

    start=i*AVE+1;
    if(i<N-1)
        end=start+AVE-1;
    else
        end=M;
    
    sum[i]=0;
    long long j;
    for(j=start;j<=end;j++)
        sum[i]+=j;
    pthread_exit(0);
}

int main()
{
    scanf("N = %d\n", &N);
    scanf("M = %lld", &M);
    
    double         t1,t2;
    pthread_t      *pthread_id = NULL; //保存子线程id
    int            i;
    long long      common_result = 0;         //总和

    // 分配空间
    pthread_id = (pthread_t*)malloc(N * sizeof(pthread_t));
    sum  = (long long*)malloc(N * sizeof(long long));

    // 开始计时
    t1 = get_time();

    //创建N个子线程
    for(i = 0; i < N; i++) {
        /**
         * 参数说明：
         * 1. 线程标识符指针
         * 2. 线程属性
         * 3. 线程运行函数
         * 4. 运行函数的参数 
         */
        pthread_create(pthread_id+i, NULL, sum_work, i);
    }

    //将各个子线程的求和结果相加
    for(i = 0; i < N; i++) {
        //等待子线程结束，如果该子线程已经结束，则立即返回
        /**
         * 参数说明：
         * 1. 线程标识
         * 2. 存储该线程返回值
         */
        pthread_join(pthread_id[i], NULL);
        common_result += sum[i];
    }

    //求和结束
    t2 = get_time();

    //输出求和结果和运行时间
    printf("result is %lld , runtime is %f\n", common_result, t2-t1);

    free(pthread_id);
    free(sum);
    return 0;
}