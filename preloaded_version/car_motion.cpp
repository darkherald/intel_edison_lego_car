//
//  car_motion.c
//  
//
//  Created by mingrui shi on 11/7/15.
//
//

#include "car_motion.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "IR_device.h"
#include <mraa/pwm.h>
#include <string>
#include <iostream>
#include "rearWheel.h"
#include "car_sender.h"

#define CENTER 0.0565f
#define Rr 31.2 // right R
#define Rl 32.7 // left R
#define V 14 // straight speed
#define Vr 12.40 // turning speed

#define C_w 13.8 // rear wheel circumference
#define A 1.8 // pid control coefficient
#define D_TH 20 // threshold distance
#define MIN_SPEED 36 // minimum speed

#define SPEED 50
#define PI 3.1415926535
#define MAXBUFSIZ 1024

using namespace std;

mraa_pwm_context speed_pwm_in1, speed_pwm_in2, turn_pwm;
int counter = 0;

bool running = false;
CarSys *car;

void speed_control(mraa_pwm_context in1, mraa_pwm_context in2, float speed) {
    speed = speed/100;
    if (speed >= 0) {
        mraa_pwm_write(in2, 1.0f);
        mraa_pwm_write(in1, 1.0f - speed);
    }
    else if (speed < 0) {
        mraa_pwm_write(in1, 1.0f);
        mraa_pwm_write(in2, 1.0f + speed);
    }
}

double getSpeed(double dist) {
    if (dist < D_TH)
      return MIN_SPEED;
    double s = A * dist;
    return s > 100 ? 100 : s;
}

double caculateMotionTime(Coordinate current, Coordinate destination, double offset, IR_device* ir){
    double dx = (destination.x - current.x); // x distance between destination and current
    double dy = (destination.y - current.y); // y distance between destination and current
    cout<<"Destination Coordinate ("<<destination.x<<", "<<destination.y<<")"<<endl;
    int turn = 0; // 0 means turn left 1 means turn right
    double alpha = atan(dy/dx); // angle between current and destinatnion
    if(dx < 0)
        alpha += PI;
    if(alpha > PI)
        alpha -= 2*PI;
    double beta = offset - alpha;
    if(beta > PI)
        beta -= 2*PI;
    else if(beta < -PI)
        beta += 2*PI;
    // determine turn direction
    if(beta > 0)
        turn = 1;
    double R = (turn)?Rr:Rl;
    if (beta < 0)
        beta = -beta;
    double d_2 = dx * dx + dy * dy;
    double l_2 = R * R + d_2 - 2 * R * sqrt(d_2)*sin(beta);
    double straight = sqrt(l_2 - R*R);
    double theta2 = acos((l_2 + R*R - d_2) /2/sqrt(l_2)/R);
    double theta1 = acos(R / sqrt(l_2));
    double turnangle;
    //case 1
    if(beta < PI/2)
    {
        turnangle = theta2 - theta1;
    //case 2
    } else {
        turnangle = 2*PI - theta1 -theta2;
    }
    double round = turnangle*R;
    
    int flag = (turn)?1:-1;
    //caculate turning center
    double x_c = current.x + R * sin(offset) *flag;
    double y_c = current.y - R * cos(offset) *flag;
    double gamma = offset + PI/2*flag;
    
    // caculate new offset
    if(turn)
    {
        offset -= turnangle;
    } else {
        offset += turnangle;
    }
    
    if (offset > PI) {
        offset -= 2*PI;
    } else if(offset < -PI)
        offset += 2*PI;
    
    double curveTime = round / Vr, straightTime = straight / V;
    
    //compensate for angle difference
    float turnCtrl = CENTER;
    if (turn)
        turnCtrl += 0.018f;
    else
        turnCtrl -= 0.015f;
    mraa_pwm_write(turn_pwm, turnCtrl);
    usleep(100000);
    
    cout<< "curveTime, straightTime, angleOffset"<<endl;
    printf("%.4f, %.4f", curveTime, straightTime);
    printf(" %.4f\n", offset);    

    RearWheel *rw = new RearWheel();

    //go for the curve
    thread t1 = rw->threading();
    
    double traveled = 0;
    while (round > traveled) {
      double left = round - traveled;
      cout<< "Distance left " << left <<endl;
      speed_control(speed_pwm_in1, speed_pwm_in2, getSpeed(left));
      usleep(500000);
      traveled = rw->getCount() * C_w / 360;
      cout<< "Traveled " << traveled << endl;
      double delta_gamma = traveled / R;
      double gamma_prime = gamma-delta_gamma*flag;
      double x_send = x_c + R * cos(gamma_prime);
      double y_send = y_c + R * sin(gamma_prime);
      car->setCarPos(x_send, y_send);
      cout<<"Coordinate Sended"<<x_send<<" "<< y_send<<endl;
      thread t_send_round = car->threading();
      t_send_round.join();
    }
    cout<<"Round Finished !!!!!!"<<endl;
    speed_control(speed_pwm_in1, speed_pwm_in2, 0.0f);
    usleep(100000);
    rw->clear();
    t1.join();

    //go straight
    turnCtrl = CENTER;
    mraa_pwm_write(turn_pwm, turnCtrl);

    thread t2 = rw->threading();
    
    traveled = 0;
    while (straight > traveled) {
      double left = straight - traveled;
      cout<< "Distance left " << left <<endl;
      speed_control(speed_pwm_in1, speed_pwm_in2, getSpeed(left));
      usleep(500000);
      traveled = rw->getCount() * C_w / 360;
      cout<< "Traveled " << traveled << endl;
      double x_send = destination.x - left * cos(offset);
      double y_send = destination.y - left * sin(offset);
      car->setCarPos(x_send, y_send);
      cout<<"Coordinate Sended"<<x_send<<" "<< y_send<<endl;
      thread t_send_straight = car->threading();
      t_send_straight.join();

    }
    cout<<"Straight part finished !!!!"<<endl;
    speed_control(speed_pwm_in1, speed_pwm_in2, 0.0f);
    usleep(100000);
    rw->clear();
    t2.join();

    //speed_control(speed_pwm_in1, speed_pwm_in2, SPEED);
    //usleep(curveTime * 1000000);
    
    /*
    turnCtrl = CENTER;
    mraa_pwm_write(turn_pwm, turnCtrl);
    usleep(100000);
    speed_control(speed_pwm_in1, speed_pwm_in2, SPEED);
    usleep(straightTime * 1000000);
*/
    cout<<"Sleep for 5 seconds"<<endl;
    sleep(5); 
    // receive message
    string msg = ir->recv();
    cout << msg << endl;
    thread t3 = rw->threading(msg);
    t3.join();
    return offset;
}

void car_report(CarSys *car) {
  while (running) {
    sleep(5);
    thread t = car->threading();
    t.join();
  }
}

int main(){
    float speed, turn;
    char speed_user_input[MAXBUFSIZ];
    char turn_user_input[MAXBUFSIZ];
    speed_pwm_in1 = mraa_pwm_init(3);
    speed_pwm_in2 = mraa_pwm_init(5);
    turn_pwm = mraa_pwm_init(6);
    
    if (speed_pwm_in1 == NULL || speed_pwm_in2 == NULL || turn_pwm == NULL) {
        fprintf(stderr, "Failed to initialized.\n");
        return 1;
    }
    
    mraa_pwm_period_us(speed_pwm_in1,870); //1150Hz
    mraa_pwm_enable(speed_pwm_in1, 1);
    mraa_pwm_period_us(speed_pwm_in2,870);
    mraa_pwm_enable(speed_pwm_in2, 1);
    
    mraa_pwm_period_ms(turn_pwm,20);
    mraa_pwm_enable(turn_pwm, 1);
    
    mraa_pwm_write(turn_pwm, CENTER);
    mraa_pwm_write(speed_pwm_in1, 1.0f);
    mraa_pwm_write(speed_pwm_in2, 1.0f);
    cout<< "Initialization Complete"<<endl;
    cout<< "Press Enter to continue."; 
    char ch = getchar();
    
    IR_device *ir = new IR_device(12, "recv");
    

/*
    Coordinate try1, try2, try3, try4, try5;
    try1.x = 0;
    try1.y = 0;
    try2.x = 60;
    try2.y = 60;
    try3.x = 140;
    try3.y = 0;
    try4.x = 60;
    try4.y = -60;
    try5.x = 0;
    try5.y = 0;
*/
    car = new CarSys(140, 0);
    thread t1 = car->threading();
    t1.join();
    running = true;
    thread t2(car_report, car);
    vector<Coordinate> path = car->getPath();
    path.push_back(Coordinate(140, 0));
    double newOffset = PI / 2;
    int n = path.size();
    for (int cur = 1; cur < n; ++cur)
      newOffset =  caculateMotionTime(path[cur - 1], path[cur], newOffset, ir);

    running = false;
    delete ir;
    t2.join();
    delete car;
}
