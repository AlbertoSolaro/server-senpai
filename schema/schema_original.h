#pragma once
#include<string>
using namespace std;

class schema_original {
public:
	int ID;
	string MAC;
	bool isPub;
	int RSSI;
	string timestamp;
	string root;

	schema_original(char* ID, char* MAC, char* isPub, char* RSSI, char* timestamp, char*root);
    schema_original(int ID, string MAC,int isPub,int RSSI,string timestamp,string root);
	bool operator <(const schema_original &r) const {
		if (this->timestamp< r.timestamp || this->MAC<r.MAC||this->root<r.root)
			return true;
		else
			return false;
	}
};

