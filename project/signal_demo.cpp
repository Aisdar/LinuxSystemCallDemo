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

// fork错误信息打印
void fork_error_msg(int return_code, const char* msg);

// 信号屏蔽
void signal_shield();

// 信号屏蔽_处理函数1
void hanlder_sigaction(int num);

// 信号冲突解决
void signal_conflict();

// 信号练习的函数声明
void signal_work();
	// 需要绑定的处理函数函数
void A_signal_function(int signum, siginfo_t* act, void* vo);
void B_signal_function(int signum, siginfo_t* act, void* vo);
void C_signal_function(int signum, siginfo_t* act, void* vo);
void D_signal_function(int signum, siginfo_t* act, void* vo);

union sigval value;	// 重要！大家都能用的全局value！

pid_t pid_A;			// 重要！D的函数里要能给A传发信号，但是又无法在处理函数里得到A的pid
						// 使用全局变量做一个交互
						// 向上级进程发送数据的时候比较麻烦，需要记录pid

/* 程序要求:共有4个子进程 A、B、C、D，要求A进程发送SIGUSR1信号携带数据2001，B进程接收到信号后将数据自增1(2002),
*          B进程将自增后的数据通过SIGUSR2发送给C进程，C进程接收到信号后同样将数据自增1(2003),C进程将自增后的数据
*          发送通过SIGRTMIN给进程D，D收到信号后不做任何处理，将数据通过SIGUSR2信号发送给进程A，A打印出最终的数据
*          信息。
*/
void signal_work() {

	pid_A = fork();	// pid_A是全局变量
	fork_error_msg(pid_A, "A fork error");

	int status;	// 退出状态

	if (pid_A == 0) {
		// A进程
		// 这里必须在A进程保存好自己pid，给D用
		pid_A = getpid();

		// 提前绑定好信号，这个信号绑定是给B进程用的
		struct sigaction act;
		act.sa_sigaction = B_signal_function;	// 绑定函数
		act.sa_flags = SA_SIGINFO;	// 声明是带数据的信号
		sigaction(SIGUSR1, &act, NULL);	// B 绑定 SIGUSR2


		pid_t pid_B = fork();
		fork_error_msg(pid_B, "B fork error");

		if (pid_B == 0) {
			// B 进程
			// 提前绑定好信号，这个信号绑定是给C进程用的
			act.sa_sigaction = C_signal_function;	// 绑定函数
			act.sa_flags = SA_SIGINFO;	// 声明是带数据的信号
			sigaction(SIGUSR2, &act, NULL);// C 绑定 SIGUSR2

			pid_t pid_C = fork();
			fork_error_msg(pid_C, "C fork error");

			if (pid_C == 0) {
				// C进程
				// 提前绑定好信号，这个信号绑定是给D进程用的
				act.sa_sigaction = D_signal_function;	// 绑定函数
				act.sa_flags = SA_SIGINFO;	// 声明是带数据的信号
				sigaction(SIGRTMIN, &act, NULL);// D 绑定 SIGRTMIN

				pid_t pid_D = fork();
				fork_error_msg(pid_D, "D fork error");

				if (pid_D == 0) {
					// D进程
					while (1);
				}
				else if (pid_D > 0) {
					// C 给 D 发送信号
					sigqueue(pid_D, SIGRTMIN, value);
					// C 给 D 收尸
					waitpid(pid_D, &status, 0);
					// 结束 C 进程，使得 B 进程得以结束
					exit(0);
				}
			}
			else if (pid_C > 0) {
				// B 给 C 发送信号
				sigqueue(pid_C, SIGUSR2, value);
				// B 给 C 收尸
				waitpid(pid_C, &status, 0);
				// 结束 B 进程，使得 A 进程得以结束
				exit(0);
			}
		}
		else if (pid_B > 0) {
			// 构造好A要发送给B的值
			// A 要能够处理SIGUSR2信号的冲突，同时要给A绑定好自己的信号函数
			act.sa_sigaction = A_signal_function;	// 绑定函数
			act.sa_flags = SA_SIGINFO;	// 声明是带数据的信号

			sigemptyset(&(act.sa_mask));
			sigaddset(&(act.sa_mask), SIGUSR1);	// 选择希望处理的冲突信号
			sigaction(SIGUSR2, &act, NULL);	// A 绑定 SIGUSR2

			// A 给 B 发信号
			value.sival_int = 2001;
			sigqueue(pid_B, SIGUSR1, value);
			// A 给 B 收尸
			waitpid(pid_B, &status, 0);
			// A 不能结束，还得等D发送信号
			while (1);
		}
	}
	else if (pid_A > 0) {
		// 父进程，为了做示例，不让结束
		// 父进程 给 A 收尸
		waitpid(pid_A, &status, 0);
	}
}

void A_signal_function(int signum, siginfo_t* act, void* vo) {
	// SIGUSR2
	if (signum == 12) {
		cout << "A pid=" << getpid() << "，A get value from D, the final value=" << act->si_int << endl;
		// 这个休眠是为了测试A是否能防止来自SIGUSR1信号的冲突
		cout << "now, A sleep for 20 seconds to check the signal confilct from SIGUSR1" << endl;
		for (int i = 0; i < 20; i++) {
			cout << "sleep " << i + 1 << "s......." << endl;
			sleep(1);
		}
		// D 进程发送来信号了，可以结束了
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
		// 给A先后发两种信号，看看他能否防止SIGUSR1的冲突
		sigqueue(pid_A, SIGUSR2, value);
		//sigqueue(pid_A, SIGUSR1, value);
		// 结束D进程，使得C进程得以结束
		exit(0);
	}
}


void signal_conflict() {
	// 信号冲突的解决
	struct sigaction act;
	act.sa_handler = hanlder_sigaction;	// action绑定函数指针，此函数不携带数据
	act.sa_flags = 0;	// 指定此函数不带参数

	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR2);	// 选择希望减少冲突的信号
	sigaction(SIGUSR1, &act, NULL);	// 给信号绑定action

	// 确保父进程或者
	while (true) {
		cout << "father process active , pid=" << getpid() << " now sleep for 1 seconds" << endl;
		sleep(1);
	}
	/* 信号的冲突是指该进程正处理某个信号而另一个信号又被发送到该进程来，导致进程直接结束的问题，换句话说：信号的冲突域是其处理函数 */

	/* 实验1：命令行使用 kill -10 pid 给自己发送SIGUSR1信号，之后在进程响应SIGUSR1时多次重复给自己发送SIGUSR1信号 */
	// 实验说明：进程需要给SIGUSR1绑定对应的处理函数
	// 实验过程：收到信号后SIGUSR1对应的处理函数打印信息并休眠10s，在此10s内给自己重复发送SIGUSR1信号(超过2次)
	// 实验结果：在接收进程执行SIGUSR1对应处理函数的时候最多能只能再次接收SIGUSR1信号1次，等待上一个SIGUSR1处理
	//          完成会再次执行SIGUSR1信号
	// 实验结论：进程接收同种信号时允许排队，队伍最大长度为1（内核只容许1个同种信号等待，其余同种信号会被内核忽略）

	/* 实验2：命令行使用 kill -10 pid 给自己发送SIGUSR1信号后再按顺序给自己发送 kill -10 pid 与 kill -12 pid*/
	// 实验说明：进程需要给SIGUSR1绑定对应的处理函数，而不给SIGUSR2绑定对应的处理函数
	// 实验过程：收到信号后SIGUSR1对应的处理函数打印信息并休眠10s，在此之后10s内再按顺序给自己发送SIGUSR1、
	//          SIGUSR2信号
	// 实验结果：连续发送两次SIGUSR1信号后，进程执行SIGUSR1信号并将第二个SIGUSR1处于“排队”状态，对于第三个信号（异种信号SIGUSR2）
	//          的到来，内核还是会发送给进程，由于该进程没有绑定过对应的处理函数，该进程会直接退出当前的信号处理函数，并结束进程自身。

	/* 实验3：与实验2相同，但是实验的说明不同 */
	// 实验说明：进程需要给SIGUSR1绑定对应的处理函数，同时将SIGUSR2信号加入信号集(struct sigaction结构体中的sigset_t sa_mask
	//          成员)
	// 实验过程：收到信号后SIGUSR1对应的处理函数打印信息并休眠10s，在此之后10s内再按顺序给自己发送SIGUSR1、
	//          SIGUSR2信号
	// 实验结果：连续发送两次SIGUSR1信号后，进程执行SIGUSR1信号并将第二个SIGUSR1置于“排队”状态，对于第三个信号（异种信号SIGUSR2）
	//          的到来，出现了截然不同的结果，进程没有立即退出而是同样将SIGUSR2信号置于“排队”状态。
}

// 信号屏蔽练习
void signal_shield() {

	// 信号连接
	struct sigaction act;
	//act.sa_sigaction = hanlder_sigaction;	// action绑定有参函数指针
	act.sa_handler = hanlder_sigaction;		// action绑定无参数的函数指针
	act.sa_flags = 0;						// action设置不带参数

	sigaction(SIGUSR1, &act, NULL);			// 信号绑定action


	// 信号屏蔽
	sigset_t array;
	sigemptyset(&array);
	sigaddset(&array, SIGUSR2);	// 将SIGUSR2添加入屏蔽容器
	// 设置阻塞该信号
	if ((sigprocmask(SIG_BLOCK, &array, NULL)) < 0) {
		perror("sigprocmask error");
		exit(-1);
	}

	// 信号传递的值
	union sigval value;
	value.sival_int = 10086;
	// 发信号，测试SIGUSR1（不可靠信号）连续发送时会产生的问题
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
	// 此处sleep是为了检测 有屏蔽容器记录的信号发来时，能够屏蔽该信号
	// 具体体现在不会被该信号中断
	sleep(10);
}
