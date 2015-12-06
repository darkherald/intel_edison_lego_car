#include "car_sender.h"
#include <iostream>
#include <time.h>

int main() {
  CarSys car(5, 5);
  srand(time(NULL));
  while (true) {
  thread t1 = car.threading();
  t1.join();
  vector<Coordinate> path = car.getPath();
  for (auto p : path){
    cout << p.x << ',' << p.y << endl;
  }
  sleep(5);
  car.setCarPos(rand() % 100, rand() % 100);
  }
}
