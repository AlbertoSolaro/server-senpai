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

  static float distance(Point point1, Point point2) {
    return sqrt(pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) * 1.0);
  }

  static float rssi2meter(int rssi){
    return pow(10,((measure_power)-rssi)/(constant_envir*10));
  }

  static void kLargest(Column* arr, int k);




public:
   static int nschede;
   static int measure_power; //potenza misurata a un metro di distanza
   static float constant_envir; //constante ambientale(da 2 a 4)

  Triangulation() {}
  static void initTriang(map<string, Point> vector_dati, int, float, int);
  static Point triangolate(vector<schema_original>);

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
 
