#include<iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#pragma comment(lib, "ws2_32.lib")
DWORD WINAPI CreateClientThread(LPVOID lpParameter);
bool a1 = true;
Mat as;

int main() {
	
	HANDLE  h_thread = ::CreateThread(nullptr, 0, CreateClientThread, (LPVOID)"192.168.2.140", 0, nullptr);

	//--------------------------------------------------------------------------------------------------
	
	//InternetCloseHandle(internet);
	while (true)
	{
		if (!a1) {
						imshow("kkk", as);a1 = true;
			waitKey(10);
		}
	}
	return 0;
}
DWORD WINAPI CreateClientThread(LPVOID lpParameter) 
{
	const char SOI_MARKER[] = { (byte)0xFF, (byte)0xD8 };
	//加载套接字库
	WSADATA wsaData;
	int iRet = 0;
	const 	char * a = "GET /stream HTTP/1.1\r\nConnection:Close\r\n\r\n";
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0)
	{
		cout << "WSAStartup(MAKEWORD(2, 2), &wsaData) execute failed!" << endl;
		return -1;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		cout << "WSADATA version is not correct!" << endl;
		return -1;
	}
	//创建套接字
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "clientSocket = socket(AF_INET, SOCK_STREAM, 0) execute failed!" << endl;
		return -1;
	}
	//初始化服务器端地址族变量
	SOCKADDR_IN srvAddr;
	//srvAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, (char*)lpParameter, &srvAddr.sin_addr);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(81);
	//连接服务器
	iRet = connect(clientSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (0 != iRet)
	{
		cout << "connect(clientSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR)) execute failed!" << endl;
		return -1;
	}

	int x = 0;

	while (true)
	{
		x++;
		//发送消息
		char sendBuf[100];
		sprintf_s(sendBuf, "Hello, this is client %s %d！", "bunny", x);
		printf(sendBuf);
		printf("\n");
		send(clientSocket, a, strlen(a), 0);
		while (true)
		{
			//接收消息
			char r, recvBuf[10000]; int jj=0, sig, j = 0; bool t = true;
			for (int i = 0; t; i++) {
				sig = recv(clientSocket, &r, 1, 0); 			//printf("Error: Lost connection!\n");

				if (sig <= 0)
				{
					printf("Error: Lost connection!\n");
					break;
				}
				recvBuf[i - 1] = r;
				//	printf("%c",r);
				if (r == SOI_MARKER[j])
				{
					j++;
					if (j == 2) {
						j = 0;		recvBuf[i] = '\0';
						char*b = strstr(recvBuf, "Content-Length");
						b = strstr(b, ":");
						jj = atoi(b + 1);
						cout << "jpeg数据流长" << jj << endl;
						t = false;
					};
				}
				else if(r != SOI_MARKER[0]&&j==1)j=0;//else j = 0;
			}
			jj -= 2; int js = 2;sig = 0;
			uchar *cs = new uchar[jj]; vector<uchar> jpg_buff; 
			cs[0] = SOI_MARKER[0];cs[1] = SOI_MARKER[1];
			while (jj > 0)
			{
				sig = recv(clientSocket, (char*)cs+js, jj, 0); 			//printf("Error: Lost connection!\n");
				if (sig <= 0)
				{
					printf("Error: Lost connection!\n");
					break;
				}
				jj -= sig; js += sig;
			}
			/*clock_t start, end;
			start = clock();
			vector<uchar> dat(cs, cs + js);
			end = clock();*/
			_InputArray pic_arr(cs, js); 
			if (a1) {
				as = imdecode(pic_arr, IMREAD_UNCHANGED);//图像解码
				a1 = false;
			}
			//vector<char> vec_data(&cs[0], &cs[0] + js);
			//printf(&r);
			printf("%s \n", recvBuf);
		}
			//跳出
			if (x == 5)
			{
				break;
			}

		
	}

	//清理
	closesocket(clientSocket);
	WSACleanup();

	system("pause");
	return 0;
}
