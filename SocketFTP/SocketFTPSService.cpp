
#include<iostream>
#include<winsock.h>
#include<string.h>
#include<sstream>
#include<fstream>
#include<filesystem>
#include<stdio.h>
#include<direct.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
using namespace std::filesystem;
void initialization();
path curPath(".\\");
//定义服务端套接字，接受请求套接字
SOCKET s_accept;
SOCKET s_server;
//定义发送缓冲区和接受缓冲区
char send_buf[1024];
char recv_buf[1024];
//void sendFile(path filename) {
//	ifstream infile;
//	string data;
//	infile.open(filename);
//	infile.seekg(0, ios::end);
//	stringstream tmp;
//	tmp << infile.tellg();
//	char fileSize[10];
//	int filesize = infile.tellg();
//	tmp >> fileSize;
//	send(s_accept, fileSize, strlen(fileSize), 0);
//	
//	infile.seekg(0, ios::beg);
//
//	for (int i = 0; i < filesize;i = i + 1024) {
//		infile.seekg(i, ios::beg);
//		infile.read(recv_buf, 1024);
//		send(s_accept, recv_buf, strlen(recv_buf), 0);
//		cout << recv_buf;
//	}
//	infile.close();
//}
void sendFile(path filename) {
	send(s_accept, "file", 4, 0);
	recv(s_accept, recv_buf, 1024, 0);//粘包
	FILE* fp;
	string tmp = filename.u8string();
	stringstream stmp;
	char* file = tmp.data();
	fopen_s(&fp, file, "rb");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	stmp << size;
	stmp >> send_buf;
	send(s_accept, send_buf, strlen(send_buf), 0);
	recv(s_accept, recv_buf, 1024, 0);//粘包
	send(s_accept, file, strlen(file), 0);
	recv(s_accept, recv_buf, 1024, 0);//粘包
	fseek(fp, 0, 0);
	size += 1;
	for (int i = 0; i < size;) {
		int len = 1000;
		i += 1000;
		if (i > size) {
			len = size + 1000 - i;
		}
		fgets(send_buf, len, fp);
		send(s_accept, send_buf, len, 0);
	}
	recv(s_accept, recv_buf, 1024, 0);//粘包
}
void sendDir(path filepath) {
	send(s_accept, "dir", 3, 0);
	recv(s_accept, recv_buf, 1024, 0);//粘包
	string tmp = filepath.u8string();
	char* sendbuf = tmp.data();
	send(s_accept, sendbuf, strlen(sendbuf), 0);
	recv(s_accept, recv_buf, 1024, 0);//粘包
	directory_iterator list(filepath);
	for (auto& it : list) {
		if (it.is_regular_file()) {
			sendFile(it);
		}
		else sendDir(it);
	}
	
}
void get(string cmd) {
	stringstream istr(cmd);
	string file;
	while (istr >> file);
	path filepath = curPath;
	filepath.append(file);
	if (exists(filepath)) {
		if (is_regular_file(filepath)) {//判断是否为文件
			cout << "这是文件" << endl;
			sendFile(filepath);
		}
		else {
			cout << "这是目录" << endl;
			sendDir(filepath);
		}
		send(s_accept, "Finished", 8, 0);
	}
}
int ls() {
	vector<string> filename;
	string temp;
	if (!exists(curPath)) {
		cout << "路径不存在" << endl;
		const char* sendbuf = "路径不存在";
		send(s_accept, sendbuf, strlen(sendbuf), 0);
		return 0;
	}
	directory_iterator list(curPath);
	for (auto& it : list) {
		string  tmp(it.path().filename().u8string());
		filename.push_back(tmp);
	}
	for (int i = 0; i < filename.size(); i++) {
		temp += filename[i] + ' ';
	}
	char* sendbuf = temp.data();
	send(s_accept, sendbuf, strlen(sendbuf), 0);
	return 1;
}
void cd(string cmd) {
	stringstream istr(cmd);
	string path;
	while (istr >> path);
	if (path == "..") {
		curPath = curPath.parent_path();
		//if (ls() == 0) {
		//	curPath.remove_filename();
		//	cout << curPath;
		//}
		//else cout << curPath;
		//_getcwd(path_buff, 1024);
		//string curPath = path_buff;
		string tmp(curPath.u8string());
		char* sendbuf = tmp.data();
		send(s_accept, sendbuf, strlen(sendbuf), 0);
	}
	else if (cmd.size() > 2) {
		curPath.append(path);
		if (ls() == 0) {
			curPath.remove_filename();
			cout << curPath;
		}
		else cout << curPath;
		string tmp(curPath.u8string());
		char* sendbuf = tmp.data();
		send(s_accept, sendbuf, strlen(sendbuf), 0);
	}
}

int main() {
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;


	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(5010);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "套接字绑定失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字绑定成功！" << endl;
	}
	//设置套接字为监听状态
	if (listen(s_server, SOMAXCONN) < 0) {
		cout << "设置监听状态失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "设置监听状态成功！" << endl;
	}
	cout << "服务端正在监听连接，请稍候...." << endl;
	//接受连接请求
	len = sizeof(SOCKADDR);
	s_accept = accept(s_server, (SOCKADDR*)&accept_addr, &len);
	if (s_accept == SOCKET_ERROR) {
		cout << "连接失败！" << endl;
		WSACleanup();
		return 0;
	}
	cout << "连接建立，等待指令..." << endl;

	//获取当前路径
	char path_buff[1024];
	_getcwd(path_buff, 1024);
	string path_string = path_buff;
	curPath.assign(path_string);
	
	//接收数据
	while (1) {
		recv_len = recv(s_accept, recv_buf, 1024, 0);
		recv_buf[recv_len] = '\0';
		if (strcmp(recv_buf, "ls") == 0) ls();
		else if (recv_buf[0] == 'c' && recv_buf[1] == 'd') cd(recv_buf);
		else if (string(recv_buf).find("get") != string::npos) get(recv_buf);
	}
	//关闭套接字
	closesocket(s_server);
	closesocket(s_accept);
	//释放DLL资源
	WSACleanup();
	return 0;
}
void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}
