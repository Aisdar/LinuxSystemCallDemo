#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

/* semctl 函数使用的union，根据cmd值的不同而设置不同的值*/
/* 实际上我们基本只用 val 这个变量，其他的结构体指针需要按照固定格式，不是很自由 */
/* 可以事先定义不同的值来代表不同的行为 */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds* buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short* array;  /* Array for GETALL, SETALL */
	struct seminfo* __buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};


int sem_create(key_t key, int nsems) {
	int sem_id = semget(key, nsems, IPC_CREAT | 0766);	// 创建信号量组，组内有 nsems 个信号量
	if (sem_id < 0) {
		perror("semget error");
	}
	return sem_id;
}

/// sem_setval 给信号量设置数据
/// @param sem_id 信号量标识码
/// @param sem_num 信号量下标
/// return 
int sem_setval(int sem_id, int sem_num, int val) {

	union semun arg;
	arg.val = val;

	int res = semctl(sem_id, sem_num, SETVAL, arg);	// 多参数函数，第三个参数往后只能放union semun
	if (res < 0) {
		perror("semctl set error");
	}
	return res;
}

int sem_getval(int sem_id, int sem_num) {

	int res = semctl(sem_id, sem_num, GETVAL);	// 多参数函数，第三个参数往后只能放union semun
	if (res < 0) {
		perror("semctl get error");
	}
	return res;
}

// 加锁 -1
int sem_p(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, -1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop p error");
	}
	return res;
}

// 解锁 +1
int sem_v(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, 1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop v error");
	}
	return res;
}

int main() {
	int sem_id = sem_create((key_t)10086, 1);	// 创建一个信号组，一组1个信号量
	cout << "sem_id=" << sem_id << endl;
	sem_setval(sem_id, 0, 1);					// 设置示例用的信号量值

	int pid = fork();

	if (pid < 0) {
		perror("fork error");
	}

	if (pid > 0) {
		while (1) {
			sem_p(sem_id, 0);	// 对 sem_id 的 0 下标信号量p操作
			cout << "父进程：" << getpid() << " 加锁" << endl;
			sleep(5);			// 父进程挂起
			cout << "父进程：" << getpid() << " 解锁" << endl;
			sem_v(sem_id, 0);	// 对 sem_id 的 0 下标信号量p
		}
	}
	else if (pid == 0) {
		while (1) {
			sem_p(sem_id, 0);	// 对 sem_id 的 0 下标信号量p操作
			cout << "子进程：" << getpid() << " 加锁" << endl;
			cout << "子进程：" << getpid() << " 解锁" << endl;
			sem_v(sem_id, 0);	// 对 sem_id 的 0 下标信号量p操作
			while (1);
		}
	}

	return 0;
}

/*
* struct sembuf
* {
*	unsigned short int sem_num;	 //semaphore number
*	short int sem_op;			 //semaphore operation
*	short int sem_flg;			 //operation flag
* };
*/





