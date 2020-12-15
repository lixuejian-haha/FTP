#include<iostream>
#include<winsock.h>
#include<string.h>
#include<sstream>
#include<filesystem>
#include<stdio.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
using namespace std::filesystem;
void initialization();
//定义服务端套接字，接受请求套接字
SOCKET s_server;
path curPath(".\\");
//定义发送缓冲区和接受缓冲区
char recv_buf[1024];
void ls() {
	
	int recv_len = recv(s_server, recv_buf, 1024, 0);
	recv_buf[recv_len] = '\0';
	cout << recv_buf << endl;
}
void cd(string cmd) {
	char* send_buf = cmd.data();
	send(s_server, send_buf, strlen(send_buf), 0);
	ls();
	//int recv_len = recv(s_server, recv_buf, 1024, 0);
	//recv_buf[recv_len] = '\0';
	//curPath = path(recv_buf);
}
void getFile(string filepath) {
	memset(recv_buf, '\0', sizeof(recv_buf));
	recv(s_server, recv_buf, 1024, 0);
	send(s_server, "ok", 2, 0);
	memset(recv_buf, '\0', sizeof(recv_buf));
	int recv_len = recv(s_server, recv_buf, 1024, 0);
	int size = atoi(recv_buf);
	send(s_server, "ok", 2, 0);
	memset(recv_buf, '\0', sizeof(recv_buf));
	recv(s_server, recv_buf, 1024, 0);
	FILE* fp;
	fopen_s(&fp, filepath.c_str(), "wb");
	send(s_server, "ok", 2, 0);
	int curSize = 0;
	while (curSize < size) {
		memset(recv_buf, '\0', sizeof(recv_buf));
		int recSize = recv(s_server, recv_buf, 1024, 0);
		fwrite(recv_buf, sizeof(char), recSize, fp);
		curSize += recSize;
		send(s_server, "ok", 2, 0);
	}
	fclose(fp);
	send(s_server, "ok", 2, 0);
}
void getDir() {

}
void get(string cmd) {
	char* sendbuf = cmd.data();
	send(s_server, sendbuf, strlen(sendbuf), 0);
	stringstream istr(cmd);
	string file;
	while (istr >> file);
	path filepath = curPath;
	filepath.append(file);
	if (filepath.has_extension()) {
		getFile(filepath.u8string());
		memset(recv_buf, '\0', sizeof(recv_buf));
		recv(s_server, recv_buf, 1024, 0);
		cout << recv_buf << endl;
	}
	else getDir();
}
int main() {
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;


	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5010);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "服务器连接成功！" << endl;
	}
	//发送,接收数据
	while (1) {
		cout << "请输入命令:" << endl;
		string cmd;
		getline(cin, cmd);
		if (cmd == "ls") {
			char* send_buf = cmd.data();
			send(s_server, send_buf, strlen(send_buf), 0);
			ls();
		}
		else if (cmd.find("cd") != string::npos) cd(cmd);
		else if (cmd.find("get") != string::npos) get(cmd);
		else if (cmd.find("end") != string::npos) { send(s_server, "end", 3, 0); break; }
		else cout << "未识别的指令" << endl;
	}
	//关闭套接字
	closesocket(s_server);
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
