#include "signal_demo.h"
#include "global.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

using namespace std;

// fork������Ϣ��ӡ
void fork_error_msg(int return_code, const char* msg);

// �ź�����
void signal_shield();

// �ź�����_������1
void hanlder_sigaction(int num);

// �źų�ͻ���
void signal_conflict();

// �ź���ϰ�ĺ�������
void signal_work();
	// ��Ҫ�󶨵Ĵ���������
void A_signal_function(int signum, siginfo_t* act, void* vo);
void B_signal_function(int signum, siginfo_t* act, void* vo);
void C_signal_function(int signum, siginfo_t* act, void* vo);
void D_signal_function(int signum, siginfo_t* act, void* vo);

union sigval value;	// ��Ҫ����Ҷ����õ�ȫ��value��

pid_t pid_A;			// ��Ҫ��D�ĺ�����Ҫ�ܸ�A�����źţ��������޷��ڴ�������õ�A��pid
						// ʹ��ȫ�ֱ�����һ������
						// ���ϼ����̷������ݵ�ʱ��Ƚ��鷳����Ҫ��¼pid

/* ����Ҫ��:����4���ӽ��� A��B��C��D��Ҫ��A���̷���SIGUSR1�ź�Я������2001��B���̽��յ��źź���������1(2002),
*          B���̽������������ͨ��SIGUSR2���͸�C���̣�C���̽��յ��źź�ͬ������������1(2003),C���̽������������
*          ����ͨ��SIGRTMIN������D��D�յ��źź����κδ���������ͨ��SIGUSR2�źŷ��͸�����A��A��ӡ�����յ�����
*          ��Ϣ��
*/
void signal_work() {

	pid_A = fork();	// pid_A��ȫ�ֱ���
	fork_error_msg(pid_A, "A fork error");

	int status;	// �˳�״̬

	if (pid_A == 0) {
		// A����
		// ���������A���̱�����Լ�pid����D��
		pid_A = getpid();

		// ��ǰ�󶨺��źţ�����źŰ��Ǹ�B�����õ�
		struct sigaction act;
		act.sa_sigaction = B_signal_function;	// �󶨺���
		act.sa_flags = SA_SIGINFO;	// �����Ǵ����ݵ��ź�
		sigaction(SIGUSR1, &act, NULL);	// B �� SIGUSR2


		pid_t pid_B = fork();
		fork_error_msg(pid_B, "B fork error");

		if (pid_B == 0) {
			// B ����
			// ��ǰ�󶨺��źţ�����źŰ��Ǹ�C�����õ�
			act.sa_sigaction = C_signal_function;	// �󶨺���
			act.sa_flags = SA_SIGINFO;	// �����Ǵ����ݵ��ź�
			sigaction(SIGUSR2, &act, NULL);// C �� SIGUSR2

			pid_t pid_C = fork();
			fork_error_msg(pid_C, "C fork error");

			if (pid_C == 0) {
				// C����
				// ��ǰ�󶨺��źţ�����źŰ��Ǹ�D�����õ�
				act.sa_sigaction = D_signal_function;	// �󶨺���
				act.sa_flags = SA_SIGINFO;	// �����Ǵ����ݵ��ź�
				sigaction(SIGRTMIN, &act, NULL);// D �� SIGRTMIN

				pid_t pid_D = fork();
				fork_error_msg(pid_D, "D fork error");

				if (pid_D == 0) {
					// D����
					while (1);
				}
				else if (pid_D > 0) {
					// C �� D �����ź�
					sigqueue(pid_D, SIGRTMIN, value);
					// C �� D ��ʬ
					waitpid(pid_D, &status, 0);
					// ���� C ���̣�ʹ�� B ���̵��Խ���
					exit(0);
				}
			}
			else if (pid_C > 0) {
				// B �� C �����ź�
				sigqueue(pid_C, SIGUSR2, value);
				// B �� C ��ʬ
				waitpid(pid_C, &status, 0);
				// ���� B ���̣�ʹ�� A ���̵��Խ���
				exit(0);
			}
		}
		else if (pid_B > 0) {
			// �����AҪ���͸�B��ֵ
			// A Ҫ�ܹ�����SIGUSR2�źŵĳ�ͻ��ͬʱҪ��A�󶨺��Լ����źź���
			act.sa_sigaction = A_signal_function;	// �󶨺���
			act.sa_flags = SA_SIGINFO;	// �����Ǵ����ݵ��ź�

			sigemptyset(&(act.sa_mask));
			sigaddset(&(act.sa_mask), SIGUSR1);	// ѡ��ϣ������ĳ�ͻ�ź�
			sigaction(SIGUSR2, &act, NULL);	// A �� SIGUSR2

			// A �� B ���ź�
			value.sival_int = 2001;
			sigqueue(pid_B, SIGUSR1, value);
			// A �� B ��ʬ
			waitpid(pid_B, &status, 0);
			// A ���ܽ��������õ�D�����ź�
			while (1);
		}
	}
	else if (pid_A > 0) {
		// �����̣�Ϊ����ʾ�������ý���
		// ������ �� A ��ʬ
		waitpid(pid_A, &status, 0);
	}
}

void A_signal_function(int signum, siginfo_t* act, void* vo) {
	// SIGUSR2
	if (signum == 12) {
		cout << "A pid=" << getpid() << "��A get value from D, the final value=" << act->si_int << endl;
		// ���������Ϊ�˲���A�Ƿ��ܷ�ֹ����SIGUSR1�źŵĳ�ͻ
		cout << "now, A sleep for 20 seconds to check the signal confilct from SIGUSR1" << endl;
		for (int i = 0; i < 20; i++) {
			cout << "sleep " << i + 1 << "s......." << endl;
			sleep(1);
		}
		// D ���̷������ź��ˣ����Խ�����
		exit(0);
	}
}

void B_signal_function(int signum, siginfo_t* act, void* vo) {
	// SIGUSR1
	if (signum == 10) {
		cout << "B get value " << act->si_int << endl;
		value.sival_int = ++act->si_int;
	}
}

void C_signal_function(int signum, siginfo_t* act, void* vo) {
	// SIGUSR2
	if (signum == 12) {
		cout << "C get value " << act->si_int << endl;
		value.sival_int = ++act->si_int;
	}
}


void D_signal_function(int signum, siginfo_t* act, void* vo) {
	// SIGRTMIN
	if (signum == 34) {
		cout << "D get value " << act->si_int << endl;
		value.sival_int = act->si_int;
		// ��A�Ⱥ������źţ��������ܷ��ֹSIGUSR1�ĳ�ͻ
		sigqueue(pid_A, SIGUSR2, value);
		//sigqueue(pid_A, SIGUSR1, value);
		// ����D���̣�ʹ��C���̵��Խ���
		exit(0);
	}
}


void signal_conflict() {
	// �źų�ͻ�Ľ��
	struct sigaction act;
	act.sa_handler = hanlder_sigaction;	// action�󶨺���ָ�룬�˺�����Я������
	act.sa_flags = 0;	// ָ���˺�����������

	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR2);	// ѡ��ϣ�����ٳ�ͻ���ź�
	sigaction(SIGUSR1, &act, NULL);	// ���źŰ�action

	// ȷ�������̻���
	while (true) {
		cout << "father process active , pid=" << getpid() << " now sleep for 1 seconds" << endl;
		sleep(1);
	}
	/* �źŵĳ�ͻ��ָ�ý���������ĳ���źŶ���һ���ź��ֱ����͵��ý����������½���ֱ�ӽ��������⣬���仰˵���źŵĳ�ͻ�����䴦���� */

	/* ʵ��1��������ʹ�� kill -10 pid ���Լ�����SIGUSR1�źţ�֮���ڽ�����ӦSIGUSR1ʱ����ظ����Լ�����SIGUSR1�ź� */
	// ʵ��˵����������Ҫ��SIGUSR1�󶨶�Ӧ�Ĵ�����
	// ʵ����̣��յ��źź�SIGUSR1��Ӧ�Ĵ�������ӡ��Ϣ������10s���ڴ�10s�ڸ��Լ��ظ�����SIGUSR1�ź�(����2��)
	// ʵ�������ڽ��ս���ִ��SIGUSR1��Ӧ��������ʱ�������ֻ���ٴν���SIGUSR1�ź�1�Σ��ȴ���һ��SIGUSR1����
	//          ��ɻ��ٴ�ִ��SIGUSR1�ź�
	// ʵ����ۣ����̽���ͬ���ź�ʱ�����Ŷӣ�������󳤶�Ϊ1���ں�ֻ����1��ͬ���źŵȴ�������ͬ���źŻᱻ�ں˺��ԣ�

	/* ʵ��2��������ʹ�� kill -10 pid ���Լ�����SIGUSR1�źź��ٰ�˳����Լ����� kill -10 pid �� kill -12 pid*/
	// ʵ��˵����������Ҫ��SIGUSR1�󶨶�Ӧ�Ĵ�������������SIGUSR2�󶨶�Ӧ�Ĵ�����
	// ʵ����̣��յ��źź�SIGUSR1��Ӧ�Ĵ�������ӡ��Ϣ������10s���ڴ�֮��10s���ٰ�˳����Լ�����SIGUSR1��
	//          SIGUSR2�ź�
	// ʵ������������������SIGUSR1�źź󣬽���ִ��SIGUSR1�źŲ����ڶ���SIGUSR1���ڡ��Ŷӡ�״̬�����ڵ������źţ������ź�SIGUSR2��
	//          �ĵ������ں˻��ǻᷢ�͸����̣����ڸý���û�а󶨹���Ӧ�Ĵ��������ý��̻�ֱ���˳���ǰ���źŴ���������������������

	/* ʵ��3����ʵ��2��ͬ������ʵ���˵����ͬ */
	// ʵ��˵����������Ҫ��SIGUSR1�󶨶�Ӧ�Ĵ�������ͬʱ��SIGUSR2�źż����źż�(struct sigaction�ṹ���е�sigset_t sa_mask
	//          ��Ա)
	// ʵ����̣��յ��źź�SIGUSR1��Ӧ�Ĵ�������ӡ��Ϣ������10s���ڴ�֮��10s���ٰ�˳����Լ�����SIGUSR1��
	//          SIGUSR2�ź�
	// ʵ������������������SIGUSR1�źź󣬽���ִ��SIGUSR1�źŲ����ڶ���SIGUSR1���ڡ��Ŷӡ�״̬�����ڵ������źţ������ź�SIGUSR2��
	//          �ĵ����������˽�Ȼ��ͬ�Ľ��������û�������˳�����ͬ����SIGUSR2�ź����ڡ��Ŷӡ�״̬��
}

// �ź�������ϰ
void signal_shield() {

	// �ź�����
	struct sigaction act;
	//act.sa_sigaction = hanlder_sigaction;	// action���вκ���ָ��
	act.sa_handler = hanlder_sigaction;		// action���޲����ĺ���ָ��
	act.sa_flags = 0;						// action���ò�������

	sigaction(SIGUSR1, &act, NULL);			// �źŰ�action


	// �ź�����
	sigset_t array;
	sigemptyset(&array);
	sigaddset(&array, SIGUSR2);	// ��SIGUSR2�������������
	// �����������ź�
	if ((sigprocmask(SIG_BLOCK, &array, NULL)) < 0) {
		perror("sigprocmask error");
		exit(-1);
	}

	// �źŴ��ݵ�ֵ
	union sigval value;
	value.sival_int = 10086;
	// ���źţ�����SIGUSR1�����ɿ��źţ���������ʱ�����������
	sigqueue(getpid(), SIGUSR1, value);
	sigqueue(getpid(), SIGUSR1, value);
	sigqueue(getpid(), SIGUSR1, value);
	//raise(SIGUSR1);
	//kill(getpid(), SIGUSR1);
	while (1) {
		cout << "pid=" << getpid() << " running" << endl;
		sleep(1);
	}

}

void hanlder_sigaction(int num) {
	cout << "signal SIGUSR1 received ,now sleep for 10 seconds" << endl;
	// �˴�sleep��Ϊ�˼�� ������������¼���źŷ���ʱ���ܹ����θ��ź�
	// ���������ڲ��ᱻ���ź��ж�
	sleep(10);
}
