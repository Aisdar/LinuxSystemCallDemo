#include "daemon.h"

void error_msg(int return_code, char* msg) {
	if (return_code < 0) {
		perror(msg);
		exit(-1);
	}
}


/* 守护进程的创建 */
void daemon() {
	// 创建一个子进程 child1
	pid_t child1_pid = fork();
	error_msg(child1_pid, "child1 fork error:");
	// session是一组进程的集合
	// 父进程输出所在的session组id
	if (child1_pid > 0) {
		cout << "father gpid=" << getpgid(getpid()) << endl;
	}
	// [1]首先将 child1子进程 放入新的session里
	// [2]在 child1子进程 中再次fork一个子进程作为守护进程
	if (child1_pid == 0) {
		setsid();	// 关键代码，将 child1 子进程放入一个新的session
		pid_t daemon_pid = fork();
		error_msg(daemon_pid, "daemon fork error:");
		if (daemon_pid == 0) {
			cout << "daemon gpid=" << getpgid(getpid()) << endl;
			exit(1);
		}
		wait();	// 等待子进程调用exit, 替它收尸
		exit(1);
	}
	wait();	// 等待子进程调用exit, 替它收尸
}


// 进程父子关系的树状结构
//  (session leader)	
// 		  ↓           (session leader)	
// father_process+           ↓   
//               |--child1_process+
//                                |--daemon_process


// session 关系
// (session leader)	            (session leader)
//        ↓                            ↓
// father_process               child1_process+
//                                            |--daemon_process

// 假装修改了