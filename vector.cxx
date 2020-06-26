/*
 * vector.cxx
 * 
 * Simple testbed for vectors.
 * 
 * Copyright 2020 robert smith
 */

#include <iostream>
#include <vector>
using namespace std;


int main (int argc, char **argv) {  
  vector<int> v;
  v.push_back(10);
  v.push_back(20);
  v.push_back(30);
  cout << "v.size() = " << v.size() << endl;
  for (unsigned int n = 0; n < v.size(); n++) {
    cout << "at:" << to_string(v.at(n)) << endl;
  }
  for (vector<int>::iterator it = v.begin() ; it != v.end(); ++it) {
    cout << "iterator:" << to_string(*it) << endl;
  }
  for (unsigned int n = 0; n < v.size(); n++) {
    cout << "[]:" << to_string(v[n]) << endl;
  }
  return 0;
}
