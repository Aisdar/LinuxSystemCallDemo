# 管道

## 代码样例

[匿名管道](https://github.com/fjnucym/LinuxSystemCallDemo/blob/master/project/pipe.cpp)

[命名管道-双人聊天室案例](https://github.com/fjnucym/LinuxSystemCallDemo/blob/master/project/named_pipe.cpp)

## 什么是管道

管道是一种两个进程间进行单向通信的机制，因为管道传输数据的单向性，管道又称为半双工管道。

管道的这一特性决定了其使用的局限性。管道是Linux支持的最初Unix IPC形式之以，具有以下特点：

- 数据只能由一个进程流向另一个进程（其中一个读管道，一个写管道）；如果要进行全双工通信，需要建立两个管道。

- 管道只能用于父子进程或者兄弟进程间通信。也就是说只能用于**具有亲缘关系的进程间通信**

关于第二点提到的只有具有亲缘关系的进程具体体现在管道的写端与读端只能在某个进程被创建，之后由fork()创建子进程后建立通信关系。

## 匿名管道-命名管道

命名管道和匿名管道都可以在进程间传送消息，命名管道和匿名管道有着相同特点，但命名管道克服了匿名管道只能在具有亲缘关系的进程间通信的问题。即**命名管道允许进程间无亲缘关系的通信**。

命名管道也称为FIFO，是一种永久性的机构。FIFO文件也具有文件名、文件长度、访问许可权等属性，它也能像其它Linux文件那样被打开、关闭和删除，所以任何进程都能找到它。换句话说，即使是不同祖先的进程，也可以利用命名管道进行通信。

## 信号和消息的区别

> 我们知道，进程间信号通信机制在传递信息时是以信号为载体的，但管道通信机制的信息载体是消息。那么信号和消息之间的区别在哪里呢？
- 数据内容方面：信号只是预定义的代码，用于表示系统发生的某一状况；消息则为一组连续语句或符号，不过量也不会太大。
- 在作用方面，信号担任进程间少量信息的传送，一般为内核程序用来通知用户进程的一些异常情况的发生；消息则用于进程间交换彼此的数据。
- 发送时机方面：信号可以在任何时候发送；消息则不可以在任何时刻发送。
- 发送者方面：信号不能确定发送者是谁；消息则知道发送者是谁
- 发送对象方面：信号是发给某个进程的；消息则是发送给消息队列。
- 处理方式方面：信号可以不予理会；消息则是必须处理的
- 数据传输效率方面：信号不适合进行大量的信息传输，因为它的效率不高；消息虽然不适合大量的数据传送，但它的效率比信号强，因此适于中等数量的数据传送。

## 管道技术模型
管道技术是Linux操作系统历来以久的一种进程间通信机制。
所有的管道技术，无论是半双工的匿名管道，还是命名管道，它们都是利用FIFO排队模型来指挥进程间的通信。
对于管道，我们可以形象地把它们当作**两个实体的一个单向连接器**。
使用管道进行通信时，两端的进程向管道读写数据是通过创建管道时，系统设置的文件描述符（file_description）进行的。从本质上说，管道也是一种文件，但它又和一般文件有所不同，可以克服使用使用文件通信的两个问题，这个文件只存在内存中。
通过管道通信的两个进程，一个进程向管道写数据，另外一个从中读数据。**写入的数据每次都添加到管道缓冲区的末尾，读数据的时候都是从缓冲区的头部读出数据的**。

## 管道的接口（系统调用）

**无名管道pipe**

创建管道pipe

```c
int pipe(int filedes[2]);
```

- int pipe(int []);会建立管道，并将文件描述词（file_description）由参数filedes数组返回
- filedes[0]为管道里的读取端，可以将其传入read进行读取操作。
- filedes[1]为管道里的写入端，可以将其传入write进行写入操作。

返回值

- 若成功则返回 0，否则返回 -1，错误原因存于errno中。

错误代码
- EMFILE：进程已经用完文件描述词的最大量
- ENFILE：系统已无文件描述词可用
- EFAULT：参数filedes数组地址不合法
> 当调用成功时，函数pipe返回值为 0，否则返回值为 -1。成功返回时数组filedes被填入两个有效的文件描述符。数组的第一个元素的文件描述符供应用程序读取之用数组的第二个元素中的文件描述符可以用来供应用程序写入。

关闭管道
- 关闭管道只是将两个文件描述符关闭即可，可以使用普通的close函数逐个关闭。
> 如果管道的写入端关闭，但是还有进程尝试从管道读取的话，将被返回0，用来指出管道已不可用，并且应当关闭它。如果管道的读出端关闭，但是还有进程尝试向管道写入的话，试图写入的进程将收到一个SIGPIPE信号，至于信号的具体处理则要视其信号处理程序而定了。
管道也是进程间通信的一种技术，实际就是利用输入输出流进行进程间信息交互

## 无名管道

无名管道为建立管道的进程及其子孙提供一条以比特流方式传送消息的通信管道。

该管道再逻辑上被看作管道文件，在物理上则由文件系统的高速缓冲区构成，而很少启动外设。

发送进程利用文件系统的系统调用`write(fd[1],buf,size)`,把`buf` 中的长度为`size`字符的消息送入管道入口`fd[1]`。

接收进程则使用系统调用`read(fd[0],buf,size)`从管道出口`fd[0]`出口读出`size`字符的消息置入`buf`中。

这里，管道按FIFO（先进先出）方式传送消息，且只能单向传送消息（如图）。

<p align="center"><img src="https://ae01.alicdn.com/kf/U0259fde9cccb44629ed5d9d2540a679a8.jpg" width="400" height="200" /> </p>
<p align="center">管道文件描述符</p>





**无名管道pipe读写**
管道用于不同进程间通信。通常先创建一个管道，再通过fork函数创建一个子进程，该子进程会继承父进程创建的管道，我们在使用时自己定义父子两端作为读/写端，使用时关闭filedes数组的其中一个fd（即读端关闭写文件的文件描述符，写端关闭读文件的文件描述符）。

- 注意事项：必须在系统调用fork()前调用pipe()，否则pipe函数会被调用两次，从而创建两个管道，因为父子进程共享同一段代码段，都会各自调用pipe()，即建立两个管道，出现异常错误。

```c
#include <unistd.h>

int main(){
	int fds[2];
	pid_t pid;
    char buf[12] = { "Hello pipe" };
	int return_code = pipe(fds);	// 创建匿名管道
	if(return_code < 0){
		perror("pipe create errpr");
		return 0;
	}
	pid = fork();	// 创建子进程
	if(pid < 0){
		perror("fork error");
		return 0;
	}
	if(pid < 0){
		// 父进程 --> 写进程
        close(fds[0]);	// 关闭读的文件描述符
        write(fds[1], buf, sizeof(buf));	// 系统调用write写入管道
        close(fds[1]);	// 规范，关闭文件
	}
	else if(pid == 0){
		// 子进程 --> 读进程
		close(fds[1]);	// 关闭写的文件描述符
        read(fds[0], buf, sizeof(buf));	// 系统调用read写入管道
        cout << "child buf:" << buf << endl;
        close(fds[0]);	// 规范，关闭文件
	}
	return 0;
}
```

## 命名管道

**命名管道mkfifo**

mkfifo函数的作用是在文件系统中创建一个文件，该文件用于提供FIFO功能，即命名管道。上边提到的管道都是无名的，故称为"匿名管道"，匿名管道对于文件系统是不可见的，它的作用仅限于在父进程和子进程间进行通信。而命名管道是一个可见的文件，因此它可以用于任何两个进程之间的通信，不管这两个进程是父子进程，也不管这两个进程有没有关系。

```c
#include <sys/types.h>
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```

- pathname 是将要在文件系统中创建的一个专用文件路径。
- mode 是用来规定FIFO的读写权限。
- mkfifo函数如果调用成功，返回值为0；调用失败则返回值为-1.
```c
// 检测管道文件是否存在
if(access("/root/named_pipe.fifo", F_OK) == -1){
	perror("/root/named_pipe.fifo file is not exist!");
}
```
```c
// 创建命名管道
int reslut;
result = mkfifo("/root/named_pipe.fifo", S_OK | 0777);
if(result < 0){
    perror("named_pipe create error");
    exit(-1);
}
```

**命名管道读写**

管道的外部存在形式是文件，与其他Linux的文件一样，想要使用必须要先打开

```c
// 打开管道
int pipe_fd = open("/root/named_pipe.fifo", O_RDONLY)
```

```c
// 读端
int pipe_fd = open("/root/named_pipe.fifo", O_RDONLY)
char buf[1024] = { '\0' };
int r_size = read(pipe_fd, buf, sizeof(buf));	// 读取管道内容
```

```c
// 写端
int pipe_fd = open("/root/named_pipe.fifo", O_RDONLY)
char buf[] = { "Hello Linux" };
int w_size = write(pipe_fd, buf, sizeof(buf));	// 写入管道内容
```

可以看出命名管道想要实现通信，通信双方的进程都必须通过在文件系统中的文件名来进行交互。这就是说**想要通过命名管道通信，必须知道外部的FIFO文件名**。

*注意事项:*程序不能以O_RDWR模式打开FIFO文件。如果需要进行两个进程间双向通信，也需要两个命名管道，但它在使用open系统调用函数打开时需要注意以下几个特点。

- 如果当前操作是为了读（read）而打开FIFO文件时，若已经有相应进程为写打开该FIFO文件，则open系统调用返回成功。否则进程可能被open系统调用阻塞直到有进程为了写（write）而打开FIFO文件。当使用系统调用write系统调用且设置了非阻塞标志标志（O_RDONLY | O_NONBLOCK）时open也会返回成功。
- 如果当前打开操作是为写(write)而打开FIFO时，如果已经有相应进程为读而打开该FIFO文件，则当前打开操作将成功返回；否则，可能阻塞直到有相应进程为读而打开该FIFO（当前打开操作设置了阻塞标志）或者，返回ENXIO错误（当前打开操作没有设置阻塞标志）

**命令行创建命名管道文件**

```shell
$ mkfifo filename
```

## 管道的最大容量

```c
int fds[2];
	char buf = 'a';
	int res = pipe(fds);
	if (res < 0) {
		perror("create pipe error");
		exit(-1);
	}

	// 创建子进程，管道只有一端时开启在运行时会报出Broken Pipe错误
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork error");
		exit(-1);
	}

	if (pid == 0) {
		close(fds[0]);	// [重要]!关闭读端
		int num = 0;
		while (1) {
			num += write(fds[1], &buf, sizeof(char));	// 每次写一个char变量(1字节)
			cout << "pipe now has " << num << " bytes！" << endl;
		}
	}
	else if(pid > 0){
		close(fds[1]);	// [重要]!关闭写端
		int num = 0;
		while (1) {
			// 不做任何事情，等待管道写满
		}
	}
```

<p align="center"><img src="https://ae01.alicdn.com/kf/U8e313e1d0f2846ffa150b9df19b71687K.jpg" width="330" height="260" /> </p>
<p align="center">管道容量测试</p>

[^部分转载自AlanTu的博客]: [linux内核剖析（八）进程间通信之-管道](https://www.cnblogs.com/alantu2018/p/8991343.html)

