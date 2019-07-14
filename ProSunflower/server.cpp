#include "SunTCP.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;
SunTCPServer server;
SunTCPServer turner;
bool IsFlower = false;
PINFO flower;
vector<PINFO> ClientCount;
vector<PINFO> TurnerCount;

void server_accept(PINFO info) {
	server.Send(flower, "A");
	ClientCount.push_back(info);
}
void server_recv(PINFO info) {
	for (size_t i = 0; i < ClientCount.size(); i++)
	{
		if (ClientCount[i] == info) {
			char* data = server.GetData(info);
			Log_DEBUG("log.log", ("Client-Server:" + EncodeLenAStr(data, 16)).c_str());
			server.Send(TurnerCount[i], EncodeLenAStr(data, 16).c_str());
			break;
		}
	}
}
void server_left(PINFO info) {
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
}
void turner_left(PINFO info) {
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