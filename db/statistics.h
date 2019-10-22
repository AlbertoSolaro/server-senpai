#pragma once
#include <vector>
#define N_ore 6
#define N_giorni 7
using namespace std;

class statistics {
public:
	vector<int> periodi;
	int count_periodi;
    int count_ril;

    statistics(int i) {
        if(i==1)
            periodi.assign(N_giorni,0);
        else
            periodi.assign(N_ore,0);

        count_periodi = 0;
        count_ril=0;
    }
};
