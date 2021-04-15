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
map<int, int> accept_fd_map;


/* ������ת������
* @param��p �����̺߳���ʹ�õĲ���
*/
void* server_transpond(void* p) {
	int fd = *(int*)p;
	// �Զ���������Ϣ�ṹ��
	CHAT chat_buf;
	// ���տͻ��˿���ʱ�������Ա���Ϣ�ṹ�壬����ÿ���ͻ��˶��ᾭ���Ĺ���
	read(fd, &chat_buf, sizeof(CHAT));
	// key = ���ͷ�id
	// value = �������뷢�ͷ������������ļ�������accept_fd
	accept_fd_map[chat_buf.sender_id] = fd;
	cout << "�Ѿ�Ϊ�ͻ���" << chat_buf.sender_id << "������" << chat_buf.receiver_id << "��ת������" << endl;
	while (1) {
		read(fd, &chat_buf, sizeof(CHAT));
		cout << "���յ��ͻ���" << chat_buf.sender_id << "�����ͻ���" << chat_buf.receiver_id << "����Ϣ:"
			<< chat_buf.message << endl;
		// �������߿ͻ�����Ѱ�� receiver
		if (accept_fd_map.find(chat_buf.receiver_id) == accept_fd_map.end()) {
			cout << "�ͻ���" << chat_buf.receiver_id << "������" << endl;
			continue;
		}
		write(accept_fd_map[chat_buf.receiver_id], &chat_buf, sizeof(chat_buf));
		bzero(chat_buf.message, sizeof(chat_buf.message));
	}
}

int client() {

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


/* ����bind error:Address already in use */
// ԭ��һ����˵��һ���˿��ͷź��ȴ�������֮������ٱ�ʹ�ã�SO_REUSEADDR���ö˿��ͷź������Ϳ��Ա��ٴ�ʹ�á�
/* ��������������ʹ�� $ netstat -an|grep �˿ں� �鿴��Ӧ�˿ںŵ�״̬
*  ���������ܴ���TIME_WAIT ���� ESTABLISHED ״̬
*  ʹ�� $ fuser -k �˿ں�/tcp ɱ�����̶˿�
*  ����bind֮ǰʹ����δ��룬
*	int val = 1;
*	int ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(int));
*	if (ret == -1){
*		printf("setsockopt");
*		exit(1);
*	}
*	Q:��д TCP/SOCK_STREAM �������ʱ��SO_REUSEADDR����ʲô��˼��
*
*   A:����׽���ѡ��֪ͨ�ںˣ�����˿�æ����TCP״̬λ�� TIME_WAIT ���������ö˿ڡ�����˿�æ����TCP״̬λ������״̬�����ö˿�ʱ���ɵõ�һ��������Ϣ��ָ��"��ַ�Ѿ�ʹ����"�������ķ������ֹͣ�������������������׽�������ʹ��ͬһ�˿ڣ���ʱSO_REUSEADDR ѡ��ǳ����á�������ʶ������ʱ�κη��������ݵ�������ܵ��·������Ӧ���ң�������ֻ��һ�ֿ��ܣ���ʵ�Ϻܲ����ܡ�
*
*   һ���׽����������Ԫ�鹹�ɣ�Э�顢���ص�ַ�����ض˿ڡ�Զ�̵�ַ��Զ�̶˿ڡ�SO_REUSEADDR ������ʾ�������ñ��ر��ص�ַ�����ض˿ڣ����������Ԫ�黹��Ψһȷ���ġ����ԣ�������ķ�������п����յ����������ݡ���������ʹ��SO_REUSEADDR ѡ���2��
*/