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


/* ����Ҫ�� */
/* Ҫ��ʹ���������� A B C , ��ִ������֮ǰ
*  �� C���� �ȴ� B���� ��B���� �ȴ� A����
* A���̸��𿽱�������ļ� file����ͬ��Ŀ¼�µõ�file-���� 
* B���̸����� file-���� ��ĩβ����"Hello Linux"
* C���̸��� file-���� ���������������̨
*/
void demo_wait_waitpid(const char *file_path) {
	// �ж��ļ�����
	struct stat buf_stat;
	if (!stat(file_path, &buf_stat) < 0) {
		perror("lstat error:");
		return;
	}
	if (!S_ISREG(buf_stat.st_mode)){
		printf("not a file\n");
		return;
	}


	string file_duplication_path = string(file_path) + "-copy";		// �ļ��ĸ�����


	pid_t pid_A = fork();
	fork_error_msg(pid_A, "fork A process:");

	// A����--�����ļ����õ�ͬ��Ŀ¼�µĸ���
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
		// �ر������ļ�
		int close_code = close(from_fd);
		close_error_msg(close_code, "sorce file close error");
		close_code = close(to_fd);
		close_error_msg(close_code, "target file close error");

		printf("A process copyfile succeed\n");
		exit(0);
	}
	
	// �ȴ�A���̽���
	int status = -1;
	waitpid(pid_A, &status, NULL);
	printf("father process recovered A process\n\n");

	pid_t pid_B = fork();
	fork_error_msg(pid_B, "fork B process:");

	if (pid_B == 0) {
		int fd = open(file_duplication_path.c_str(), O_RDWR);
		open_error_msg(fd, "duplication file open error");

		int seeklen = -1;
		seeklen = lseek(fd, 0, SEEK_END);	// ��λ���ļ�ĩβ, ƫ��(offset = 0)
		if (seeklen < 0) {
			perror("seek file end error");
			exit(-1);
		}
		// д��Hello Linux
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

	// �ȴ�B���̽���
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
		printf("\n");	// ˢ�»�����

		int closeFlg = close(fd);
		close_error_msg(closeFlg, "duplication file close error");

		printf("\nC process write file infomation succeed\n");
		exit(0);
	}

	// �ȴ�C���̽���
	waitpid(pid_C, &status, NULL);
	printf("father process recovered C process\n\n");
	// ��ӡһ���˳���status�������й�status��ֵ�����н��ܣ�status��int���͡�
	// status���ӽ��̽���ʱ��״̬��ֱ�ӹ�������ֱ�Ӵ�ӡ�������������Ҫʹ�ú궨�庯��
	printf("child return: %d\n", WEXITSTATUS(status));
}


// �������̲�����ͬʱ������
/* father_process+
*                |-- process_A
*                |-- process_B
*                |-- process_C
*/

/******************************************************************************/
/* ����Ҫ�� */
/* Ҫ��ʹ���������� A B C , ��ִ������֮ǰ��Ҫ�����������Ѿ�������������
*  �� C���� �ȴ� B���� ��B���� �ȴ� A����
* A���̸��𿽱�������ļ� file����ͬ��Ŀ¼�µõ�file-����
* B���̸����� file-���� ��ĩβ����"Hello Linux"
* C���̸��� file-���� ���������������̨
*/
void demo_wait_waitpid2(const char* file_path) {
	// �ж��ļ�����
	struct stat buf_stat;
	if (!stat(file_path, &buf_stat) < 0) {
		perror("lstat error:");
		return;
	}
	if (!S_ISREG(buf_stat.st_mode)) {
		printf("not a file\n");
		return;
	}

	string file_duplication_path = string(file_path) + "-copy";		// �ļ��ĸ�����



	int status;	
	pid_t pid_C = fork();
	fork_error_msg(pid_C, "fork C process error");
	
	// C����
	if (pid_C == 0) {		
		pid_t pid_B = fork();
		fork_error_msg(pid_B, "fork B process error");

		// B����
		if (pid_B == 0) {		
			pid_t pid_A = fork();
			fork_error_msg(pid_A, "fork A process error");


			// A����
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
				// �ر������ļ�
				int close_code = close(from_fd);
				close_error_msg(close_code, "sorce file close error");
				close_code = close(to_fd);
				close_error_msg(close_code, "target file close error");

				printf("A process copyfile succeed\n");
				exit(0);
			}
			else if (pid_A > 0) {
				// B���̵ȴ�A���̽������ܿ�ʼ
				waitpid(pid_A, &status, 0);
				// B do something
				int fd = open(file_duplication_path.c_str(), O_RDWR);
				open_error_msg(fd, "duplication file open error");

				int seeklen = -1;
				seeklen = lseek(fd, 0, SEEK_END);	// ��λ���ļ�ĩβ, ƫ��(offset = 0)
				if (seeklen < 0) {
					perror("seek file end error");
					exit(-1);
				}
				// д��Hello Linux
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
			// �ȴ�B�����˳���C���ܿ�ʼ
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
			printf("\n");	// ˢ�»�����

			int closeFlg = close(fd);
			close_error_msg(closeFlg, "duplication file close error");

			printf("\nC process write file infomation succeed\n");
			exit(0);
		}

		waitpid(pid_B, &status, NULL);
	}
	else if (pid_C > 0) {
		// �ȴ�C�����˳�
		waitpid(pid_C, &status, 0);
		printf("task is done, program exit");
		exit(0);
	}
}


// ����������һ�𴴽�����֮���ٿ�ʼִ�������
/* father_process+
*                |--process_C+
*                     ��      |--process_B+
*                     ��              ��   |--process_A
*                     ��              ��           �� 
*                  wait for B        ��           ��
*                                wait for A      �� 
*                                               do something
*/




/* 
* #include <sys/types.h>
* #include <sys/wait.h>
* 
* pid_t wait(int *status);								// �ȴ�����һ���ӽ��̣����������������̣�
* pid_t waitpid(pid_t pid, int *status, int options);	// ָ�����̺ŵĽ��̵ȴ�������������������!
* 
* status��������һ��int���Ͳ��������Կ�����һ��λͼ������ֻ�о����16λ��
* [CSDN]https://blog.csdn.net/ranjiahao_study/article/details/102620991
* 
* sys/wait.h���ṩ��һЩ��������Щ������
* if (WIFEXITED(status)) {
*	// �����˳���((status) & 0x7f) == 0
* 	// ��ӡ�˳��룺(status >> 8) & 0xff
* 	printf("child return: %d\n", WEXITSTATUS(status));
* } else if (WIFSIGNALED(status)) {
*	// �쳣�˳���((signed char) (((status) & 0x7f) + 1) >> 1) > 0
*	// ��ӡ�쳣�ź�ֵ��(status) & 0x7f
* 	printf("child signal: %d\n", WTERMSIG(status));
* }
* 
*/


