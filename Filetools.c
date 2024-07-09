#include<stdio.h>
#include<string.h>
#include<unistd.h> //UNIX系统标准头文件之一，定义了系统调用接口
#include<fcntl.h>   //标准头文件之一
#include<sys/stat.h>    //c语言预处理指令，包含系统头文件，提供了交互所需的定义与函数原型
#include<sys/types.h>


void PrintMenu() {
	printf("------------欢迎使用filetools----------------\n");
	printf("--------------1.创建新文件-------------------\n");
	printf("--------------2.写文件-----------------------\n");
	printf("--------------3.读文件-----------------------\n");
	printf("--------------4.修改文件权限-----------------\n");
	printf("--------------5.查看文件权限-------------\n");
	printf("--------------0.退出工具---------------------\n");
	printf("\n");
	printf("请选择功能：");
}

//修改权限时的声明，封装成一个函数
void PermissionMenu() {
	printf("声明修改权限的规则:\n");
	printf("读的权值为4  写的权值为2  执行的权值为1\n");
	printf("请输入一个三位数，最高位是用户权限，中间位是组权限，最低位是其他用户权限（每一位都是三种操作之和，例如6=4+2，表示拥有读写权限）\n");
	printf("请输入：");
}
int Fd;
char* Path;

void CreateNewFile() {
	char filename[100];
	printf("请输入要创建的文件名:");
	scanf("%s", filename);
	Path = filename;
	Fd = open(Path, O_RDWR | O_TRUNC | O_CREAT, 0755);//第一个表示位置，第二个表示打开方式，第三个是权限的掩码
	/*
	O_RDONLY 只读方式  O_WRONLY 只写方式  O_RDWR读写方式
	O_CREAT 如果文件不存在则创建该文件，若存在则忽略。
	O_TRUNC 如果文件存在则将文件长度截为 0，属性和所有者不变。
	C_EXECL 如果文件存在且 O_CREAT 被设置则强制 open 调用失败。
	O_APPEND 每次写入时都从文件尾部开始。

	*/
	if (Fd < 0) {      //Fd小于0是创建失败的
		perror("创建新文件失败");
	}
	else {
		printf("文件%s创建成功", filename);
		printf("文件标识符为：%d\n", Fd);
		close(Fd);
	}
}

void WriteFile() {
	char filename[100];
	printf("请输入要写的文件名:");
	scanf("%s", filename);
	Path = filename;
	char Content[100000];
	Fd = open(Path, O_WRONLY | O_APPEND);
	if (Fd < 0) {
		perror("写文件失败：");
		return;
	}
	printf("请写入信息:\n");
	int size = read(0, Content, 100000);  //第一个参数0表示从键盘输入，第二个为一个字符数组，第三个指定尺寸
	write(Fd, Content, size);   //写入，文件名，内容，大小
	printf("内容已成功写入文件\n");
	close(Fd);
}

void ReadFIle() {
	char filename[100];
	printf("请输入要读的文件名:");
	scanf("%s", filename);
	Path = filename;
	Fd = open(Path, O_RDONLY);
	if (Fd < 0) {
		perror("读文件失败:");
	}
	char Content[100000];
	int size = read(Fd, Content, 100000);   //改为从文件读入，其余和上个函数中该语句的参数一致
	write(1, Content, size);     //表示从终端输出
	printf("\n读文件执行完毕\n");
	close(Fd);
}

void ChangePermission() {
	char filename[100];
	printf("请输入要修改权限的文件名:");
	scanf("%s", filename);
	Path = filename;

	Fd = open(Path, O_RDONLY);
	if (Fd < 0) {
		perror("修改权限失败:");
		return;
	}
	PermissionMenu();
	int number;
	scanf("%d", &number);
	int user = number / 100;
	int group = (number / 10) % 10;
	int other = number % 10;
	int mode = user * 8 * 8 + group * 8 + other;       //修改权限依照8进制实现，依照前面的0777
	int IfSuccess = chmod(Path, mode);
	if (IfSuccess == -1) {
		printf("权限修改失败\n");
	}
	else {
		printf("文件%s权限修改成功\n", Path);
	}
	close(Fd);
}

void CheckPermission() {
	char filename[100];
	printf("请输入要查看权限的文件名:");
	scanf("%s", filename);
	Path = filename;
	char* pargv[4] = { "ls","-l",Path,NULL };   //先定义一大小为4的字符数组，作为下函数的参数
	execv("/bin/ls", pargv);   //系统调用，用于执行一个外部程序
}

int main() {
	while (1) {
		PrintMenu();
		int choice = 0;
		scanf("%d", &choice);
		if (choice == 1) {
			CreateNewFile();
		}
		else if (choice == 2) {
			WriteFile();
		}
		else if (choice == 3) {
			ReadFIle();
		}
		else if (choice == 4) {
			ChangePermission();
		}
		else if (choice == 5) {
			CheckPermission();
		}
		else if (choice == 0) {
			return 0;
		}
		else {
			printf("选择错误，请重新输入选择");
		}
	}

	return 0;
}