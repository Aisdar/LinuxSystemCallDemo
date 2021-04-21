#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

/* semctl ����ʹ�õ�union������cmdֵ�Ĳ�ͬ�����ò�ͬ��ֵ*/
/* ʵ�������ǻ���ֻ�� val ��������������Ľṹ��ָ����Ҫ���չ̶���ʽ�����Ǻ����� */
/* �������ȶ��岻ͬ��ֵ������ͬ����Ϊ */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds* buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short* array;  /* Array for GETALL, SETALL */
	struct seminfo* __buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};


int sem_create(key_t key, int nsems) {
	int sem_id = semget(key, nsems, IPC_CREAT | 0766);	// �����ź����飬������ nsems ���ź���
	if (sem_id < 0) {
		perror("semget error");
	}
	return sem_id;
}

/// sem_setval ���ź�����������
/// @param sem_id �ź�����ʶ��
/// @param sem_num �ź����±�
/// return 
int sem_setval(int sem_id, int sem_num, int val) {

	union semun arg;
	arg.val = val;

	int res = semctl(sem_id, sem_num, SETVAL, arg);	// �������������������������ֻ�ܷ�union semun
	if (res < 0) {
		perror("semctl set error");
	}
	return res;
}

int sem_getval(int sem_id, int sem_num) {

	int res = semctl(sem_id, sem_num, GETVAL);	// �������������������������ֻ�ܷ�union semun
	if (res < 0) {
		perror("semctl get error");
	}
	return res;
}

// ���� -1
int sem_p(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, -1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop p error");
	}
	return res;
}

// ���� +1
int sem_v(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, 1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop v error");
	}
	return res;
}

int main() {
	int sem_id = sem_create((key_t)10086, 1);	// ����һ���ź��飬һ��1���ź���
	cout << "sem_id=" << sem_id << endl;
	sem_setval(sem_id, 0, 1);					// ����ʾ���õ��ź���ֵ

	int pid = fork();

	if (pid < 0) {
		perror("fork error");
	}

	if (pid > 0) {
		while (1) {
			sem_p(sem_id, 0);	// �� sem_id �� 0 �±��ź���p����
			cout << "�����̣�" << getpid() << " ����" << endl;
			sleep(5);			// �����̹���
			cout << "�����̣�" << getpid() << " ����" << endl;
			sem_v(sem_id, 0);	// �� sem_id �� 0 �±��ź���p
		}
	}
	else if (pid == 0) {
		while (1) {
			sem_p(sem_id, 0);	// �� sem_id �� 0 �±��ź���p����
			cout << "�ӽ��̣�" << getpid() << " ����" << endl;
			cout << "�ӽ��̣�" << getpid() << " ����" << endl;
			sem_v(sem_id, 0);	// �� sem_id �� 0 �±��ź���p����
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





