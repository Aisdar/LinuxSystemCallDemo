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
	int chat_type;		// 聊天类型 私聊|群聊
	int sender_id;		// [重要]发送方id	双方必须互相指定
	int receiver_id;	// [重要]接收方id	双方必须互相指定
	char message[100];	// 信息
}CHAT;


int socket_fd;
int accept_fd;
struct sockaddr_in s_addr;		// 网络套接字结构体
pthread_t transpond_pthread_id;	// 客户端上线时为其开启的转发线程ID
int len;
map<int, int> accept_fd_private_map;		// 记录私聊在线的人	// key = 客户端账号
map<int, int> accept_fd_group_map;	// 记录群聊在线的人	// value = accept_fd
pthread_mutex_t accept_private_mutex;
pthread_mutex_t accept_group_mutex;

/// <summary>
/// 服务器转发函数
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
void* server_transpond(void* p) {
	int fd = *(int*)p;
	// 自定义聊天消息结构体
	CHAT chat_buf;
	// 接收客户端开启时发来的自报信息结构体，这是每个客户端都会经历的过程
	read(fd, &chat_buf, sizeof(CHAT));
	// key = 发送方id
	// value = 服务器与发送发建立的连接文件描述符accept_fd
	// 根据聊天类型存储到对应的map当中，这里代码这么写要保证客户端给的聊天类型信息正确
	if (chat_buf.chat_type == PRIVATE_CHAT) {
		accept_fd_private_map[chat_buf.sender_id] = fd;
		cout << "已经为客户端" << chat_buf.sender_id << "建立向" << chat_buf.receiver_id << "的转发服务" << endl;
	}
	else if (chat_buf.chat_type == GROUP_CHAT) {
		accept_fd_group_map[chat_buf.sender_id] = fd;
		cout << "已经为客户端" << chat_buf.sender_id << "建立群聊服务" << endl;
	}
	else {
		cout << "客户端聊天类型错误，为客户端" << chat_buf.sender_id << "开启服务失败" << endl;
		return NULL;
	}


	while (1) {
		// 读取客户端信息
		read(fd, &chat_buf, sizeof(CHAT));

		if (chat_buf.chat_type == PRIVATE_CHAT) {
			// 私发
			cout << "接收到客户端" << chat_buf.sender_id << "发往客户端" << chat_buf.receiver_id << "的消息:"
				<< chat_buf.message << endl;

			pthread_mutex_lock(&accept_private_mutex);	// -------------加锁--------------------

			// 在以上线客户端中寻找 receiver
			if (accept_fd_private_map.find(chat_buf.receiver_id) == accept_fd_private_map.end()) {
				cout << "客户端" << chat_buf.receiver_id << "不在线" << endl;
				continue;
			}
			write(accept_fd_private_map[chat_buf.sender_id], &chat_buf, sizeof(chat_buf));		// 发回给发送方
			write(accept_fd_private_map[chat_buf.receiver_id], &chat_buf, sizeof(chat_buf));	// 发送给接收方

			pthread_mutex_unlock(&accept_private_mutex);	// -------------解锁--------------------

			bzero(chat_buf.message, sizeof(chat_buf.message));
		}
		else if (chat_buf.chat_type == GROUP_CHAT) {
			// 群发
			cout << "接收到客户端" << chat_buf.sender_id << "发来的群聊消息:" << chat_buf.message << endl;
			// 接收到某客户端的信息，在群聊map中的连接挨个写数据

			pthread_mutex_lock(&accept_group_mutex);	// -------------加锁--------------------

			map<int, int>::iterator it = accept_fd_group_map.begin();
			for (; it != accept_fd_group_map.end(); it++) {
				write(it->second, &chat_buf, sizeof(chat_buf));
			}

			pthread_mutex_unlock(&accept_group_mutex);	// -------------解锁--------------------

			bzero(chat_buf.message, sizeof(chat_buf.message));
		}

	}
}

int client_group() {
	// 初始化互斥锁
	pthread_mutex_init(&accept_private_mutex, NULL);
	pthread_mutex_init(&accept_group_mutex, NULL);
	/*
	* 搭建网络通路
	* 参数1：协议族
	* 参数2：协议类型
	* 参数3：因为第二个参数已经确定了协议类型，默认0
	*/
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
		//（但此参数不重要），后期可以通过别的手段扩大连接数
		if (listen(socket_fd, 10)) {
			perror("listen error");
		}

		int client_count = 0;	// 客户端在线数量
		// 需要服务器长时间等待客户端上线
		while (1) {
			// 之前经过bind函数绑定后，accept的后两个参数可以不需要(给NULL)，这是版本更新留下的接口
			// 阻塞式函数，一直等待客户端访问，客户端一但访问成即返回accept_fd
			// 后续我们想要写入数据需要通过这个accept_fd
			accept_fd = accept(socket_fd, NULL, NULL);
			cout << "有客户端上线" << endl;

			// 开启子线程执行两客户端之间的消息转发
			// [重要]我们需要给子线程传递发送发的accept_fd
			//		虽然我们以及执行了记录
			if (pthread_create(&transpond_pthread_id, NULL, server_transpond, &accept_fd)) {
				perror("socket error");
			}
		}
		close(socket_fd);
	}
	return 0;
}
