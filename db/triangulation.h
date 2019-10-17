#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include "schema/helperclass.h"
#include "schema/schema_original.h"
#include "schema/schema_triang.h"

using namespace std;

class Triangulation {
  static map<string, Point> roots;
  static vector<Distance> distances;

public:
   static int nschede;
   static int measure_power; //potenza misurata a un metro di distanza
   static float constant_envir; //constante ambientale(da 2 a 4)

  Triangulation();
  static void initTriang(map<string, Point> vector_dati, int, float, int);
  static Point triangolate(vector<schema_original>);

  static float rssi2meter(int rssi);
  static vector<Point> findPoints(Distance, map<string, float>);
  static Point findTruePoint(vector<Point>);
  static map<string, Point> getRoots() { return roots; }
  static void setRoots(map<string,Point> r) { roots = r; }
  static void setMeasuredPower(int m){measure_power=m;}
  static void setConstantEnvir(float e){constant_envir=e;}
  static void setNschede(int n){nschede=n;}
  static vector<Distance> getDistances() { return distances; }
  static void setDistances(vector<Distance> d) { distances = d; }
};
