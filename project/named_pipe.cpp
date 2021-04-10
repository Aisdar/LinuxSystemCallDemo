#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

using namespace std;

// Lucy�ˣ����Խ���Jack���͵��ܵ�pipe2.fifo������
void named_pipe_Lucy() {

	char buf[128] = { '\0' };					// ���뻺����

	pid_t pid_write = fork();
	if (pid_write < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid_write == 0) {
		// д����
		int fd_write_fifo;							// �����ܵ���fd������

		int w_size = -1;							// ��ȡ�����ַ���С

		char* path = "/root/projects/pipe2.fifo";	// �����ܵ����ļ���

		// �жϹܵ��Ƿ����
		if (access(path, F_OK) == -1) {
			// ���򴴽�
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}
		fd_write_fifo = open(path, O_WRONLY);		// ���ļ�
		if (fd_write_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		cout << "�����ҿ�����������֣�Lucy" << endl;
		while (1) {
			fgets(buf, sizeof(buf), stdin);				// stdin���ʾ����̨����
			string name = "Lucy";						// ����
			string content = name + ":" + string(buf);	// ��������=����:����

			w_size = write(fd_write_fifo, content.c_str(), content.length());	// д�ܵ�����
			if (w_size < 0) {
				perror("write pipe2 error");
				exit(-1);
			}
			bzero(buf, sizeof(buf));
		}
	}
	else if (pid_write > 0) {
		// ������

		int fd_read_fifo;							// �����ܵ���fd������
		int r_size = -1;							// ��ȡ�����ַ���С

		// �жϹܵ��Ƿ����
		char* path = "/root/projects/pipe1.fifo";	// �����ܵ����ļ���
		if (access(path, F_OK) == -1) {
			// ���򴴽�
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}

		fd_read_fifo = open(path, O_RDONLY);		// ���ļ�
		if (fd_read_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		while (1) {
			r_size = read(fd_read_fifo, buf, sizeof(buf));	// ���ܵ�����
			if (r_size < 0) {
				perror("read pipe1 error");
				exit(-1);
			}

			fputs(buf, stdout);			// ������ն�
			bzero(buf, sizeof(buf));
		}
	}
}


// Jack�ˣ����Խ���Lucy���͵��ܵ�pipe2.fifo������
void named_pipe_Jack() {
	char buf[128] = { '\0' };					// ���뻺����

	pid_t pid_write = fork();
	if (pid_write < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid_write == 0) {
		// д����
		int fd_write_fifo;							// �����ܵ���fd������
		
		int w_size = -1;							// ��ȡ�����ַ���С

		// �жϹܵ��Ƿ����
		char* path = "/root/projects/pipe1.fifo";	// �����ܵ����ļ���

		if (access(path, F_OK) == -1) {
			// ���򴴽�
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}
		fd_write_fifo = open(path, O_WRONLY);		// ���ļ�
		if (fd_write_fifo < 0) {
			perror("pipe open error");
			exit(-1);
		}

		cout << "�����ҿ�����������֣�Jack" << endl;
		string name = "Jack";
		while (1) {
			fgets(buf, sizeof(buf), stdin);				// stdin���ʾ����̨����
			string content = name + ":" + string(buf);	// ��������=����:����

			w_size = write(fd_write_fifo, content.c_str(), content.length());
			if (w_size < 0) {
				perror("write pipe1 error");
				exit(-1);
			}
			bzero(buf, sizeof(buf));
		}
	}
	else if (pid_write > 0) {
		// д����
		int fd_read_fifo;							// �����ܵ���fd������
		int r_size = -1;							// ��ȡ�����ַ���С

		// �жϹܵ��Ƿ����
		char* path = "/root/projects/pipe2.fifo";	// �����ܵ����ļ���
		if (access(path, F_OK) == -1) {
			// ���򴴽�
			if (mkfifo(path, 0777) == -1) {
				perror("mkfifo error");
				exit(-1);
			}
		}

		fd_read_fifo = open(path, O_RDONLY);		// ���ļ�
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

			fputs(buf, stdout);						// ������ն�
			bzero(buf, sizeof(buf));
		}
	}
}