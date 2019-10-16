#include "schema_original.h"
#include<string>


schema_original::schema_original(char* ID, char* MAC,char* isPub,char* RSSI,char* timestamp,char*root) {
	this->ID = atoi(ID);
	this->MAC= MAC;	
	this->isPub = atoi(isPub);
	this->RSSI = atoi(RSSI);
	this->timestamp=timestamp;
	this->root =root;

}
schema_original::schema_original(int ID, string MAC,int isPub,int RSSI,string timestamp,string root) {
    this->ID = ID;
    this->MAC= MAC;
    this->isPub = isPub;
    this->RSSI = RSSI;
    this->timestamp=timestamp;
    this->root =root;

}
