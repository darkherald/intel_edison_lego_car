#include <iostream>
#include "rearWheel.h"
using namespace std;

int main() {
  RearWheel *rw = new RearWheel();
  thread t1 = rw->threading();
  sleep(1);
  rw->clear();
  t1.join();
  delete rw;
}
