#include "global.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;

typedef struct student {
	string number;	// 学号
	string name;	// 姓名
	int age;		// 年龄
	float height;	// 身高
}STU;

void pipe() {
	pid_t pid;
	int mypipe[2];	// 大小一定为2
	STU buf[] = { "1234567", "吴嘉伟", 18, 188.8 };
	int res = pipe(mypipe);

	if (res < 0) {
		perror("pipe error");
		exit(-1);
	}

	pid = fork();
	if (pid < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid > 0) {
		//close(mypipe[0]);					// [重要]，父进程关闭读端
		write(mypipe[1], buf, sizeof(buf));
		// 收尸
		int status;
		waitpid(pid, &status, 0);
	}
	else if (pid == 0) {
		//close(mypipe[1]);					// [重要]，子进程关闭写端
		read(mypipe[0], buf, sizeof(buf));	// read是阻塞进程的，在读取到内容前不会返回
		cout << "学号:" << buf->number << "姓名:" << buf->name << " 年龄:" << buf->age << " 身高:" << buf->height << endl;
		exit(0);
	}
}
