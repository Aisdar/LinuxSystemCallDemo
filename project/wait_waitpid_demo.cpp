#include "global.h"
#include "function_demo.h"
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>

using namespace std;


/* 任务要求 */
/* 要求使用三个进程 A B C , 在执行任务之前
*  即 C进程 等待 B进程 ，B进程 等待 A进程
* A进程负责拷贝传入的文件 file，在同级目录下得到file-副本 
* B进程负责向 file-副本 的末尾插入"Hello Linux"
* C进程负责将 file-副本 的内容输出到控制台
*/
void demo_wait_waitpid(const char *file_path) {
	// 判断文件类型
	struct stat buf_stat;
	if (!stat(file_path, &buf_stat) < 0) {
		perror("lstat error:");
		return;
	}
	if (!S_ISREG(buf_stat.st_mode)){
		printf("not a file\n");
		return;
	}


	string file_duplication_path = string(file_path) + "-copy";		// 文件的副本名


	pid_t pid_A = fork();
	fork_error_msg(pid_A, "fork A process:");

	// A进程--拷贝文件，得到同级目录下的副本
	if (pid_A == 0) {
		char buffer[1024] = { '\0' };
		int read_size = 0, write_size = 0;
		
		int from_fd = open(file_path, O_RDONLY);
		open_error_msg(from_fd, "sorcefile open error");

		int to_fd = open(file_duplication_path.c_str(), O_WRONLY | O_CREAT);
		open_error_msg(to_fd, "targetfile open error");

		while ((read_size = read(from_fd, buffer, sizeof(buffer))) > 0) {
			write_size = write(to_fd, buffer, read_size);
			if (write_size < 0) {
				perror("targetfile write error");
				exit(-1);
			}
			bzero(buffer, sizeof(buffer));
		}
		// 关闭两个文件
		int close_code = close(from_fd);
		close_error_msg(close_code, "sorce file close error");
		close_code = close(to_fd);
		close_error_msg(close_code, "target file close error");

		printf("A process copyfile succeed\n");
		exit(0);
	}
	
	// 等待A进程结束
	int status = -1;
	waitpid(pid_A, &status, NULL);
	printf("father process recovered A process\n\n");

	pid_t pid_B = fork();
	fork_error_msg(pid_B, "fork B process:");

	if (pid_B == 0) {
		int fd = open(file_duplication_path.c_str(), O_RDWR);
		open_error_msg(fd, "duplication file open error");

		int seeklen = -1;
		seeklen = lseek(fd, 0, SEEK_END);	// 定位到文件末尾, 偏移(offset = 0)
		if (seeklen < 0) {
			perror("seek file end error");
			exit(-1);
		}
		// 写入Hello Linux
		char *buf = "Hello Linux";
		int write_size = write(fd, buf, strlen(buf));
		if (write_size < 0) {
			perror("duplication file write error");
			exit(-1);
		}

		int closeFlg = close(fd);
		close_error_msg(closeFlg, "duplication file close error");

		printf("\nB process write \"Hello Linux\" to %s succeed\n", file_duplication_path.c_str());
		exit(0);
	}

	// 等待B进程结束
	waitpid(pid_B, &status, NULL);
	printf("father process recovered B process\n\n");

	pid_t pid_C = fork();
	fork_error_msg(pid_C, "fork C process");

	if (pid_C == 0) {
		int fd = open(file_duplication_path.c_str(), O_RDWR);
		open_error_msg(fd, "duplication file open error");

		int read_size = -1;
		char buf[1024] = { '\0' };
		printf("file infomation in %s:\n", file_duplication_path.c_str());
		while ((read_size = read(fd, buf, sizeof(buf))) > 0) {
			printf("%s", buf);
		}
		printf("\n");	// 刷新缓冲区

		int closeFlg = close(fd);
		close_error_msg(closeFlg, "duplication file close error");

		printf("\nC process write file infomation succeed\n");
		exit(0);
	}

	// 等待C进程结束
	waitpid(pid_C, &status, NULL);
	printf("father process recovered C process\n\n");
	// 打印一下退出码status，正常有关status的值下面有介绍，status是int类型。
	// status与子进程结束时的状态码直接关联，但直接打印看不到结果，需要使用宏定义函数
	printf("child return: %d\n", WEXITSTATUS(status));
}


// 三个进程并不是同时创建的
/* father_process+
*                |-- process_A
*                |-- process_B
*                |-- process_C
*/

/******************************************************************************/
/* 任务要求 */
/* 要求使用三个进程 A B C , 在执行任务之前，要求三个进程已经都被创建出来
*  即 C进程 等待 B进程 ，B进程 等待 A进程
* A进程负责拷贝传入的文件 file，在同级目录下得到file-副本
* B进程负责向 file-副本 的末尾插入"Hello Linux"
* C进程负责将 file-副本 的内容输出到控制台
*/
void demo_wait_waitpid2(const char* file_path) {
	// 判断文件类型
	struct stat buf_stat;
	if (!stat(file_path, &buf_stat) < 0) {
		perror("lstat error:");
		return;
	}
	if (!S_ISREG(buf_stat.st_mode)) {
		printf("not a file\n");
		return;
	}

	string file_duplication_path = string(file_path) + "-copy";		// 文件的副本名



	int status;	
	pid_t pid_C = fork();
	fork_error_msg(pid_C, "fork C process error");
	
	// C进程
	if (pid_C == 0) {		
		pid_t pid_B = fork();
		fork_error_msg(pid_B, "fork B process error");

		// B进程
		if (pid_B == 0) {		
			pid_t pid_A = fork();
			fork_error_msg(pid_A, "fork A process error");


			// A进程
			if (pid_A == 0) {
				
				// A do something
				char buffer[1024] = { '\0' };
				int read_size = 0, write_size = 0;

				int from_fd = open(file_path, O_RDONLY);
				open_error_msg(from_fd, "sorcefile open error");

				int to_fd = open(file_duplication_path.c_str(), O_WRONLY | O_CREAT);
				open_error_msg(to_fd, "targetfile open error");

				while ((read_size = read(from_fd, buffer, sizeof(buffer))) > 0) {
					write_size = write(to_fd, buffer, read_size);
					if (write_size < 0) {
						perror("targetfile write error");
						exit(-1);
					}
					bzero(buffer, sizeof(buffer));
				}
				// 关闭两个文件
				int close_code = close(from_fd);
				close_error_msg(close_code, "sorce file close error");
				close_code = close(to_fd);
				close_error_msg(close_code, "target file close error");

				printf("A process copyfile succeed\n");
				exit(0);
			}
			else if (pid_A > 0) {
				// B进程等待A进程结束才能开始
				waitpid(pid_A, &status, 0);
				// B do something
				int fd = open(file_duplication_path.c_str(), O_RDWR);
				open_error_msg(fd, "duplication file open error");

				int seeklen = -1;
				seeklen = lseek(fd, 0, SEEK_END);	// 定位到文件末尾, 偏移(offset = 0)
				if (seeklen < 0) {
					perror("seek file end error");
					exit(-1);
				}
				// 写入Hello Linux
				char* buf = "Hello Linux";
				int write_size = write(fd, buf, strlen(buf));
				if (write_size < 0) {
					perror("duplication file write error");
					exit(-1);
				}

				int closeFlg = close(fd);
				close_error_msg(closeFlg, "duplication file close error");

				printf("\nB process write \"Hello Linux\" to %s succeed\n", file_duplication_path.c_str());
				exit(0);
			}
		}
		else if (pid_B > 0) {	
			// 等待B进程退出，C才能开始
			waitpid(pid_B, &status, 0);
			// C do something
			int fd = open(file_duplication_path.c_str(), O_RDWR);
			open_error_msg(fd, "duplication file open error");

			int read_size = -1;
			char buf[1024] = { '\0' };
			printf("file infomation in %s:\n", file_duplication_path.c_str());
			while ((read_size = read(fd, buf, sizeof(buf))) > 0) {
				printf("%s", buf);
			}
			printf("\n");	// 刷新缓冲区

			int closeFlg = close(fd);
			close_error_msg(closeFlg, "duplication file close error");

			printf("\nC process write file infomation succeed\n");
			exit(0);
		}

		waitpid(pid_B, &status, NULL);
	}
	else if (pid_C > 0) {
		// 等待C进程退出
		waitpid(pid_C, &status, 0);
		printf("task is done, program exit");
		exit(0);
	}
}


// 三个进程是一起创建出来之后再开始执行任务的
/* father_process+
*                |--process_C+
*                     ↑      |--process_B+
*                     ↑              ↑   |--process_A
*                     ↑              ↑           ↑ 
*                  wait for B        ↑           ↑
*                                wait for A      ↑ 
*                                               do something
*/




/* 
* #include <sys/types.h>
* #include <sys/wait.h>
* 
* pid_t wait(int *status);								// 等待任意一个子进程，！！会阻塞父进程！
* pid_t waitpid(pid_t pid, int *status, int options);	// 指定进程号的进程等待，！！会阻塞父进程!
* 
* status不仅仅是一个int类型参数，可以看作是一个位图。我们只研究其低16位。
* [CSDN]https://blog.csdn.net/ranjiahao_study/article/details/102620991
* 
* sys/wait.h中提供了一些宏来简化这些操作：
* if (WIFEXITED(status)) {
*	// 正常退出：((status) & 0x7f) == 0
* 	// 打印退出码：(status >> 8) & 0xff
* 	printf("child return: %d\n", WEXITSTATUS(status));
* } else if (WIFSIGNALED(status)) {
*	// 异常退出：((signed char) (((status) & 0x7f) + 1) >> 1) > 0
*	// 打印异常信号值：(status) & 0x7f
* 	printf("child signal: %d\n", WTERMSIG(status));
* }
* 
*/


