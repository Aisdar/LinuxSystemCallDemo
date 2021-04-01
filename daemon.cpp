#include "daemon.h"

void error_msg(int return_code, char* msg) {
	if (return_code < 0) {
		perror(msg);
		exit(-1);
	}
}


/* �ػ����̵Ĵ��� */
void daemon() {
	// ����һ���ӽ��� child1
	pid_t child1_pid = fork();
	error_msg(child1_pid, "child1 fork error:");
	// session��һ����̵ļ���
	// ������������ڵ�session��id
	if (child1_pid > 0) {
		cout << "father gpid=" << getpgid(getpid()) << endl;
	}
	// [1]���Ƚ� child1�ӽ��� �����µ�session��
	// [2]�� child1�ӽ��� ���ٴ�forkһ���ӽ�����Ϊ�ػ�����
	if (child1_pid == 0) {
		setsid();	// �ؼ����룬�� child1 �ӽ��̷���һ���µ�session
		pid_t daemon_pid = fork();
		error_msg(daemon_pid, "daemon fork error:");
		if (daemon_pid == 0) {
			cout << "daemon gpid=" << getpgid(getpid()) << endl;
			exit(1);
		}
		wait();	// �ȴ��ӽ��̵���exit, ������ʬ
		exit(1);
	}
	wait();	// �ȴ��ӽ��̵���exit, ������ʬ
}


// ���̸��ӹ�ϵ����״�ṹ
//  (session leader)	
// 		  ��           (session leader)	
// father_process+           ��   
//               |--child1_process+
//                                |--daemon_process


// session ��ϵ
// (session leader)	            (session leader)
//        ��                            ��
// father_process               child1_process+
//                                            |--daemon_process

// ��װ�޸���