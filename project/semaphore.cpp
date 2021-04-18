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

/* �ź���������߳�ͬ��������
*  �������߳�1��numִ�� ���� �������߳�2��numִ�� �Լ� ������
*  Ŀ�ģ�ϣ�������߳��ܹ��� �߳�1���߳�2���߳�1..... �Ľ���ִ�С�
*        �����̵߳��첽�ԣ�����ʹ���ź������߳�֮���ͬ����
*  ������߳�1
*/
void semaphore() {
	// ��ʼ���ź���
	sem_init(&sem_g, 0, 0);	// �ź�������ʼֵΪ0
	sem_init(&sem_p, 0, 1);	// �ź�������ʼֵΪ1
	// �����߳�1���߳�2
	if (pthread_create(&pthread_id1, NULL, pthread_fun1, NULL)) {
		perror("create pthread error");
	}

	if (pthread_create(&pthread_id2, NULL, pthread_fun2, NULL)) {
		perror("create pthread error");
	}

	while (1);

	// �����ź���
	sem_destroy(&sem_p);
	sem_destroy(&sem_g);
}

// �ȴ��ź���ֵ����0���˴�Ϊ1�����̵߳õ�ִ��
// ��֮�ź���ֵС��0 ���߳̽���һ��æ�ȴ����൱�� while(1)��
void* pthread_fun1(void* p) {
	// + ����
	while (1) {
		sem_wait(&sem_p);	// ��ʼֵΪ1�����õȴ�

		cout << "pthread1,num=" << ++num << endl;
		sleep(1);	// �´������ʧȥCPU

		sem_post(&sem_g);	// ���߳̽��������⿪��һ���̵߳�æ�ȴ�
	}
}

void* pthread_fun2(void* p) {
	// - ����
	while (1) {
		sem_wait(&sem_g);	// ��ʼֵΪ0����Ҫ�ȴ����˽���æ�ȴ�

		cout << "pthread2,num=" << --num << endl;
		sleep(1);	// �´������ʧȥCPU

		sem_post(&sem_p);	// ���߳̽��������⿪��һ���̵߳�æ�ȴ�
	}
}

