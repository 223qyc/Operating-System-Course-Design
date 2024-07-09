#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<cstdlib>
#include <algorithm>
#include <climits>
#include<utility>
using namespace std;


struct Request {    //定义磁盘调度请求的结构体
	string name;
	int number;
	//重载=         方便后续操作直接通过=进行赋值
	Request& operator=(const Request& r) {
		this->name = r.name;
		this->number = r.number;
		return *this;
	}
};


//从文件读入信息并存入数组
vector<Request> GetInformation(string filename) {
	vector<Request>v;
	ifstream file(filename);
	if (file.is_open()) {
		for (int i = 0; i < 14; i++) {
			Request r;
			file >> r.name >> r.number;
			v.push_back(r);
		}
	}
	return v;
}

//打印完成队列
void PrintInformation(vector<Request>v) {
	cout << "请求名称" << "\t\t" << "磁道号" << endl;
	for (vector<Request>::iterator it = v.begin(); it != v.end(); it++) {
		cout << it->name << "\t\t\t" << it->number << endl;
	}
}

//打印菜单
void menu() {
	cout << "-----------欢迎使用磁盘调度算法---------------" << endl;
	cout << "-------------1.先来先服务算法-----------------" << endl;
	cout << "-------------2.最短寻道优先算法---------------" << endl;
	cout << "-------------3.电梯调度算法-------------------" << endl;
	cout << "-------------0.退出程序-----------------------" << endl;
	cout << "请输入您的选择" << endl;
}

//先来先服务
vector<Request> FCFS(vector<Request>Requests,double&totalmove) {
	vector<Request>finish;     //存放完成请求
	int current_position = 90;   //初始磁道位置
	while (!Requests.empty()) {              //实现这个十分简单，即按顺序扫描序列即可
		if (current_position >= Requests[0].number) {
			totalmove += current_position - Requests[0].number;
			current_position = Requests[0].number;
		}
		else {
			totalmove+= Requests[0].number - current_position;
			current_position = Requests[0].number;
		}
		Request temp = Requests[0];
		Requests.erase(Requests.begin());
		finish.push_back(temp);
	}
	return finish;
}

//辅助实现最短寻道的函数，参数是当前剩余请求和当前位置，返回当前容器下最短寻道的索引
int FindBest(vector<Request>v, int position) {
	int index,i=0;
	int temp = 999;
	int n = v.size();
	for ( ; i < n; i++) {
		int delta = abs(v[i].number - position);
		if (delta < temp) {   //这个<就可以做到即使在相等情况下的FCFS,即使相等也被直接掠过
			temp = delta;
			index = i;
		}
	}
	return index;
}


//最短寻道优先
vector<Request>Shortest(vector<Request>Requests, double& totalmove) {
	vector<Request>finish;
	int index = 0;
	int current_position = 90;
	while (!Requests.empty()) {
		index = FindBest(Requests, current_position);
		totalmove += abs(Requests[index].number - current_position);
		current_position = Requests[index].number;
		Request r = Requests[index];
		Requests.erase(Requests.begin() + index);//通过迭代器+索引的方式将已处理项目删掉
		finish.push_back(r);
	};
	return finish;
}


//辅助电梯算法实现的函数，将请求根据小于当前磁道号和大于当前磁道号分为两个请求队列
pair<vector<Request>,vector<Request>>SortforSCAN(vector<Request>Requests, int position) {
	vector<Request>R1;   //R1存放位置小于当前位置的
	vector<Request>R2;    //R2存放位置大于当前位置的
	for (vector<Request>::iterator it = Requests.begin(); it != Requests.end(); it++) {
		if (it->number < position) {
			Request temp = *it;
			R1.push_back(temp);
		}
		else {
			Request temp = *it;
			R2.push_back(temp);
		}
	}
	return make_pair(R1, R2);
}

//辅助电梯算法实现的函数，将传入的vector从小到大排列
vector<Request> Sortnumber(vector<Request> v) {
	vector<Request> Sorted;
	int n = v.size();
	while (n > 0) {
		int min_index = 0;
		int min_value = INT_MAX; 
		for (int i = 0; i < n; ++i) { 
			if (v[i].number < min_value) {
				min_value = v[i].number;
				min_index = i;
			}
		}
		Sorted.push_back(v[min_index]);
		v.erase(v.begin() + min_index);
		n--; 
	}
	return Sorted;
}

vector<Request>SCAN(vector<Request>Requests, double& totalmove) {
	vector<Request>finish;
	int current_position = 90;
	pair <vector<Request>, vector<Request>>R1_R2 = SortforSCAN(Requests, current_position);  //R1是小于的   R2是大于的
	vector<Request>R1 = R1_R2.first;
	vector<Request>R2 = R1_R2.second;
	vector<Request>R1_sort = Sortnumber(R1);
	vector<Request>R2_sort = Sortnumber(R2);

	for (vector<Request>::iterator it = R2_sort.begin(); it != R2_sort.end(); it++) {     //先处理右侧队列，队列大小已从小到达排列
		totalmove += it->number - current_position;
		current_position = it->number;
		Request temp = *it;
		finish.push_back(temp);
	}
	reverse(R1_sort.begin(), R1_sort.end());     //先将左侧队列反转，改为从大到小排列
	for (vector<Request>::iterator it = R1_sort.begin(); it != R1_sort.end(); it++) {
		totalmove += current_position - it->number;
		current_position = it->number;
		Request temp = *it;
		finish.push_back(temp);
	}
	return finish;
}


int main() {
	string filename = "C:\\Users\\86158\\Desktop\\Request.txt";
	vector<Request>Requests = GetInformation(filename);
	int choice = 0;
	while (1) {
		menu();
		cin >> choice;
		if (choice == 1) {
			double totalmove = 0;
			vector<Request>finish = FCFS(Requests, totalmove);
			PrintInformation(finish);
			cout << "先来先服务算法--磁头总移动数:" << totalmove << "\t\t" << "平均移动数：" << totalmove / 14 << endl;
		}
		else if (choice == 2) {
			double totalmove = 0;
			vector<Request>finish1 = Shortest(Requests, totalmove);
			PrintInformation(finish1);
			cout << "最短寻道优先算法--磁头总移动数:" << totalmove <<"\t\t" << "平均移动数：" << totalmove / 14 << endl;
		}
		else if (choice == 3) {
			double totalmove = 0;
			vector<Request>finish2 = SCAN(Requests, totalmove);
			PrintInformation(finish2);
			cout << "电梯调度算法--磁头总移动数:" << totalmove <<"\t\t" << "平均移动数：" << totalmove / 14 << endl;
		}
		else if (choice == 0) {
			break;
		}
		else {
			cout << "输入错误，请重新输入" << endl;
		}
	}
	return 0;
}