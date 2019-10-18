#include "triangulation.h"
#include <limits>
#include <set>
#include <iostream>
#include <algorithm>
#include <QDebug>
#include <functional>
#include <string>
#include "schema/helperclass.h"

using namespace std;

map<string, Point> Triangulation::roots;
vector<Distance> Triangulation::distances;

int Triangulation::measure_power=-43;
float Triangulation::constant_envir=2.5;
int Triangulation::nschede;
float Triangulation::margin = 0.5;

void Triangulation::initTriang(map<string, Point> roots, int measured_power, float enviromental_constant, int nschede) {
  Triangulation::setRoots(roots);
  Triangulation::setConstantEnvir(enviromental_constant);
  Triangulation::setMeasuredPower(measured_power);
  Triangulation::setNschede(nschede);

  for(auto scheda1 : roots) {
    for(auto scheda2 : roots) {
      if(!scheda1.first.compare(scheda2.first)) continue;
      Point point1 = scheda1.second, point2 = scheda2.second;

      float distance=sqrt(pow(point1.x - point2.x, 2) +pow(point1.y - point2.y, 2) * 1.0);
      Distance dist = Distance(distance, scheda1.first, scheda2.first);

      distances.push_back(dist);
    }
  }
}

Point Triangulation::triangolate(vector<schema_original> vector_dati) {
  auto distances = getDistances();
  if(distances.empty())
      qDebug() << "CAZZO DI BUDDAH";

  map<string, float> dists;
  vector<Point> allpoints;

  for(auto v : vector_dati) {
    dists.insert(pair<string, float>(v.root, Triangulation::rssi2meter(v.RSSI)));
    qDebug() << "MAC_disp: " << v.MAC.c_str() << "MAC_scheda: "<< v.root.c_str();
    qDebug() << " - triangulation::rssi= " << v.RSSI << " triangulation::meter= " << (double)Triangulation::rssi2meter(v.RSSI);
  }

  for(auto d : distances){
    vector<Point> points = Triangulation::findPoints(d, dists);
    for(auto p : points)
      allpoints.push_back(p);
  }

  Point result=Triangulation::findTruePoint(allpoints);
  qDebug() << "Point found: x=" << result.x << " y=" << result.y << endl << endl;
  return result;
}

vector<Point> Triangulation::findPoints(Distance rootDistance, map<string, float> dists) {
  auto roots = Triangulation::getRoots();
  vector<Point> ret;

  float r0=dists.find(rootDistance.mac1)->second;
  float x0=roots.find(rootDistance.mac1)->second.x;
  float y0=roots.find(rootDistance.mac1)->second.y;

  float r1=dists.find(rootDistance.mac2)->second;
  float x1=roots.find(rootDistance.mac2)->second.x;
  float y1=roots.find(rootDistance.mac2)->second.y;

  float d=rootDistance.distance;

  if(d>(r0+r1)||d<(sqrt(pow(r0-r1, 2)))) {
      qDebug()<<"d="<<d<<" r0+r1="<<r0+r1<<" r0-r1="<<abs(r0-r1);
      return ret;

      /*if(d > (r0+r1+margin) || d<(abs(r0-r1)-margin)){
          qDebug()<<"d="<<d<<" r0+r1="<<r0+r1<<" r0-r1="<<abs(r0-r1);
          return ret;
      } else if(d <= (r0+r1+margin) && d > (r0+r1)){
          float diff = d - (r0 + r1) - (float)0.1;
          if(r0 < r1) r0 += diff;
          else r1 += diff;

          // r0 = r0 + diff / 2;
          // r1 = r1 + diff / 2;
      } else if(d>=(abs(r0-r1)-margin) && d<(sqrt(pow(r0-r1, 2)))) {
          float diff = sqrt(pow(r0-r1, 2)) - d + (float)0.1;
          if(r0 < r1) r1 -= diff;
          else r0 -= diff;
      } else {
          qDebug() << "Torna a studiare logica";
      }*/
  }

  float a=(pow(r0,2)-pow(r1,2)+pow(d,2))/(2*d);
  float h=sqrt(pow(r0,2)-pow(a,2));

  float x2=x0+a*(x1-x0)/d;
  float y2=y0+a*(y1-y0)/d;

  qDebug() << "X2: " << x2 << " - Y2: " << y2 << endl;

  float x31=x2+h*(y1-y0)/d;
  float x32=x2-h*(y1-y0)/d;
  float y31=y2-h*(x1-x0)/d;
  float y32=y2+h*(x1-x0)/d;

  qDebug() << "X31: " << x31 << " - Y31: " << y31 << endl;
  qDebug() << "X32: " << x32 << " - Y32: " << y32 << endl;

  Point point1=Point(x31, y31);
  Point point2=Point(x32, y32);
  ret.push_back(point1);
  ret.push_back(point2);

  return ret;
}


typedef function<bool(pair<int, int>, pair<int, int>)> Comparator2;
  vector<pair<int,float>> sortMap(map<int, float> m, Comparator2 comp){
    vector<pair<int, float>> vec;

    std::copy(m.begin(),
              m.end(),
              std::back_inserter<std::vector<pair<int, float>>>(vec));

    std::sort(vec.begin(), vec.end(), comp);

    return vec;
  }

typedef function<bool(pair<int, int>, pair<int, int>)> Comparator3;
  vector<pair<int,int>> sortMap2(map<int, int> m, Comparator3 comp){
    vector<pair<int, int>> vec;

    std::copy(m.begin(),
              m.end(),
              std::back_inserter<std::vector<pair<int, int>>>(vec));

    std::sort(vec.begin(), vec.end(), comp);

    return vec;
  }

Point Triangulation::findTruePoint(vector<Point> points) {
  if (points.size() == 0) {
    qDebug() << "Points vuoto. Impossibile trovare punti";
    return Point(numeric_limits<float>::quiet_NaN(), numeric_limits<float>::quiet_NaN());
  }

  qDebug() << "points size" << points.size();

  auto roots = Triangulation::getRoots();

  vector<Column> matrix;
  int k_correct = roots.size() * (roots.size() - 1) / 2;
  int k = points.size() / 2;

  if (k_correct > k)
    qDebug() << "Punti scartati: " << k_correct - k;

  qDebug() << "K=" << k;

  for (int i = 0; i < points.size(); i++) {
    matrix.push_back(Column(i));

    for (int j = 0; j < points.size(); j++) {
      if (i == j)
        matrix[i].distances.insert(pair<int, float>(j, numeric_limits<float>::max()));
      else
        matrix[i].distances.insert(pair<int, float>(j, abs(distance(points[i], points[j]))));
    }
    kLargest(&matrix[i], k);
  }

  map<int, int> reps;
  qDebug() << "Points size: " << points.size();
  for (int i = 0; i < points.size(); i++)
    reps.insert(pair<int, int>(i, 0));

  for (int i = 0; i < points.size(); i++) {
    for (int j = 0; j < k; j++) {
      reps[matrix[i].topk[j]]++;
    }
  }

  // typedef function<bool(pair<int, int>, pair<int, int>)> Comparator;
  // Comparator compFunctor = ;

  // set<pair<int, int>, Comparator> setOfWords(reps.begin(), reps.end(), compFunctor);

    vector<pair<int, int>> setOfWords = sortMap2(reps,
                                          [](pair<int, int> elem1, std::pair<int, int> elem2) {
                                                if(elem1.second == elem2.second)
                                                  return elem1.first>elem2.first;
                                                else return (elem1.second>elem2.second);
                                          });

  vector<int> topkpos;
  for (int i = 0; i < k; i++)
     topkpos.push_back(next(setOfWords.begin(), i)->first);

  vector<Point> meanPoints;
  for (int i = 0; i < topkpos.size(); i++)
    meanPoints.push_back(points[topkpos[i]]);

  float sumx = 0;
  float sumy = 0;

  for (auto m : meanPoints) {
    qDebug() << "P:" << m.x << " - " << m.y;
    sumx += m.x;
    sumy += m.y;
  }

  float x = sumx / meanPoints.size();
  float y = sumy / meanPoints.size();
  Point retval = Point(x, y);

  return retval;
}

void Triangulation::kLargest(Column* arr, int k) {
  vector<pair<int, float>> setOfWords = sortMap(arr->distances,
    [](const pair<int, float> &l, const pair<int, float> &r) {
      if (l.second != r.second)
        return l.second > r.second;
      return l.first < r.first;
    });

  int i = 0;
  for (auto it = setOfWords.begin(); i < k && it != setOfWords.end(); i++, ++it)
    arr->topk.push_back(it->first);
}



