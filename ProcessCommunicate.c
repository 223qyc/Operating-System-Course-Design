#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<wait.h>

#define MaxSize 256
#define SVKEY 75   //关键字,这里消息缓冲队列和共享存储区使用了同一个关键字，但两者没有任何关系
#define RequestType 4321    //消息缓冲队列中的请求信息类型
#define ResponseType 1234    //消息缓冲队列中的回复请求类型

struct msgbuf {
	long mtype;
	char mtext[MaxSize];
}msg;   //定义消息结构体


//菜单
void PrintMenu() {
	printf("-----------------------欢迎使用进程通信功能-----------------------------\n");
	printf("------------1.使用管道来实现父子进程之间的进程通信----------------------\n");
	printf("------------2.使用消息缓冲队列来实现client进程和server进程之间的通信----\n");
	printf("------------3.使用共享存储区来实现两个进程之间的进程通信----------------\n");
	printf("------------------------0.退出程序--------------------------------------\n");
	printf("请选择你的通信方式:");
}

//管道通信
void Pipe(){
	int Pipefd[2];
	pid_t Parent= getpid();
	printf("父进程的PID为：%d\n",Parent);
	if (pipe(Pipefd) < 0) {
		printf("Pipe创建失败");
		return;
	}
	pid_t Child;
	Child = fork();
	if (Child == 0) {
		char Information[50];
		close(Pipefd[0]);   //写，因此关闭读端
		sprintf(Information, "Child PID:%d is sending a meaasge to parent", getpid());
		write(Pipefd[1], Information, 50);
		sleep(1);
		exit(0);
	}
	else {
		close(Pipefd[1]);   //读，关闭写端
		char Buffer[50];
		read(Pipefd[0], Buffer, 50);
		printf("%s\n", Buffer);
		close(Pipefd[0]);
		close(Pipefd[1]);
		waitpid(Child, NULL, 0);
		printf("Child Process is over!\n");
	}
}


void Client() {
	int QID = msgget(SVKEY, 0777 | IPC_CREAT); // 获取或创建消息队列，第一个参数是关键字，第二个参数是权限掩码以及打开方式
	if (QID == -1) {
		perror("客户端：获取消息或创建消息队列失败");
		exit(EXIT_FAILURE);
	}
	pid_t Cpid = getpid();
	struct msgbuf msg;
	int* Cint = (int*)msg.mtext;   //将进程号（原为字符串类型），转化为整形类型
	*Cint = Cpid;
	msg.mtype = RequestType;    //定义请求类型
	if (msgsnd(QID, &msg, sizeof(int), 0) == -1) {    //第一个参数是标识符，第二个是接收消息结构体的指针，第三个是长度，第四个是消息类型，0则表示都可以接收
		perror("客户端：消息发送失败");
		exit(EXIT_FAILURE);
	}
	//第一个参数是标识符，第二个接收消息的结构的指针,第三个长度，第四个消息类型
	msgrcv(QID, &msg, sizeof(int), ResponseType, 0); // 接收服务端回应
	// 检查接收到的消息类型是否为RESPONSE_TYPE
	if (msg.mtype == ResponseType) {
		printf("客户端PID:%d 接收到了服务端的回应 服务端PID:%d\n", Cpid, *(int*)msg.mtext);
	}
	else {
		fprintf(stderr, "客户端:未接收到预期回应类型\n");
		exit(EXIT_FAILURE);
	}
	exit(0);
}

void Server() {
	int QID = msgget(SVKEY, 0777 | IPC_CREAT);
	if (QID == -1) {
		perror("服务端：获取消息或创建消息队列失败");
		exit(EXIT_FAILURE);
	}
	struct msgbuf msg;
	if (msgrcv(QID, &msg, MaxSize, RequestType, 0) == -1) { // 接收类型为1的消息
		perror("服务端：消息接收失败");
		exit(EXIT_FAILURE);
	}
	pid_t Spid = *(int*)msg.mtext;
	printf("服务端PID:%d 为客户端提供了服务 客户端PID:%d\n", getpid(), Spid);

	// 发送回应给客户端
	msg.mtype = ResponseType; // 使用相同的消息类型
	*(int*)msg.mtext = getpid();
	if (msgsnd(QID, &msg, sizeof(int), 0) == -1) {
		perror("服务端：消息发送失败");
		exit(EXIT_FAILURE);
	}

	exit(0);
}

void Bufferqueues() {
	pid_t pid1, pid2;
	do {
		pid1 = fork();
	} while (pid1 == -1); // 处理 fork 失败的情况

	if (pid1 == 0) {
		Server(); // 子进程1执行服务端逻辑
		exit(0); // 子进程执行完毕后退出
	}

	do {
		pid2 = fork();
	} while (pid2 == -1); // 处理 fork 失败的情况

	if (pid2 == 0) {
		Client(); // 子进程2执行客户端逻辑
		exit(0); // 子进程执行完毕后退出
	}
	wait(NULL); // 等待子进程1结束
	wait(NULL); // 等待子进程2结束
}
 
void ProcessA() {
	int SID = shmget(SVKEY, 512, 0777 | IPC_CREAT);   //第一个参数是关键字，第二个参数是大小，第三个参数是权限掩码以及打开方式
	char* addr = shmat(SID, 0, 0);  //将共享内存段附加到调用进程的地址空间  第一个参数标识符，第二个是附加的地址，第三个是附着的操作的标志
	printf("共享存储区内容:%s\n", addr);
	if (shmdt(addr) == 0) {
		printf("进程A脱离了共享存储区，与共享内存区断开了连接\n");
	}
	exit(0);
}

void ProcessB() {
	int SID = shmget(SVKEY, 512, 0777 | IPC_CREAT);
	char* addr = shmat(SID, 0, 0);
	char* messages[] = { "进程B写入了信息"};
	memset(addr, '\0', 512);//addr内容初始化
	strncpy(addr, messages[0], 512);   //将一个字符串复制到另一个字符串中
	if (shmdt(addr) == 0) {
		printf("进程B脱离了共享存储区，与共享内存区断开了连接\n");
	}
	exit(0);

}

void Sharedmemory() {
	pid_t pid1, pid2;
	do {
		pid1 = fork();
	} while (pid1 == -1); // 处理 fork 失败的情况

	if (pid1 == 0) {
		ProcessB(); 
		exit(0); 
	}

	do {
		pid2 = fork();
	} while (pid2 == -1); 

	if (pid2 == 0) {
		ProcessA(); 
		exit(0); 
	}
	wait(NULL); 
	wait(NULL); 
}

int main() {
	while (1) {
		PrintMenu();
		int choice = 0;
		scanf("%d", &choice);
		if (choice == 1) {
			Pipe();
		}
		else if (choice == 2) {
			Bufferqueues();
		}
		else if (choice == 3) {
			Sharedmemory();
		}
		else if (choice == 0) {
			break;
		}
		else {
			printf("选择错误，请重新输入\n");
		}
	}
	return 0;
}