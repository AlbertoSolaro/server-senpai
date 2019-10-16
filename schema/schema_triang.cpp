#include "schema_triang.h"
#include <string>


schema_triang::schema_triang(char* ID, char* MAC, char* isPub, char* timestamp, char* x, char* y)
{
	this->ID = atoi(ID);
    strcpy_s(this->MAC, MAC);
	this->isPub = atoi(isPub);
    strcpy_s(this->timestamp, timestamp);
	this->x = atoi(x);
	this->y = atoi(y);
}

schema_triang::schema_triang(int ID, const char* MAC, int isPub, const char* timestamp, float x, float y)
{
	this->ID = ID;
    strcpy_s(this->MAC, MAC);

	this->isPub = isPub;
    strcpy_s(this->timestamp, timestamp);
    this->x =x;
	this->y = y;
}
