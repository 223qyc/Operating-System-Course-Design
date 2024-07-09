#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include <climits>

using namespace std;

struct Job {
	string name;                //作业名，到达，开始，结束，运行，周转，带权周转时间
	double arrive_time;         //还有响应比，但是只在第三个算法中会使用到
	double run_time;
	double start_time;
	double end_time;
	double turn_time;
	double weight_time;
	double rate;
	//重载=,方便在之后操作中直接用=进行赋值
	Job& operator=(const Job& job) {                
		this->name = job.name;
		this->arrive_time = job.arrive_time;
		this->end_time = job.end_time;
		this->run_time = job.run_time;
		this->start_time = job.start_time;
		this->turn_time = job.turn_time;
		this->weight_time = job.weight_time;
		this->rate = job.rate;
		return *this;
	}
};


//从文件中读入数据，读入前三个，剩下的全部初始化0 ,返回一个vector容器存放读取到的所有作业信息
vector<Job> GetJobs(const string& filename) {
	vector<Job>Jobs;
	ifstream file(filename);
	if (file.is_open()) {
		for (int i = 0; i < 10; i++) {
			Job job;
			file >> job.name >> job.arrive_time >> job.run_time;
			job.start_time = 0;
			job.end_time = 0;
			job.rate = 0;
			job.turn_time = 0;
			job.weight_time = 0;    //这里赋值的时候没有给响应比赋值，因为其局限性，选择在对应部分再赋值
			Jobs.push_back(job);
		}
	}
	else {
		cout << "没有找到文件：" << filename << endl;
	}
	return Jobs;
}

int CPU1_time;  //定义两个CPU时间，模拟系统中同时有两道程序可以运行
int CPU2_time;


//先来先服务
vector<Job>FCFS(vector<Job>Jobs) {
	vector<Job>Finish;       //存放已完成作业
	vector<Job>::iterator p;
	CPU1_time = 0, CPU2_time = 0;//开始前将两个CPU时间置0
	bool flag;
	int n = Jobs.size();
	for ( int i = 0; i < n; i++) {
		vector<Job>::iterator it = Jobs.begin();
		Job temp;
		temp.arrive_time = INT_MAX;
		flag = false;
		while (it != Jobs.end()) {                 //寻找当前已到达可以在CPU上运行，而且到达时间最早的作业
			if (it->run_time != 0 && it->arrive_time <= temp.arrive_time && (it->arrive_time <=CPU1_time || it->arrive_time <= CPU2_time)) {
				temp = *it;
				p = it;
				flag = true;
			}
			it++;
		}
		if (flag == false) {
			it = Jobs.begin();
			while (it != Jobs.end()) {           //找不到就是现在CPU暂时不能用，那只需要找运行到达时间最早的作业即可
				if (it->run_time != 0 && it->arrive_time < temp.arrive_time) {
					p = it;
					temp = *it;
				}
				it++;
			}
		}

		Jobs.erase(p);           //接下来模拟上机运行，修改对应参数
		if (CPU1_time <= CPU2_time) {               //这是CPU1空闲的情况
			if (i == 0) {
				CPU1_time = temp.arrive_time;   //0表示第一个作业直接上
			}
			if (flag == false) {
				CPU1_time = temp.arrive_time;    //全忙但是CPU1先空出来
			}
			
			temp.start_time = CPU1_time;
			CPU1_time += temp.run_time;
			temp.end_time = CPU1_time;            //修改
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
		else {
			if (i == 1 || flag == false) {
				CPU2_time = temp.arrive_time;
			}
			temp.start_time = CPU2_time;
			CPU2_time += temp.run_time;
			temp.end_time = CPU2_time;
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
	}
	return Finish;   //返回完成队列
}


//短作业优先
vector<Job>SJF(vector<Job>Jobs) {
	vector<Job>Finish;
	vector<Job>::iterator p;
	CPU1_time = 0, CPU2_time = 0;
	bool flag;
	int n = Jobs.size();
	for (int i = 0; i < n; i++) {
		vector<Job>::iterator it = Jobs.begin();
		Job temp;
		temp.arrive_time = INT_MAX;
		temp.run_time = INT_MAX;   //这里将到达时间和运行时间设为最大
		flag = false;
		while (it != Jobs.end()) {      
			if (CPU1_time <= CPU2_time && it->arrive_time <= CPU1_time) {  //满足可上CPU1且已经到达
				if (it->run_time != 0 && it->run_time <temp.run_time) {    //在满足的作业中选择运行时间最短的
					temp = *it;
					p = it;
					flag = true;
				}
			}
			if (CPU1_time > CPU2_time && it->arrive_time <= CPU2_time) {    //满足可上CPU2且已经到达
				if (it->run_time != 0 && it->run_time < temp.run_time) {    //在满足的作业中选择运行时间最短的
					temp = *it;
					p = it;
					flag = true;
				}
			}
			it++;
		}
		if (flag == false) {               
			it = Jobs.begin();
			while (it != Jobs.end()) {   //如果暂时没有空闲CPU，则直接选一个先到达的即可，这里不用考虑短作业优先，因为只会有一个先来，而先来必定先上
				if (it->run_time != 0 && it->arrive_time < temp.arrive_time) {
					temp = *it;
					p = it;
				}
				it++;
			}
		}
		Jobs.erase(p);
		if (CPU1_time <= CPU2_time) {          //满足可上CPU1
			if (i == 0 || flag == false) {           //第0道作业以及CPU全忙时的情况
				CPU1_time = temp.arrive_time;    
			} 
			temp.start_time = CPU1_time;
			CPU1_time += temp.run_time;
			temp.end_time = CPU1_time;
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
		else { 
			if (i == 1 || flag == false) {           //满足可以上CPU2的情况
				CPU2_time = temp.arrive_time;
			}
			temp.start_time = CPU2_time;
			CPU2_time += temp.run_time;
			temp.end_time = CPU2_time;
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
	}
	return Finish;
}


//高响应比优先
vector<Job>HRRN(vector<Job>Jobs) {
	vector<Job>Finish;
	vector<Job>::iterator p;
	CPU1_time = 0, CPU2_time = 0;
	bool flag;
	int n = Jobs.size();
	for (int i = 0; i < n; i++) {
		vector<Job>::iterator it = Jobs.begin();
		Job temp;
		temp.rate = INT_MIN;
		temp.arrive_time = INT_MAX;    //响应比设为最小，到达时间设为最大
		flag = false;
		while (it != Jobs.end()) {
			if (CPU1_time <= CPU2_time && it->arrive_time <= CPU1_time) {     //满足可以上CPU1的作业，
				it->rate = (CPU1_time-it->arrive_time + it->run_time) / it->run_time;    //计算响应比
				if (it->rate != 0 && it->rate > temp.rate) {
					temp = *it;
					p = it;          //不断寻找，找到满足条件的最高响应比
					flag = true;
				}
			}
			if (CPU1_time > CPU2_time && it->arrive_time <= CPU2_time) {          //同理，换一个CPU
				it->rate = (CPU2_time - it->arrive_time + it->run_time) / it->run_time;
				if (it->rate != 0 && it->rate > temp.rate) {
					temp = *it;
					p = it;
					flag = true;
				}
			}
			it++;
		}
		if (flag == false) {
			it = Jobs.begin();
			while (it != Jobs.end()) {              //均忙则找先来的即可，这时不用考虑响应比
				if (it->run_time != 0 && it->arrive_time < temp.arrive_time) {
					temp = *it;
					p = it;
				}
				it++;
			}
		}
		Jobs.erase(p);  
		if (CPU1_time <= CPU2_time) {              //均同理
			if (i == 0 || flag == false) {
				CPU1_time = temp.arrive_time;
			}
			temp.start_time = CPU1_time;
			CPU1_time += temp.run_time;
			temp.end_time = CPU1_time;
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
		else {
			if (i == 1 || flag == false) {
				CPU2_time = temp.arrive_time;
			}
			temp.start_time = CPU2_time;
			CPU2_time += temp.run_time;
			temp.end_time = CPU2_time;
			temp.turn_time = temp.end_time - temp.arrive_time;
			temp.weight_time = temp.turn_time / temp.run_time;
			Finish.push_back(temp);
		}
	}
	return Finish;
}


//计算平均周转，平均带权周转
void PrintAverage(vector<Job>v) {
	int n = v.size();
	double total = 0;
	double wtotal = 0;
	for (vector<Job>::iterator it = v.begin(); it != v.end(); it++) {
		total += it->turn_time;
		wtotal += it->weight_time;
	}
	cout << "平均周转时间为:" << total / n<<"     " << "平均带权周转时间为:" << wtotal / n << endl;
}

//输出信息，用于前三个功能
void PrintInforamtion(vector<Job>v) {
	cout << "name" << "\t" 
		<< "到达时间" << "\t" 
		<< "开始时间" << "\t" 
		<< "结束时间" << "\t" 
		<< "运行时间" << "\t" 
		<< "周转时间" << "\t" 
		<< "带权周转" << endl;
	for (vector<Job>::iterator it = v.begin(); it != v.end(); it++) {
		cout << it->name << "\t"
			<< it->arrive_time << "\t\t"
			<< it->start_time << "\t\t"
			<< it->end_time << "\t\t"
			<< it->run_time << "\t\t"
			<< it->turn_time << "\t\t"
			<< it->weight_time << endl;
	}
}

//打印菜单
void PrintMenu() {
	cout << "--------------欢迎使用作业调度程序------------------" << endl;
	cout << "---------------1.先来先服务算法---------------------" << endl;
	cout << "---------------2.短作业优先算法---------------------" << endl;
	cout << "---------------3.高响应比优先算法-------------------" << endl;
	cout << "---------------4.比较三种算法的性能优劣-------------" << endl;
	cout << "---------------0.退出程序---------------------------" << endl;
	cout << "请输入您要选择的功能:" << endl;

}


int main() {
	string filename = "C:\\Users\\86158\\Desktop\\Jobs.txt";
	vector<Job>Jobs = GetJobs(filename);
	int choice = 0;
	vector<Job> fcfsfinish = FCFS(Jobs);
	vector<Job>sjffinish = SJF(Jobs);
	vector<Job>hrrnfinish = HRRN(Jobs);
	while (1) {
		PrintMenu();
		cin >> choice;
		if (choice == 1) {
			PrintInforamtion(fcfsfinish);
		}
		else if (choice == 2) {
			PrintInforamtion(sjffinish);
		}
		else if (choice == 3) {
			PrintInforamtion(hrrnfinish);
		}
		else if (choice == 4) {
			cout << "先来先服务算法性能如下：" << endl;
			PrintAverage(fcfsfinish);
			cout << "短作业优先算法性能如下：" << endl;
			PrintAverage(sjffinish);
			cout << "高响应比优先算法性能如下:" << endl;
			PrintAverage(hrrnfinish);
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
