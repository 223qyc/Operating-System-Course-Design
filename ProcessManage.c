#include <stdio.h>      // 包含标准输入输出库
#include <stdlib.h>     // 包含标准库，提供一些通用函数如exit
#include <unistd.h>     // 包含Unix标准函数库，提供read, write, sleep等
#include <string.h>     // 包含字符串操作函数库
#include <signal.h>     // 包含信号处理库
#include <sys/types.h>  // 包含系统类型定义
#include <sys/wait.h>   // 包含等待子进程状态改变的函数库

#define MAX 50
pid_t P1;
pid_t P2;
int PipeFd[2];

void Handle1(int s) {
	printf("\nParent Process 接收到了信号：Ctrl+C\n");
	if (s == SIGINT) {
		kill(P1, SIGUSR1);     //向P1传送SIGUSR1,
		kill(P2, SIGUSR2);
	}
}

void Handle2(int s) {
	close(PipeFd[0]);
	close(PipeFd[1]);
	if (P1==0 && s == SIGUSR1) {
		printf("Child Process 1 is killed by Parent!\n"); 
		exit(0); 
	}
	if ( P2==0 && s == SIGUSR2) {
		printf("Child Process 2 is killed by Parent!\n"); 
		exit(0); 
	}
}

int main() {
	int number = 1;
	pid_t getpid();     //创建父进
	char Buffer1[MAX];
	char Buffer2[MAX];
	printf("Parent Process's PID is:%d\n", getpid());
	//pipe创建管道，成功返回0，失败返回-1
	if (pipe(PipeFd) < 0) {
		printf("未能成功创建管道\n");
		return -1;
	}
	signal(SIGINT, Handle1);    //signal()函数，第一个参数表示处理的信号，第二个参数是函数指针，指向处理函数

	P1 = fork();//创建子进程
	if (P1 == -1) {
		perror("子进程1创建失败；");
		exit(EXIT_FAILURE);
	}
	if (P1 == 0) {
		printf("Child1 Process's PID is:%d\n", getpid());
		signal(SIGINT, SIG_IGN);    //忽略ctrl+C
		signal(SIGUSR1, Handle2);     
		while (1) {
			close(PipeFd[0]);      //0表示读管道，1表示写管道 
			sprintf(Buffer1, "I send message %d times", number); //第一个参数是目标字符串，第二个参数是内容
			write(PipeFd[1], Buffer1, MAX);   //P1是负责写的，所以关闭读管道
			number++; //
			sleep(1);
		}
	}
	else if (P1 > 0) {
		P2 = fork();
		if (P2 == -1) {
			perror("子进程2创建失败；");
			exit(EXIT_FAILURE);
		}
		if (P2 == 0) {
			printf("Child2 Process's PID is:%d\n", getpid());
			signal(SIGINT, SIG_IGN);
			signal(SIGUSR2, Handle2);
			while (1) {
				close(PipeFd[1]);    //P2是要读的，所以关闭写管道，读管道打开
				read(PipeFd[0], Buffer2, MAX);
				printf("%s\n", Buffer2);
			}
		}
		else {
			waitpid(P1, NULL, 0);    //避免僵尸进程,要将其收回
			waitpid(P2, NULL, 0);
			close(PipeFd[0]);
			close(PipeFd[1]);
			printf("Parent Process is Killed!\n");
		}
	}
	return 0;
}