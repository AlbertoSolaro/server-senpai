#include "Db_original.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <atltime.h>
#include <string>
#include "lib/sqlite3.h"
#include <stdarg.h>
#include <set>
#include <vector>
#include <cmath>
#include "QDebug"
#include "schema/mac_time.h"
#include "schema/schema_original.h"
#include "schema/schema_triang.h"


#include "triangulation.h"

#define K_BEST 2 //numero di mac piu frequenti ritornati
#define INT_SEC 120   //intervallo valido per mostrare l'ultima posizione
#define N_rilevazioni 1  //numero rilevazioni necessarie per essere "continuamente presente"
#define N_schede 3		//numero di schede di rilevazione
#define minuti_cont 5	//intervallo di tempo in cui rilevare dispositivi presenti
using namespace std;

//Database contenente tutti i dati provenienti dalle varie schedine.
//N e una variabile di configurazione  globale che indica il numero di schedine
//Si prendono i dati comuni


Db_original::Db_original() {
    sqlite3_open("Progetto_malnati.db", &db);

}

map<mac_time, set<schema_original>>  Db_original::dati_scheda;
map<string,num_ril> Db_original::count_ril;
vector<schema_triang> Db_original::last_positions_ril;
map<string, statistics> Db_original::stat;

int Db_original::callback(void *data, int argc, char **argv, char **azColName) {
    (void)data;(void)argc;(void)argv;(void)azColName;

    int i;

    //i record sono inseriti in una mappa avente MAC-TIMESTAMP come chiave e un set dei record aventi ROOT diversi
    //Cosi tutte le rilevazioni allo stesso tempo per lo stesso mac sono inserite nel set, e si eliminano doppioni
    mac_time key(argv[1],argv[4]);
    schema_original dati(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]);
    auto found = dati_scheda.find(key);

    if (found != dati_scheda.end()) {
        found->second.insert(dati);
        cout << (found->first) << " size=" <<found->second.size()<<endl;
    } else {
        set<schema_original> v = {dati};
        dati_scheda.insert(pair<mac_time,set<schema_original>>(key,v));
    }

    /*// da eliminare, comodo per debug iniziale
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
*/
    return 0;
}


void Db_original::loop(time_t timestamp)  {
    //La funzione riceve il timestamp e calcola l'intervallo di tempo da cui prelevare il dato

    //n_sec e un parametro globale che indica ogni quanto la pulizia e effettuata
    int n_sec = 30;
    struct tm  timeinfo;
    time_t rawtime;

    CTime timestamp_in(timestamp);
    char timestamp_in_char[30];
    rawtime = timestamp_in.GetTime();
    localtime_s(&timeinfo, &rawtime);
    strftime(timestamp_in_char, 30, "%Y%m%d%H%M%S", &timeinfo);
    string timestamp_in_s(timestamp_in_char);

    char timestamp_fin_char[30];
    CTimeSpan sec_range(0, 0, 0, n_sec);
    CTime timestamp_fin = timestamp_in + sec_range;
    rawtime = timestamp_fin.GetTime();
    localtime_s(&timeinfo, &rawtime);
    strftime(timestamp_fin_char, 30, "%Y%m%d%H%M%S", &timeinfo);
    string timestamp_fin_s(timestamp_fin_char);

    //seleziono tutti i record aventi tempo nell'intervallo

    string sql;
    sql = "SELECT * FROM Originale where TIMESTAMP >='"+timestamp_in_s+"' AND TIMESTAMP <'" + timestamp_fin_s+"'";

    /* Execute SQL statement */
    //char *zErrMsg = nullptr;
    int rc;
    const char* data = "Callback function called";

    rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data,NULL); //per oni record ritornato dalla query chiamo la callback

    if (rc != SQLITE_OK) {
        //fprintf(stderr, "SQL error: %s\n", zErrMsg);
        qDebug()<< "ERROR select from Originale: "<< sqlite3_errmsg(db);

       // sqlite3_free(zErrMsg);
    } else {
        //fprintf(stdout, "Operation done successfully\n");
    }

    //creo una mappa con MAC->TIMESTAMP in cui salvo solo le rilevazioni ricevute da N_schede e mantenendo solo l'ultima rilevazione
    map<string, string> last_mac;
    for (map<mac_time, set<schema_original>>::iterator it = dati_scheda.begin(); it != dati_scheda.end(); ++it) {
        if ((it->second).size() == /*triang.n_schede */ N_schede)
        {
            map<string, string>::iterator selected = last_mac.find(string((it->first).MAC));
            if (selected == last_mac.end()) {
                last_mac.insert(pair<string, string>(string((it->first).MAC), string((it->first).timestamp)));
            }
            else
            {
                if (selected->second < string((it->first).timestamp))
                {
                    selected->second = string((it->first).timestamp);
                }
            }
        }

    }

    for (map<string, string>::iterator selected = last_mac.begin(); selected != last_mac.end(); ++selected) {
        set<schema_original> dati = dati_scheda.find(mac_time((selected->first).c_str(), (selected->second).c_str()))->second;
        vector<schema_original> vector_dati(dati.begin(), dati.end());

        //per ogni MAC-TIMESTAMP che rispetta la condizione, chiamo la triangolazione passando un vector con i record e il numero di record del vettore(e quindi delle schede)
        //schema_triang dato_triang = triangolazione(vector_dati, N_schede);
        Point estimated_point = triang.triangolate(vector_dati);

        //serve solo per test
        //schema_triang dato_triang(3, "98-54-1B-31-AC-8B", 0, "20191001190020", 10, 20);
    if(!isnan(estimated_point.x)||!isnan(estimated_point.y)){
        schema_triang dato_triang(69, selected->first.c_str(), vector_dati[0].isPub, selected->second.c_str(), estimated_point.x, estimated_point.y);

        //inserisco il risultato della triangolazione nella tabella History
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "insert into History (MAC,ISPUB,TIMESTAMP,X,Y) values (?2,?4,?5,?6,?7);", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 2, dato_triang.MAC, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, dato_triang.isPub);
        sqlite3_bind_text(stmt, 5,dato_triang.timestamp, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 6, dato_triang.x );
        sqlite3_bind_double(stmt, 7, dato_triang.y );

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            qDebug()<< "ERROR inserting data: "<< sqlite3_errmsg(db);
           // printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));

        }

        sqlite3_finalize(stmt);
    }
    }
/*
    //elimino le righe analizzate dalla tabellaa originale
    sql = "DELETE FROM Original where TIMESTAMP >='" + timestamp_in_s + "' AND TIMESTAMP <'" + timestamp_fin_s + "'";
    rc = sqlite3_exec(db, sql.c_str(), NULL, (void*)data, &zErrMsg); //per oni record ritornato dalla query chiamo la callback

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else {
        fprintf(stdout, "Operation done successfully\n");
    }

*/
    dati_scheda.clear();
    last_mac.clear();
    //sqlite3_free(zErrMsg);
}

int Db_original::callback_count_ril_pub(void *data, int argc, char **argv, char **azColName) {
    (void)data;(void)argc;(void)argv;(void)azColName;
    //ogni volta che arriva un record vado a implementare il contatore delle rilevazioni pubbliche(ovvero bit 0) nell'orario rilevato,
    //ovvero nell'ultimo elemento della mappa, perche inserisco gli elementi uno alla volta prima di fare la query al database

    (count_ril.rbegin())->second.n_pub++;

    return 0;
}

int Db_original::callback_count_ril_no_pub(void *data, int argc, char **argv, char **azColName) {
    (void)data;(void)argc;(void)argv;(void)azColName;
    //ogni volta che arriva un record vado a implementare il contatore delle rilevazioni non pubbliche(ovvero bit 1) nell'orario rilevato,
    //ovvero nell'ultimo elemento della mappa, perche inserisco gli elementi uno alla volta prima di fare la query al database

    (count_ril.rbegin())->second.n_priv++;

    return 0;
}


map<string,num_ril> Db_original::number_of_rilevations(time_t timestamp_start, time_t timestamp_end) {
    //questa funzione riceve due timestamp: il primo impostato dall'utente, il secondo che abbia differenza di 5 minuti, e possono essere impostati dall'utente e consideriamo la mezzora successiva, mostrando le statistiche ogni 5 minuti

    //questa soluzione conta quante sono le rilevazioni di un dispositivo nei 5 minuti e restituisce solo quelle rilevate piu di N_volte
    //N volte potrebbe essere 1,2,3... a seconda di quanti probe vediamo nei test: Daniel ha messo 1, potremmo usare 2

    struct tm  timeinfo;
    time_t rawtime;
    /* Execute SQL statement */
    char *zErrMsg = 0;
    int rc;
    const char* data = "Callback function called";

    count_ril.clear(); //pulisco la mappa che e allocata solo una volta e pu? essere stata usata precedentemente


    CTime timestamp_in(timestamp_start);
    CTime timestamp_stop(timestamp_end);

    char timestamp_in_char[20];
    CTime timestamp_fin;
    CTimeSpan min_range(0, 0, minuti_cont, 0);
    char timestamp_fin_char[20];

    int init_count[2] = { 0, 0 };
    for (; timestamp_in != timestamp_stop; timestamp_in = timestamp_fin) {

        rawtime = timestamp_in.GetTime();
        localtime_s(&timeinfo, &rawtime);
        strftime(timestamp_in_char, 20, "%Y%m%d%H%M%S", &timeinfo);
        string timestamp_in_s(timestamp_in_char);

        timestamp_fin = timestamp_in + min_range;
        rawtime = timestamp_fin.GetTime();
        localtime_s(&timeinfo, &rawtime);
        strftime(timestamp_fin_char, 20, "%Y%m%d%H%M%S", &timeinfo);
        string timestamp_fin_s(timestamp_fin_char);
        num_ril n_ril;
        count_ril.insert(pair<string, num_ril>(timestamp_in_s, n_ril));  //ogni riga ha l'orario di inizio e due bit a 0: il bit 0
                                                                          //indica mac pubblico, il bit 1 indica mac non pubblico

        string sql;
        sql = "SELECT COUNT(*) FROM History where ISPUB=1 AND TIMESTAMP >='" + timestamp_in_s + "' AND TIMESTAMP <'" + timestamp_fin_s + "' GROUP BY MAC HAVING COUNT(DISTINCT TIMESTAMP)>="+ to_string(N_rilevazioni);

        rc = sqlite3_exec(db, sql.c_str(), callback_count_ril_pub, (void*)data, &zErrMsg); //per oni record ritornato dalla query chiamo la callback

        if (rc != SQLITE_OK) {
            clog << "SQL error in Query count_rilevazioni_pub: " << zErrMsg << endl;
            sqlite3_free(zErrMsg);
        }
        else {
          //  clog << "Query count_rilevazioni_pub tra " << timestamp_in_s << " and " << timestamp_fin_s << " effettuata correttamente" <<endl;
        }

        sql = "SELECT COUNT(*) FROM History where ISPUB=0 AND TIMESTAMP >='" + timestamp_in_s + "' AND TIMESTAMP <'" + timestamp_fin_s + "' GROUP BY MAC HAVING COUNT(DISTINCT TIMESTAMP)>=" + to_string(N_rilevazioni);

        rc = sqlite3_exec(db, sql.c_str(), callback_count_ril_no_pub, (void*)data, &zErrMsg); //per oni record ritornato dalla query chiamo la callback

        if (rc != SQLITE_OK) {
            clog << "SQL error in Query count_rilevazioni_no_pub : " << zErrMsg << endl;
            sqlite3_free(zErrMsg);
        } else {
          //  clog << "Query count_rilevazioni_pub tra " << timestamp_in_s << " and " << timestamp_fin_s << " effettuata correttamente" << endl;
        }
    }
     return count_ril;

}

int Db_original::callback_last_pos(void *data, int argc, char **argv, char **azColName) {
    (void)data;(void)argc;(void)argv;(void)azColName;
    //salvo il record in un vector

    schema_triang dati_ril(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);

    last_positions_ril.push_back(dati_ril);

    return 0;
}

vector<schema_triang> Db_original::last_positions(time_t timestamp) {

    //questa funzione riceveun orario e restituisce l'ultima posizione dei dispositivi rilevati entro gli ultimi INT_SEC secondi
    //questa funzione e chiamata ogni n secondi(30?)

    struct tm  timeinfo;
    time_t rawtime;

    last_positions_ril.clear();

    CTime timestamp_fin(timestamp);
    char timestamp_fin_char[20];
    rawtime = timestamp_fin.GetTime();
    localtime_s(&timeinfo, &rawtime);
    strftime(timestamp_fin_char, 20, "%Y%m%d%H%M%S", &timeinfo);
    string timestamp_fin_s(timestamp_fin_char);

    char timestamp_in_char[20];
    CTimeSpan sec_range(0, 0, 0, INT_SEC);
    CTime timestamp_in = timestamp_fin - sec_range;
    rawtime = timestamp_in.GetTime();
    localtime_s(&timeinfo, &rawtime);
    strftime(timestamp_in_char, 20, "%Y%m%d%H%M%S", &timeinfo);
    string timestamp_in_s(timestamp_in_char);

    //seleziono tutti i record aventi tempo nell'intervallo

    string sql;
    sql = "SELECT * FROM History AS H1 where TIMESTAMP=( SELECT MAX(TIMESTAMP) FROM History AS H2 WHERE H1.MAC = H2.MAC AND H2.TIMESTAMP >='" + timestamp_in_s + "' AND H2.TIMESTAMP <'" + timestamp_fin_s + "')";

    /* Execute SQL statement */
    char *zErrMsg = 0;
    int rc;
    const char* data = "Callback function called";

    rc = sqlite3_exec(db, sql.c_str(), callback_last_pos, (void*)data, &zErrMsg); //per oni record ritornato dalla query chiamo la callback

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else {
       // fprintf(stdout, "Operation done successfully\n");
    }

    return last_positions_ril;
}

int Db_original::callback_statistics(void *data, int argc, char **argv, char **azColName) {
    (void)data;(void)argc;(void)argv;(void)azColName;
    //guardo se il mac e gia persente nella mappa: se lo e, vado a inidividuare il periodo della rilevazione accedendo a data[1], e incremento la cella corrispondente e il contatore
    //Se non e presente, inzerisco un nuovo record

    string MAC=argv[0];
    int * info = (int*)data;
    cout << "info " << info[0] << " - " << info[1] << endl;
    map<string, statistics>::iterator found;
    found = stat.find(MAC);
    if (found == stat.end()) {

        if (info[0] != 1)
        {
            statistics s(0); //chiamo costruttore con vector di 6, per ore
            found = stat.insert(pair<string, statistics>(MAC, s)).first;
        }
        else {
            statistics s(1); //chiamo costruttore con vector di 7, per 7 giorni
            found = stat.insert(pair<string, statistics>(MAC, s)).first;
        }
    }

    if((found->second).periodi.at(info[1])==0)
        (found->second).count_periodi++;
    (found->second).periodi.at(info[1])+=atoi(argv[1]);


    return 0;
}


best_k_mac Db_original::statistics_fun(time_t timestamp_start, int mode)
{
    //La funzione riceve un timestamp iniziale e il mode per calcolare periodo finale e divisione in fasce, divide il periodo in intervalli, controlla in quali intervalli il dispositivo e stato rilevato e ritorna i piu rilevati

    //oppure un solo timestamp(inizio o fine) e il range che si vuole(3ore, 6ore, 1giorno,ultima settimana)

    struct tm  timeinfo;
    time_t rawtime;
    char *zErrMsg = nullptr;
    int rc;
    //const char* data = "Callback function called";

    stat.clear(); //pulisco la mappa che e allocata solo una volta e puo essere stata usata precedentemente

    CTime timestamp_in(timestamp_start);
    CTime timestamp_end;
    CTimeSpan range;
    int range_variazione;
    int durata;
    int information[2] = { 0,0 }; //vettore da passare alla callback: primo bit indica se usare vectr lunghezza 6 o 7, secondo bit indica il periodo
    switch (mode) {
    case 1: //range 2 ore
        durata = 2;
        timestamp_end=timestamp_in + CTimeSpan(0, durata, 0, 0);
        range_variazione=20;
        range = CTimeSpan(0, 0, range_variazione, 0);
        break;
    case 2: //range 6 ore
         durata = 6;
        timestamp_end=timestamp_in + CTimeSpan(0, durata, 0, 0);
         range_variazione=1;
        range=CTimeSpan(0, range_variazione, 0,0);
        break;
    case 3: //range 24 ore
        durata = 1;
        timestamp_end=timestamp_in + CTimeSpan(durata, 0, 0, 0);
        range_variazione=4;
        range = CTimeSpan(0, range_variazione,0, 0);
        break;
    case 4: //range 1 settimana
        durata = 7;
        information[0] = 1; //dico che deve usare 'oggetto con vector di dimensione 7
        timestamp_end=timestamp_in + CTimeSpan(durata, 0, 0, 0);
        range_variazione=1;
        range = CTimeSpan(range_variazione, 0, 0, 0);
        break;
    }


    char timestamp_in_char[20];
    CTime timestamp_fin;
    char timestamp_fin_char[20];

    vector<string> etichette_periodo;
    //int init_count[] = { 0, 0 };
    int i;
    //ad ogni ciclo faccio la query per un oeriodo diverso e salvo l'etichetta del periodo
    for (i=0; timestamp_in != timestamp_end; timestamp_in = timestamp_fin,i++) {
        information[1] = i;
        rawtime = timestamp_in.GetTime();
        localtime_s(&timeinfo, &rawtime);
        strftime(timestamp_in_char, 20, "%Y%m%d%H%M%S", &timeinfo);
        string timestamp_in_s(timestamp_in_char);

        timestamp_fin = timestamp_in + range;
        rawtime = timestamp_fin.GetTime();
        localtime_s(&timeinfo, &rawtime);
        strftime(timestamp_fin_char, 20, "%Y%m%d%H%M%S", &timeinfo);
        string timestamp_fin_s(timestamp_fin_char);

        etichette_periodo.push_back(timestamp_in_s);  //inserisco la data-ora

        string sql;
        sql = "SELECT MAC,COUNT(*) FROM History where TIMESTAMP >='" + timestamp_in_s + "' AND TIMESTAMP <'" + timestamp_fin_s + "' GROUP BY MAC HAVING COUNT(DISTINCT TIMESTAMP)>=1" ;

        rc = sqlite3_exec(db, sql.c_str(), callback_statistics, (void*)information, &zErrMsg); //per oni record ritornato dalla query chiamo la callback

        if (rc != SQLITE_OK) {
            clog << "SQL error in Query count_rilevazioni_pub: " << zErrMsg << endl;
            sqlite3_free(zErrMsg);
        } else {
         //   clog << "Query count_rilevazioni_pub tra " << timestamp_in_s << " and " << timestamp_fin_s << " effettuata correttamente" << endl;
        }
    }

    //creo un comparatore per ordinare la mappa in modo da avere prima i mac rilevati in piu periodi
    typedef function<bool(pair<string, statistics>, pair<string, statistics>)> Comparator;

    Comparator compFunctor = [](pair<string, statistics> elem1, pair<string, statistics> elem2) {
        return elem2.second.count_periodi < elem1.second.count_periodi;
    };

    set<pair<string, statistics>, Comparator> ordered_stat(	stat.begin(), stat.end(), compFunctor);

    //salvo i K_BEST mac aventi maggiore frequenza: salvo il vettore con le etichette e un vettore di MAC->vettore_frequenze_periodo
    best_k_mac best_macs(etichette_periodo);
    set<pair<string, statistics>>::iterator it = ordered_stat.begin();
    for (i = 0; i < K_BEST && it!=ordered_stat.end(); i++, ++it)
    {
        best_macs.insert(pair<string, vector<int>>(it->first, it->second.periodi));
    }
    if(i==0)
    {
        best_k_mac vuoto(mode);
        return vuoto;
    }
    if(i==1)
    {
        best_macs.insert(pair<string,vector<int>>("0",vector<int>(etichette_periodo.size(),0)));
    }

    return best_macs;
}


Db_original::~Db_original() {
    sqlite3_close(db);
}
