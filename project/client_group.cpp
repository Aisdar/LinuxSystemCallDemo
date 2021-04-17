#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <pthread.h>

using namespace std;

#define PRIVATE_CHAT 0
#define GROUP_CHAT 1

typedef struct chat {
	int chat_type;		// �������� ˽��|Ⱥ��
	int sender_id;		// [��Ҫ]���ͷ�id	˫�����뻥��ָ��
	int receiver_id;	// [��Ҫ]���շ�id	˫�����뻥��ָ��
	char message[100];	// ��Ϣ
}CHAT;


int socket_fd;
int accept_fd;
struct sockaddr_in s_addr;		// �����׽��ֽṹ��
pthread_t transpond_pthread_id;	// �ͻ�������ʱΪ�俪����ת���߳�ID
int len;
map<int, int> accept_fd_private_map;		// ��¼˽�����ߵ���	// key = �ͻ����˺�
map<int, int> accept_fd_group_map;	// ��¼Ⱥ�����ߵ���	// value = accept_fd
pthread_mutex_t accept_private_mutex;
pthread_mutex_t accept_group_mutex;

/// <summary>
/// ������ת������
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
void* server_transpond(void* p) {
	int fd = *(int*)p;
	// �Զ���������Ϣ�ṹ��
	CHAT chat_buf;
	// ���տͻ��˿���ʱ�������Ա���Ϣ�ṹ�壬����ÿ���ͻ��˶��ᾭ���Ĺ���
	read(fd, &chat_buf, sizeof(CHAT));
	// key = ���ͷ�id
	// value = �������뷢�ͷ������������ļ�������accept_fd
	// �����������ʹ洢����Ӧ��map���У����������ôдҪ��֤�ͻ��˸�������������Ϣ��ȷ
	if (chat_buf.chat_type == PRIVATE_CHAT) {
		accept_fd_private_map[chat_buf.sender_id] = fd;
		cout << "�Ѿ�Ϊ�ͻ���" << chat_buf.sender_id << "������" << chat_buf.receiver_id << "��ת������" << endl;
	}
	else if (chat_buf.chat_type == GROUP_CHAT) {
		accept_fd_group_map[chat_buf.sender_id] = fd;
		cout << "�Ѿ�Ϊ�ͻ���" << chat_buf.sender_id << "����Ⱥ�ķ���" << endl;
	}
	else {
		cout << "�ͻ����������ʹ���Ϊ�ͻ���" << chat_buf.sender_id << "��������ʧ��" << endl;
		return NULL;
	}


	while (1) {
		// ��ȡ�ͻ�����Ϣ
		read(fd, &chat_buf, sizeof(CHAT));

		if (chat_buf.chat_type == PRIVATE_CHAT) {
			// ˽��
			cout << "���յ��ͻ���" << chat_buf.sender_id << "�����ͻ���" << chat_buf.receiver_id << "����Ϣ:"
				<< chat_buf.message << endl;

			pthread_mutex_lock(&accept_private_mutex);	// -------------����--------------------

			// �������߿ͻ�����Ѱ�� receiver
			if (accept_fd_private_map.find(chat_buf.receiver_id) == accept_fd_private_map.end()) {
				cout << "�ͻ���" << chat_buf.receiver_id << "������" << endl;
				continue;
			}
			write(accept_fd_private_map[chat_buf.sender_id], &chat_buf, sizeof(chat_buf));		// ���ظ����ͷ�
			write(accept_fd_private_map[chat_buf.receiver_id], &chat_buf, sizeof(chat_buf));	// ���͸����շ�

			pthread_mutex_unlock(&accept_private_mutex);	// -------------����--------------------

			bzero(chat_buf.message, sizeof(chat_buf.message));
		}
		else if (chat_buf.chat_type == GROUP_CHAT) {
			// Ⱥ��
			cout << "���յ��ͻ���" << chat_buf.sender_id << "������Ⱥ����Ϣ:" << chat_buf.message << endl;
			// ���յ�ĳ�ͻ��˵���Ϣ����Ⱥ��map�е����Ӱ���д����

			pthread_mutex_lock(&accept_group_mutex);	// -------------����--------------------

			map<int, int>::iterator it = accept_fd_group_map.begin();
			for (; it != accept_fd_group_map.end(); it++) {
				write(it->second, &chat_buf, sizeof(chat_buf));
			}

			pthread_mutex_unlock(&accept_group_mutex);	// -------------����--------------------

			bzero(chat_buf.message, sizeof(chat_buf.message));
		}

	}
}

int client_group() {
	// ��ʼ��������
	pthread_mutex_init(&accept_private_mutex, NULL);
	pthread_mutex_init(&accept_group_mutex, NULL);
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
		// Э���壬һ������� IPV4=AF_INET
		s_addr.sin_family = AF_INET;
		// ���ö˿ں� 9000���ϣ�����֮�⻹�д�С�����⣬����ͨ��htonsת���������
		s_addr.sin_port = htons(10086);
		// ����IP��Linux����ϵͳĬ���ṩ
			// �ػ���ַ 127.0.0.1
		s_addr.sin_addr.s_addr = INADDR_ANY;// ע�⽨��ʹ�� s_addr.sin_addr.s_addr ������addr

		len = sizeof(s_addr);

		int val = 1;
		int ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&val, sizeof(int));
		if (ret == -1) {
			printf("setsockopt");
			exit(1);
		}
		if (bind(socket_fd, (struct sockaddr*)&s_addr, len) < 0) {
			perror("bind error");
		}
		// ���ü��������Ƿ���� ������·ͬһʱ���ܹ����ռ����ͻ���
		//�����˲�������Ҫ�������ڿ���ͨ������ֶ�����������
		if (listen(socket_fd, 10)) {
			perror("listen error");
		}

		int client_count = 0;	// �ͻ�����������
		// ��Ҫ��������ʱ��ȴ��ͻ�������
		while (1) {
			// ֮ǰ����bind�����󶨺�accept�ĺ������������Բ���Ҫ(��NULL)�����ǰ汾�������µĽӿ�
			// ����ʽ������һֱ�ȴ��ͻ��˷��ʣ��ͻ���һ�����ʳɼ�����accept_fd
			// ����������Ҫд��������Ҫͨ�����accept_fd
			accept_fd = accept(socket_fd, NULL, NULL);
			cout << "�пͻ�������" << endl;

			// �������߳�ִ�����ͻ���֮�����Ϣת��
			// [��Ҫ]������Ҫ�����̴߳��ݷ��ͷ���accept_fd
			//		��Ȼ�����Լ�ִ���˼�¼
			if (pthread_create(&transpond_pthread_id, NULL, server_transpond, &accept_fd)) {
				perror("socket error");
			}
		}
		close(socket_fd);
	}
	return 0;
}
