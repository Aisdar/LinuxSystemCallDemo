#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


using namespace std;


typedef struct chat {
	int chat_type;		// �������� ˽��|Ⱥ��
	int sender_id;		// [��Ҫ]���ͷ�id	˫�����뻥��ָ��
	int receiver_id;	// [��Ҫ]���շ�id	˫�����뻥��ָ��
	char message[100];	// ��Ϣ
}CHAT;

int socket_fd;
struct sockaddr_in s_addr;	// �����׽��ֽṹ��
int len;
pthread_t read_pthread;
pthread_t write_pthread;
CHAT send_buf;				// ������Ϣ�ṹ��
CHAT rcv_buf;				// ������Ϣ�ṹ��
char buf[50];

/*
* �ͻ��� �� �̺߳���
* @param p ������߳�ʹ�õĲ���
*/
void* client_read(void* p) {
	while (1) {
		read(socket_fd, &rcv_buf, sizeof(rcv_buf));
		cout << "�ͻ���" << rcv_buf.receiver_id << ":" << rcv_buf.message << endl;
		bzero(rcv_buf.message, sizeof(rcv_buf.message));
	}
}

/*
* �ͻ��� д �̺߳���
* @param p ������߳�ʹ�õĲ���
*/
void* client_write(void* p) {
	send_buf.chat_type = 0;		// ˽��
	cout << "�����Լ���id��";
	cin >> send_buf.sender_id;
	cout << "������շ���id��";
	cin >> send_buf.receiver_id;
	// [��Ҫ] �ͻ����Ա���Ϣ������������޷����м�¼��Ҳ���޷�Ϊ�俪��ת���߳�
	write(socket_fd, &send_buf, sizeof(send_buf));
	while (1) {
		cout << "���뷢�����ݣ�" << endl;
		cin >> send_buf.message;
		write(socket_fd, &send_buf, sizeof(send_buf));
		bzero(send_buf.message, sizeof(send_buf.message));
	}
}

void client() {

	/*
	* �����ͨ·
	* ����1��Э����
	* ����2��Э������
	* ����3����Ϊ�ڶ��������Ѿ�ȷ����Э�����ͣ�Ĭ��0
	*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0) {
		perror("socket error");
	}
	else {
		// Э����
		s_addr.sin_family = AF_INET;
		// ���ö˿ں� 9000���ϣ�����֮�⻹�д�С�����⣬����ͨ��htonsת���������
		s_addr.sin_port = htons(10086);
		// ����IP��Linux����ϵͳĬ���ṩ
			// �ػ���ַ 127.0.0.1
		s_addr.sin_addr.s_addr = inet_addr("192.168.150.199");// ע�⽨��ʹ�� s_addr.sin_addr.s_addr ������addr
		len = sizeof(s_addr);
		// connect�ǵ�Ҫʹ��if�����֤�ķ�ʽ
		if (connect(socket_fd, (struct sockaddr*)&s_addr, len)) {
			perror("connect error");
			exit(-1);
		}

		// �ͻ��˿�������д�߳����������Ϣ
		if (pthread_create(&write_pthread, NULL, client_write, NULL) < 0) {
			perror("pthread create error");
		}

		if (pthread_create(&read_pthread, NULL, client_read, NULL) < 0) {
			perror("pthrea create error");
		}

		// [��Ҫ]���߳���ѭ���ȴ�
		while (1);
	}
	return 0;
}