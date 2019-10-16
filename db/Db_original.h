#pragma once
#include <iostream>
#include "lib/sqlite3.h"
#include <map>
#include <set>
#include <vector>
#include "schema/mac_time.h"
#include "schema/schema_original.h"
#include "schema/schema_triang.h"
#include "statistics.h"
#include "best_k_mac.h"
#include "triangulation.h"
using namespace std;

class Db_original {
	static map<string, statistics> stat;
    static map<string, num_ril> count_ril;
    static map<mac_time, set<schema_original>> dati_scheda;
	static vector<schema_triang> last_positions_ril;
	sqlite3 *db;

   static map<string,map<string,set<schema_original>>> rilevazioni;
    static int callback1(void *data, int argc, char **argv, char **azColName);


	static int callback(void *data, int argc, char **argv, char **azColName);
	static int callback_count_ril_pub(void *data, int argc, char **argv, char **azColName);
	static int callback_count_ril_no_pub(void *data, int argc, char **argv, char **azColName);
	static int callback_last_pos(void *data, int argc, char **argv, char **azColName);
	static int callback_statistics(void *data, int argc, char **argv, char **azColName);

public:
    Triangulation triang;
	Db_original();

    void loop1(time_t timestamp);

    void loop(time_t timestamp);
    map<string,num_ril> number_of_rilevations(time_t timestamp_start, time_t timestamp_end);
	vector<schema_triang> last_positions(time_t timestamp);
	best_k_mac statistics_fun(time_t timestamp_start, int mode);
	~Db_original();
};

