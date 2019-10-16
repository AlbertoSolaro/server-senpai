#pragma once

#include <string>
#include <map>
#include <vector>

using namespace std;

class Column {
  public:
    int pos;

    Column();
    Column(int pos) { this->pos = pos; }

    map<int, float> distances;
    vector<int> topk;
};

class Distance {
   public:
    Distance(float distance, string mac1, string mac2)
    {
        this->distance = distance;
        this->mac1 = mac1;
        this->mac2 = mac2;
    }
    float distance;
    string mac2;
    string mac1;
};

class Point {
  public:
    float x;
    float y;
    Point(float x, float y) { this->x = x; this->y = y; }
};

class num_ril {
  public:
    int n_pub;
    int n_priv;
    num_ril() { n_pub=0; n_priv=0; };
};