#pragma once

class schema_triang {
public:
	int ID;
    char MAC[20];
	bool isPub;
    char timestamp[20];
    float x;
    float y;

	schema_triang(char* ID, char* MAC, char* isPub, char* timestamp, char* x, char* y);
    schema_triang(int ID, const char* MAC, int isPub, const char* timestamp, float x, float y);
};

