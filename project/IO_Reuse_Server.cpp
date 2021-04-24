#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <sys/epoll.h>

using namespace std;

#define PRIVATE_CHAT 0
#define GROUP_CHAT 1

int socket_fd;
int accept_fd;
struct sockaddr_in s_addr;		// �����׽��ֽṹ��
int len;
map<int, int> group;			// Ⱥ��fd
map<int, int> single;			// Ⱥ��fd

typedef struct chat {
	int chat_type;		// �������� ˽��|Ⱥ��
	int sender_id;		// [��Ҫ]���ͷ�id	˫�����뻥��ָ��
	int receiver_id;	// [��Ҫ]���շ�id	˫�����뻥��ָ��
	char message[100];	// ��Ϣ
}CHAT;

CHAT chat_buf;

int main() {
	// epoll������
	int epollwait_fd;
	int epoll_fd;
	// ����epoll�¼��ṹ��
	struct epoll_event my_epoll_event;
	struct epoll_event my_epoll_event_array[5];

	/*----------------------------������������------------------------------*/
		// ������ͨ·
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
		if (listen(socket_fd, 10)) {
			perror("listen error");
		}

		/*----------------------------��ʼ��epoll�¼��ṹ��----------------------*/
		bzero(&my_epoll_event, sizeof(my_epoll_event));
		// �󶨵�ǰ�Ѿ�׼���õĿ�������socket_fd
		my_epoll_event.data.fd = socket_fd;
		// ���¼�Ϊ�����¼� ���꣩
		my_epoll_event.events = EPOLLIN;

		// ����epoll
		epoll_fd = epoll_create(5);
		// ���Ѿ�׼���õ�������������ӵ�epoll��
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &my_epoll_event);

		/* ����˼�룺��ѯ�������У�*/
		// ��Ҫ��������ʱ��ȴ��ͻ�������
		while (1) {
			cout << "epoll wait ......" << endl;
			// ����ͨ·֮����epoll�����е�socket���ڵȴ�״̬
			epollwait_fd = epoll_wait(epoll_fd, my_epoll_event_array, 5, -1);

			if (epollwait_fd < 0) {
				perror("epollwait error");
			}

			// epoll���select��poll�����ƾ����ڴ��޲����ѯ��Ϊ����ѯ�������У�����һ��Ƚ�С����
			for (int i = 0; i < epollwait_fd; i++) {
				if (my_epoll_event_array[i].data.fd == socket_fd) {
					// ע�⣬�˴�acceptΪ����ʽ�ģ��пͻ������ӲŻ�ִ���±ߵ��߼�
					accept_fd = accept(socket_fd, NULL, NULL);
					cout << "client connect ..... accept_fd=" << accept_fd << endl;
					// ���߳ɹ��ͻ���������accept_fd�󶨲�ע���¼�EPOLLIN��ӵ�epoll��
					my_epoll_event.data.fd = accept_fd;
					my_epoll_event.events = EPOLLIN;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &my_epoll_event);
				}
				else if (my_epoll_event_array[i].events & EPOLLIN) {
					bzero(&chat_buf, sizeof(chat_buf));
					int res = read(my_epoll_event_array[i].data.fd, &chat_buf, sizeof(chat_buf));
					if (res <= 0) {
						cout << "client " << chat_buf.sender_id << " disconnect....." << endl;
						// �ͻ��˵����ˣ�������Ҫ����Ӧ��accept_fd���һ��
							// [1]�رոÿͻ��˵�������
						close(my_epoll_event_array[i].data.fd);
						// [2]��epoll��ɾ���ͻ����ļ�������
						my_epoll_event.data.fd = my_epoll_event_array[i].data.fd;
						my_epoll_event.events = EPOLLIN;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, my_epoll_event_array[i].data.fd, &my_epoll_event);
					}
					else if (res > 0) {

						// ��ϢΪ�գ�˵��Ϊ �Ǽ���Ϣ �Ľṹ��
						if (strlen(chat_buf.message) == 0) {
							if (chat_buf.chat_type == PRIVATE_CHAT) {
								single[chat_buf.sender_id] = my_epoll_event_array[i].data.fd;
							}
							else if (chat_buf.chat_type == GROUP_CHAT) {
								group[chat_buf.sender_id] = my_epoll_event_array[i].data.fd;
							}
							cout << "�ͻ������ߣ��˺ţ�" << chat_buf.sender_id << endl;

						}
						// ��Ϣ��Ϊ�գ�˵���Ǵ�����Ϣ�Ľṹ��
						else {
							// ˽��
							if (chat_buf.chat_type == PRIVATE_CHAT) {
								// ���� ���ͷ� �� ���շ�
								write(single[chat_buf.receiver_id], &chat_buf, sizeof(chat_buf));
								write(single[chat_buf.sender_id], &chat_buf, sizeof(chat_buf));
							}
							// Ⱥ��
							else if (chat_buf.chat_type == GROUP_CHAT) {
								// ������Ⱥ�ĵ�ȫ����
								map<int, int>::iterator it = group.begin();
								for (; it != group.end(); it++) {
									write(it->second, &chat_buf, sizeof(chat_buf));
								}
							}
							cout << "server receive from " << chat_buf.sender_id << ":" << chat_buf.message << endl;
						}


					}
				}
			}
		}
		close(socket_fd);
	}
	return 0;
}