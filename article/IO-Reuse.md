# socket I/O复用 模型

在编写前置和后置服务器时，可以采用消息队列和共享内存的机制，或者信号量和共享内存的机制，信号量和消息队列任选其一

## 什么是阻塞式

阻塞调用是指调用结果返回之前，当前线程会被挂起。调用线程只有在得到结果之后才会返回。

非阻塞调用指在不能立刻得到结果之前，该调用不会阻塞当前线程。

> 阻塞式的方式在业务处理时会尽量少地浪费CPU时间

## 阻塞式I/0和非阻塞式I/0

- 阻塞式I/0:类似于read函数，自动从运行态转换成阻塞态，直到等待的资源被分配。

- 非阻塞式I/0:当前请求的I/О操作无法完成返回一个错误信息，处于一种忙轮询的状态，直到操作准备好。

## read函数的 阻塞式/非阻塞 执行

- 阻塞式

最流行的I/O模型是阻塞I/O模型，缺省时，所有的套接口都是阻塞的。

<p align="center"><img src="https://gitee.com/aiyudehua/drawing-bed/raw/master/https://gitee.com/aiyudehua/drawing-bed/tree/master/img/image-20210424105925463.png" width="1000" height="600" /> </p>
<p align="center">阻塞I/O模型</p>

- 非阻塞式

  
 <p align="center"><img src="https://gitee.com/aiyudehua/drawing-bed/raw/master/https://gitee.com/aiyudehua/drawing-bed/tree/master/img/image-20210424105959762.png" width="1000" height="600" /> </p>
<p align="center">非阻塞I/O模型</p> 

应用程序连续不断地查询内核，访问某操作是否准备好，这无疑是对CPU时间的巨大浪费。

## I/0复用的场合

- 一个客户同时处理多个描述字
- 一个客户同时处理多个套接字
- 如果一个服务器既要处理监听套接口，又要处理连接套接口
- 如果一个服务器既要处理TCP，又要处理UDP

## 多路复用性I/O

如果一个或多个I/O条件满足时我们能够被通知到。

例如：输入已准备好被读，或者描述字可以承接更多输出时

我们称这种能力为I/O复用。在Linux系统中它们是由select函数和poll函数支持的。

## select函数

select函数允许进程指示内核等待多个事件中的任一个发生，并仅在一个或多个事件发生或经过某指定的时间后才唤醒进程。

```c
#include <sys/select.h>
int select(int nfds,				// 等待最大套接字值加1（等待套接字的数量）
           fd_set *readfds, 		// 待检查读事件的容器
           fd_set *writefds,		// 待检查写时间的容器
           fd_set *exceptfds,		// 
           struct timeval *timeout);// 超时时间
```

- 返回值：返回触发套接字的个数
参数中的fd_set类型需要有特别的宏指定其行为

**操作fd_set的宏**

```c
void FD_ZERO(df_set *fdset);		// 清空描述子集合
void FD_SET(int fd, fd_set *fdset);	// 添加一个描述字到集合
void FD_CLR(int fd, fd_set *fdset);	// 从集合中删除一个描述字
int FD_ISSET(int fd, fd_set *fdset);// 描述字是否在该集合中
```

select的缺点：

- 单个进程能够监视的文件描述符的数量存在最大限制，通常是1024，当然可以更改数量，但由于select采用轮询的方式扫描文件描述符，文件描述符数量越多，性能越差
- 内核/用户空间内存拷贝问题，select需要复制大量的句柄数据结构，产生巨大的开销
- select返回的是含有整个句柄的数组，应用程序需要遍历整个数组才能发现哪些句柄发生了事件
- select的触发方式是水平触发，应用程序如果没有完成对一个已经就绪的文件描述符进行IO，那么之后再次select调用还是会将这些文件描述符通知进程

## poll函数

相比于select模型，poll使用链表保存文件描述符，因此没有了监视文件数量的限制，但其他三个缺点依然存在。

但poll的链表的长度是不限的，长度不固定，同时仍然是无差别轮询的方式去检查件时的文件描述符。

## ==epoll==

epoll在Linux2.6内核正式提出，是基于事件驱动的I/O方式，相对于select来说，epoll没有描述符个数限制，使用一个文件描述符管理多个描述符，将用户关心的文件描述符的事件存放到内核的一个事件表中，这样在用户空间和内核空间的copy只需一次。

epoll是Linux内核为处理大批量文件描述符而作了改进的poll，是linux下多路复用IO接口select/poll的增强版本，它能显著提高程序在大量并发连接中只有少量活跃的情况下的系统CPU利用率。原因就是获取事件的时候，**它无须遍历整个被侦听的描述符集，只要遍历那些被内核IO事件异步唤醒而加入Ready队列的描述符集合就行了。**(无差别轮询变为轮询就绪队列)

**epoll_ctl函数**

```C
int epoll_ctl(int epfd,						// epoll句柄
              int op,						// op表示fd操作类型
              int fd,
              struct epoll_event *event);

```

- epfd：epoll的句柄
- op：fd操作类型，表示此函数即将对epfd执行的操作，有以下几种宏来刻画
  - EPOLL_CTL_ADD 注册新的fd到epfd中
  - EPOLL_CTL_MOD 修改已注册fd的监听事件
  - EPOLL_CTL_DEL 从epgd中删除一个fd
- event：待监听的事件

```c
typedef union epoll_data {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;
struct epoll_event {
               uint32_t     events;      /* Epoll events */
               epoll_data_t data;        /* User data variable */
           };
```

对于如上的结构体，我们在使用epoll_ctl()函数时一定会用到。可以认为epoll_ctl()函数实际操作的依据就是epoll_event结构体。

epoll_event结构体参数：

- events：
  - 

```c
int epoll_wait(int epfd,
               struct epoll_event *events,
			   int maxevents,
               int timeout);
```

参数：

- epfd：epoll句柄
- events：便是内核得到的就绪时间集合（数组）
- maxevents：告知内核events的大小，*必须和events参数的大小一致！*
- timeout：等待的超时事件

## 代码示例

[IO复用-客户端](https://github.com/fjnucym/LinuxSystemCallDemo/blob/master/project/IO_Reuse_Client.cpp)

[IO复用-服务器](https://github.com/fjnucym/LinuxSystemCallDemo/blob/master/project/IO_Reuse_Server.cpp)