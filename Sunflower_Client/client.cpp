#include "SunTCP.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;
SunTCPServer client;
SunTCPServer local;
short server_port = 9998;
short turner_port = server_port + 1;
short local_port = 1234;
short gThread = 1;
vector<short> gServerSocks;
vector<short> gLocalSocks;

void client_accept(PINFO info) {

}
void client_recv(PINFO info) {
	auto data = DecodeLenAStr(client.GetData(info), 16);
	for (auto i : data)
	{
		if (i == "CON") {
			short mserver_port = local.Connect("127.0.0.1", turner_port, gThread);
			gServerSocks.push_back(mserver_port);
			if (mserver_port > 0) {
				cout << "Local:Connect Server Successfully" << endl;
			}
			else {
				perror("Local:Connect Server");
				exit(0);
			}
			short mlocal_port = local.Connect("127.0.0.1", local_port, 0);
			gLocalSocks.push_back(mlocal_port);
			if (mlocal_port > 0) {
				cout << "Local:Connect Local Successfully" << endl;
			}
			else {
				perror("Local:Connect Local");
				exit(0);
			}
			gThread = 0;
		}
		else {
			cout << "Unexpected! Server send to flower in a uncommon way." << endl;
		}
	}
}
void client_left(PINFO info) {
	client.Close(info);
	exit(0);
}
void local_accept(PINFO info) {

}
void local_recv(PINFO info) {
	bool isError = false;
	while (true)
	{
		if (gServerSocks.size() == gLocalSocks.size()) {
			for (size_t i = 0; i < gServerSocks.size(); i++)
			{
				if (info->mSock == gServerSocks[i]) {
					auto data = DecodeLenAStr(local.GetData(info), 16);
					for (auto k : data)
					{
						Log_DEBUG("log.log", ("Local-Turner:" + k).c_str());
						local.Send(gLocalSocks[i], k.c_str());
					}
					break;
				}
				else if (info->mSock == gLocalSocks[i]) {
					Log_DEBUG("log.log", ("Local-Turner:" + EncodeLenAStr(local.GetData(info), 16)).c_str());
					local.Send(gServerSocks[i], EncodeLenAStr(local.GetData(info), 16).c_str());
					break;
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 81,Repaired" << endl;
			}
			break;
		}
		else {
			isError = true;
			cout << "WARN:Line 81,Client!=Turner" << endl;
		}
	}
}
void local_left(PINFO info) {
	bool isError = false;
	while (true)
	{
		if (gServerSocks.size() == gLocalSocks.size()) {
			for (size_t i = 0; i < gServerSocks.size(); i++)
			{
				if (gServerSocks[i] == info->mSock) {
					local.Close(info);
					gServerSocks[i] = -1;
					local.Close(gLocalSocks[i]);
					gLocalSocks[i] = -1;
					break;
				}
				else if (gLocalSocks[i] == info->mSock) {
					local.Close(info);
					gLocalSocks[i] = -1;
					local.Close(gServerSocks[i]);
					gServerSocks[i] = -1;
					break;
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 113,Repaired" << endl;
			}
			break;
		}
		else {
			isError = true;
			cout << "WARN:Line 113,Client!=Turner" << endl;
		}

	}
	cout << "A Local Client left" << endl;
}
int main() {
	client.Match(client_accept, client_recv, client_left);
	local.Match(local_accept, local_recv, local_left);
	if (client.Connect("127.0.0.1", turner_port, 1) > 0) {
		cout << "connected server." << endl;
	}
	else {
		perror("Connect");
		exit(0);
	}
	while (true);
	return 0;
}