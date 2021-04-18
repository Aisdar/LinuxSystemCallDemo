#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

int num = 0;
void* pthread_fun1(void* p);
void* pthread_fun2(void* p);
pthread_t pthread_id1, pthread_id2;
sem_t sem_g, sem_p;

/* 信号量：解决线程同步的问题
*  样例：线程1对num执行 自增 操作、线程2对num执行 自减 操作。
*  目的：希望两个线程能够以 线程1、线程2、线程1..... 的交替执行。
*        由于线程的异步性，我们使用信号量做线程之间的同步。
*  输出：线程1
*/
void semaphore() {
	// 初始化信号量
	sem_init(&sem_g, 0, 0);	// 信号量赋初始值为0
	sem_init(&sem_p, 0, 1);	// 信号量赋初始值为1
	// 创建线程1、线程2
	if (pthread_create(&pthread_id1, NULL, pthread_fun1, NULL)) {
		perror("create pthread error");
	}

	if (pthread_create(&pthread_id2, NULL, pthread_fun2, NULL)) {
		perror("create pthread error");
	}

	while (1);

	// 清理信号量
	sem_destroy(&sem_p);
	sem_destroy(&sem_g);
}

// 等待信号量值大于0（此处为1），线程得到执行
// 反之信号量值小于0 ，线程进入一个忙等待（相当于 while(1)）
void* pthread_fun1(void* p) {
	// + 操作
	while (1) {
		sem_wait(&sem_p);	// 初始值为1，不用等待

		cout << "pthread1,num=" << ++num << endl;
		sleep(1);	// 下处理机，失去CPU

		sem_post(&sem_g);	// 本线程结束啦，解开另一个线程的忙等待
	}
}

void* pthread_fun2(void* p) {
	// - 操作
	while (1) {
		sem_wait(&sem_g);	// 初始值为0，需要等待他人解锁忙等待

		cout << "pthread2,num=" << --num << endl;
		sleep(1);	// 下处理机，失去CPU

		sem_post(&sem_p);	// 本线程结束啦，解开另一个线程的忙等待
	}
}

