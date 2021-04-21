# IPC-信号量

## 代码样例

[两进程PV操作-同步进程](https://github.com/fjnucym/LinuxSystemCallDemo/blob/master/project/IPC_Semaphore.cpp)

## 信号量创建和控制

```c
int semget(key_t key, int num_sems, int sem_flag);
```

- key：是一个整数值，不相关的进程将通过这个值去访问同一个信号量
- num_sems：需要使用的信号量个数，它几乎总是取值为1
- sem_flags:是一组标志，其作用与open函数的各种标志很相似，它低端的九个位是该信号量的权限，其作用相当于文件的访问权限，可以与键值IPC_CREATE做按位的OR操作以创建一个新的信号量(IPC_CREAT|0766)
- 返回值：成功时将返回一个正数值，该数为信号量的标识码（信号量id，不是我们给入的key），如果失败，将返回-1

```c
// 信号量控制函数，使用第四个参数来设置信号量的值
// 可以接收3~4个参数
int semctl(int semid, int semnum, int cmd, ...);
// 第4个参数若有，则必须是如下的一个union，这个val是比较有用的，其他的用处不大
union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           };
```

- semid：信号量组的唯一标识符，由semget函数返回。

- semnum：信号量组内信号量的下标

- cmd：宏定义命令，有12种取值，对应函数不同的行为

  | SETVAL                                     | GETVAL             |
  | ------------------------------------------ | ------------------ |
  | 设置绑定的信号量值，需要semctl的第四个参数 | 获取对应的信号量值 |

## 信号量PV操作

```c
// 对信号量的操作，PV只需要一个函数。
int semop(int semid, struct sembuf *sops, size_t nsops);
// 参数2涉及到的结构体
struct sembuf
{
    unsigned short int sem_num;	/* 信号量编号 */
	short int sem_op;			/* 信号组内信号量的下标 */
	short int sem_flg;			/* 操作符标志 */
};
```

semop函数对指定的信号量组中的某个下标操作。

- semid：信号量组编号
- sops：特殊的操作数组
  - sem_num：信号量编号
  - sem_op：信号组内信号量的下标
  - sem_flg：操作符标志
- nsops：有关该函数的行为标志

**封装P、V操作**

```c
// P操作 -1
int sem_v(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, 1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop v error");
	}
	return res;
}
// V操作 +1
int sem_v(int sem_id, int sem_num) {
	struct sembuf buf { sem_num, 1, 0 };
	int res = semop(sem_id, &buf, 1);
	if (res < 0) {
		perror("semop v error");
	}
	return res;
}
```

