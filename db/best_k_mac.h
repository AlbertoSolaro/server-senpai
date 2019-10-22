#pragma once
#include <set>
#include <vector>
#include <string>
using namespace std;

class best_k_mac {
public:
	vector<pair<string, vector<int>>> best_mac;
	vector<string> etichette_periodi;
    vector<int> n_period;
    void insert(pair < string, vector<int>> mac_periods,int n_periods){
        best_mac.push_back(mac_periods);
        n_period.push_back(n_periods);
    }

    best_k_mac(vector<string> etichette){
        vector<pair<string, vector<int>>> best_mac;
        etichette_periodi.assign(etichette.begin(),etichette.end());
    }
    best_k_mac(int mode){
        if(mode==4){
            vector<int> giorni(7,0);
            for(int i=0;i<2;i++)
            {
                best_mac.push_back(pair<string,vector<int>>("0",giorni));
            }
        }else{
            vector<int> giorni(6,0);
            for(int i=0;i<2;i++)
            {
                best_mac.push_back(pair<string,vector<int>>("0",giorni));
            }
        }

    }
    best_k_mac() {}
};
