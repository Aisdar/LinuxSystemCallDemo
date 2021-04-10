#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

using namespace std;

// Lucy端，可以接收Jack发送到管道pipe2.fifo里内容
void named_pipe_Lucy() {

	char buf[128] = { '\0' };					// 输入缓冲区

	pid_t pid_write = fork();
	if (pid_write < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid_write == 0) {
		// 写进程
		int fd_write_fifo;							// 命名管道的fd描述符

		int w_size = -1;							// 读取到的字符大小

		char* path = "/root/projects/pipe2.fifo";	// 命名管道的文件名

		// 判断管道是否存在
		if (access(path, F_OK) == -1) {
			// 无则创建
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}
		fd_write_fifo = open(path, O_WRONLY);		// 打开文件
		if (fd_write_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		cout << "聊天室开启！你的名字：Lucy" << endl;
		while (1) {
			fgets(buf, sizeof(buf), stdin);				// stdin宏表示控制台输入
			string name = "Lucy";						// 名字
			string content = name + ":" + string(buf);	// 聊天内容=名字:内容

			w_size = write(fd_write_fifo, content.c_str(), content.length());	// 写管道内容
			if (w_size < 0) {
				perror("write pipe2 error");
				exit(-1);
			}
			bzero(buf, sizeof(buf));
		}
	}
	else if (pid_write > 0) {
		// 读进程

		int fd_read_fifo;							// 命名管道的fd描述符
		int r_size = -1;							// 读取到的字符大小

		// 判断管道是否存在
		char* path = "/root/projects/pipe1.fifo";	// 命名管道的文件名
		if (access(path, F_OK) == -1) {
			// 无则创建
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}

		fd_read_fifo = open(path, O_RDONLY);		// 打开文件
		if (fd_read_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		while (1) {
			r_size = read(fd_read_fifo, buf, sizeof(buf));	// 读管道内容
			if (r_size < 0) {
				perror("read pipe1 error");
				exit(-1);
			}

			fputs(buf, stdout);			// 输出到终端
			bzero(buf, sizeof(buf));
		}
	}
}


// Jack端，可以接收Lucy发送到管道pipe2.fifo里内容
void named_pipe_Jack() {
	char buf[128] = { '\0' };					// 输入缓冲区

	pid_t pid_write = fork();
	if (pid_write < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid_write == 0) {
		// 写进程
		int fd_write_fifo;							// 命名管道的fd描述符
		
		int w_size = -1;							// 读取到的字符大小

		// 判断管道是否存在
		char* path = "/root/projects/pipe1.fifo";	// 命名管道的文件名

		if (access(path, F_OK) == -1) {
			// 无则创建
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}
		fd_write_fifo = open(path, O_WRONLY);		// 打开文件
		if (fd_write_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		cout << "聊天室开启！你的名字：Jack" << endl;
		string name = "Jack";
		while (1) {
			fgets(buf, sizeof(buf), stdin);				// stdin宏表示控制台输入
			string content = name + ":" + string(buf);	// 聊天内容=名字:内容

			w_size = write(fd_write_fifo, content.c_str(), content.length());
			if (w_size < 0) {
				perror("write pipe1 error");
				exit(-1);
			}
			bzero(buf, sizeof(buf));
		}
	}
	else if (pid_write > 0) {
		// 写进程
		int fd_read_fifo;							// 命名管道的fd描述符
		int r_size = -1;							// 读取到的字符大小

		// 判断管道是否存在
		char* path = "/root/projects/pipe2.fifo";	// 命名管道的文件名
		if (access(path, F_OK) == -1) {
			// 无则创建
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}

		fd_read_fifo = open(path, O_RDONLY);		// 打开文件
		if (fd_read_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		while (1) {
			r_size = read(fd_read_fifo, buf, sizeof(buf));
			if (r_size < 0) {
				perror("read pipe1 error");
				exit(-1);
			}

			fputs(buf, stdout);						// 输出到终端
			bzero(buf, sizeof(buf));
		}
	}
}