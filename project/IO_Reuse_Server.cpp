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
struct sockaddr_in s_addr;		// 网络套接字结构体
int len;
map<int, int> group;			// 群聊fd
map<int, int> single;			// 群聊fd

typedef struct chat {
	int chat_type;		// 聊天类型 私聊|群聊
	int sender_id;		// [重要]发送方id	双方必须互相指定
	int receiver_id;	// [重要]接收方id	双方必须互相指定
	char message[100];	// 信息
}CHAT;

CHAT chat_buf;

int main() {
	// epoll描述字
	int epollwait_fd;
	int epoll_fd;
	// 创建epoll事件结构体
	struct epoll_event my_epoll_event;
	struct epoll_event my_epoll_event_array[5];

	/*----------------------------建立网络连接------------------------------*/
		// 搭网络通路
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0) {
		perror("socket error");
	}
	else {
		// 协议族，一般情况下 IPV4=AF_INET
		s_addr.sin_family = AF_INET;
		// 设置端口号 9000以上，除此之外还有大小端问题，可以通过htons转换函数解决
		s_addr.sin_port = htons(10086);
		// 设置IP有Linux操作系统默认提供
			// 回环地址 127.0.0.1
		s_addr.sin_addr.s_addr = INADDR_ANY;// 注意建议使用 s_addr.sin_addr.s_addr 有两层addr

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
		// 设置监听网络是否可行 设置网路同一时间能够接收几个客户端
		if (listen(socket_fd, 10)) {
			perror("listen error");
		}

		/*----------------------------初始化epoll事件结构体----------------------*/
		bzero(&my_epoll_event, sizeof(my_epoll_event));
		// 绑定当前已经准备好的可用网络socket_fd
		my_epoll_event.data.fd = socket_fd;
		// 绑定事件为进入事件 （宏）
		my_epoll_event.events = EPOLLIN;

		// 创建epoll
		epoll_fd = epoll_create(5);
		// 将已经准备好的网络描述符添加到epoll中
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &my_epoll_event);

		/* 中心思想：轮询就绪队列！*/
		// 需要服务器长时间等待客户端上线
		while (1) {
			cout << "epoll wait ......" << endl;
			// 网络通路之后，让epoll中所有的socket处于等待状态
			epollwait_fd = epoll_wait(epoll_fd, my_epoll_event_array, 5, -1);

			if (epollwait_fd < 0) {
				perror("epollwait error");
			}

			// epoll相比select、poll的优势就在于从无差别轮询改为了轮询就绪队列（队列一般比较小）！
			for (int i = 0; i < epollwait_fd; i++) {
				if (my_epoll_event_array[i].data.fd == socket_fd) {
					// 注意，此处accept为阻塞式的！有客户端连接才会执行下边的逻辑
					accept_fd = accept(socket_fd, NULL, NULL);
					cout << "client connect ..... accept_fd=" << accept_fd << endl;
					// 上线成功客户端描述符accept_fd绑定并注册事件EPOLLIN添加到epoll中
					my_epoll_event.data.fd = accept_fd;
					my_epoll_event.events = EPOLLIN;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &my_epoll_event);
				}
				else if (my_epoll_event_array[i].events & EPOLLIN) {
					bzero(&chat_buf, sizeof(chat_buf));
					int res = read(my_epoll_event_array[i].data.fd, &chat_buf, sizeof(chat_buf));
					if (res <= 0) {
						cout << "client " << chat_buf.sender_id << " disconnect....." << endl;
						// 客户端掉线了，我们需要将对应的accept_fd清除一下
							// [1]关闭该客户端的描述符
						close(my_epoll_event_array[i].data.fd);
						// [2]从epoll中删除客户端文件描述符
						my_epoll_event.data.fd = my_epoll_event_array[i].data.fd;
						my_epoll_event.events = EPOLLIN;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, my_epoll_event_array[i].data.fd, &my_epoll_event);
					}
					else if (res > 0) {

						// 消息为空，说明为 登记信息 的结构体
						if (strlen(chat_buf.message) == 0) {
							if (chat_buf.chat_type == PRIVATE_CHAT) {
								single[chat_buf.sender_id] = my_epoll_event_array[i].data.fd;
							}
							else if (chat_buf.chat_type == GROUP_CHAT) {
								group[chat_buf.sender_id] = my_epoll_event_array[i].data.fd;
							}
							cout << "客户端上线，账号：" << chat_buf.sender_id << endl;

						}
						// 消息不为空，说明是带有消息的结构体
						else {
							// 私聊
							if (chat_buf.chat_type == PRIVATE_CHAT) {
								// 发给 发送方 与 接收方
								write(single[chat_buf.receiver_id], &chat_buf, sizeof(chat_buf));
								write(single[chat_buf.sender_id], &chat_buf, sizeof(chat_buf));
							}
							// 群聊
							else if (chat_buf.chat_type == GROUP_CHAT) {
								// 发给在群聊的全部人
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