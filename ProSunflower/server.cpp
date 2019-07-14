#include "SunTCP.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;
SunTCPServer server;
SunTCPServer turner;
bool IsFlower = false;
PINFO flower = 0;
vector<PINFO> ClientCount;
vector<PINFO> TurnerCount;

void server_accept(PINFO info) {
	if (flower != 0) {
		server.Send(flower, EncodeLenAStr("CON", 16).c_str());
		ClientCount.push_back(info);
	}
	else {
		cout << "No Flower,everything will fail." << endl;
	}
}
void server_recv(PINFO info) {
	bool isError = false;
	while (true)
	{
		if (ClientCount.size() == TurnerCount.size()) {
			for (size_t i = 0; i < ClientCount.size(); i++)
			{
				if (ClientCount[i] == info) {
					char* data = server.GetData(info);
					auto result = EncodeLenAStr(data, 16);
					Log_DEBUG("log.log", ("Client-Server:" + result).c_str());
					server.Send(TurnerCount[i], result.c_str());
					break;
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 39,Repaired" << endl;
			}
			break;
		}
		else {
			cout << "WARN:Line 39,Client!=Turner" << endl;
		}
	}
}
void server_left(PINFO info) {
	bool isError = false;
	while (true)
	{
		if (ClientCount.size() == TurnerCount.size()) {
			for (size_t i = 0; i < ClientCount.size(); i++)
			{
				if (ClientCount[i] == info) {
					server.Close(info);
					ClientCount[i] = 0;
					turner.Close(TurnerCount[i]);
					TurnerCount[i] = 0;
					break;
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 60,Repaired" << endl;
			}
			break;
		}
		else {
			cout << "WARN:Line 60,Client!=Turner" << endl;
		}
	}
}
void turner_accept(PINFO info) {
	if (IsFlower == false) {//if the flower is the real flower.
		flower = info;
		IsFlower = true;
		cout << "Flower has been ready!" << endl;
	}
	else {
		TurnerCount.push_back(info);
	}
}
void turner_recv(PINFO info) {
	bool isError = false;
	while (true)
	{
		if (ClientCount.size() == TurnerCount.size()) {
			for (size_t i = 0; i < TurnerCount.size(); i++)
			{
				if (TurnerCount[i] == info) {
					auto data = DecodeLenAStr(turner.GetData(info), 16);
					for (auto k : data) {
						Log_DEBUG("log.log", ("Turner-Client:" + k).c_str());
						server.Send(ClientCount[i], k.c_str());
					}
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 91,Repaired" << endl;
			}
			break;
		}
		else {
			cout << "WARN:Line 91,Client!=Turner" << endl;
		}
	}
}
void turner_left(PINFO info) {
	bool isError = false;
	while (true) {
		if (ClientCount.size() == TurnerCount.size()) {
			if (info == flower) {
				turner.Close(info);
				exit(0);
			}
			else {
				for (size_t i = 0; i < TurnerCount.size(); i++)
				{
					if (TurnerCount[i] == info) {
						turner.Close(info);
						TurnerCount[i] = 0;
						server.Close(ClientCount[i]);
						ClientCount[i] = 0;
					}
				}
			}
			if (isError == true) {
				cout << "GOOD:Line 116,Repaired" << endl;
			}
			break;
		}
		else {
			cout << "WARN:Line 116,Client!=Turner" << endl;
		}
	}
	cout << "A Turner Client left" << endl;
}
int main() {
	short server_port = 9998;
	short turner_port = server_port + 1;
	server.Match(server_accept, server_recv, server_left);
	turner.Match(turner_accept, turner_recv, turner_left);
	server.Start("0.0.0.0", server_port, 100, 1, 0x10000);
	turner.Start("0.0.0.0", turner_port, 100, 1, 0x10000);
	while (true);
	return 0;
}