#include "car_sender.h"
#include <iostream>

int main() {
  CarSys car(5, 5);
  thread t1 = car.threading();
  t1.join();
  vector<Coordinate> path = car.getPath();
  for (auto p : path){
    cout << p.x << ',' << p.y << endl;
  }
  sleep(5);
  thread t2 = car.threading();
  t2.join();
  path = car.getPath();
  for (auto p :path) {
    cout << p.x << ',' << p.y << endl;
  }
}
