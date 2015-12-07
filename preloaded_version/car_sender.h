#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <unordered_set>
#include <stdlib.h>
#include <utility>
#include <thread>
#include <algorithm>
#include <climits>
using namespace std;

class Coordinate {
public:
  double x;
  double y;
  Coordinate(double a, double b) {
    x = a;
    y = b;
  }

  Coordinate() {
    x = 0;
    y = 0;
  }
};

string hostname = "192.168.1.20", port = "1222";

void createAdjMat(const vector<Coordinate> &points, vector<vector<double> > &adjMat) {
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        double x1 = points[i].x, y1 = points[i].y;
        adjMat[i][i] = INT_MAX;
        for (int j = i + 1; j < n; ++j) {
            double x2 = points[j].x, y2 = points[j].y;
            double dx = x2 - x1, dy = y2 - y1;
            double dist = sqrt(dx * dx + dy * dy);
            adjMat[i][j] = adjMat[j][i] = dist;
        }
    }
}

double createGreedyPath(const vector<vector<double> > &adjMat, vector<int> &path) {
    int cur = 0;
    double len = 0, n = adjMat.size();
    unordered_set<int> visited;
    visited.insert(cur);
    path.push_back(cur);
    while (visited.size() < n) {
        double curMin = INT_MAX;
        int minPos = -1;
        for (int i = 0; i < n; ++i) {
            if (visited.count(i))
                continue;
            if (adjMat[cur][i] < curMin)
                curMin = adjMat[cur][i], minPos = i;
        }
        path.push_back(minPos);
        visited.insert(minPos);
        cur = minPos;
        len += curMin;
    }
    return len;
}

double twoOpt(const vector<vector<double> > &adjMat, vector<int> &path, double len) {
    double prevLen;
    int n = path.size();
    do {
        prevLen = len;
        for (int i = 1; i < n; ++i) {
            for (int j = i + 2; j < n; ++j) {
                double diff = adjMat[path[i - 1]][path[i]] + adjMat[path[j - 1]][path[j]] - adjMat[path[i - 1]][path[j - 1]] - adjMat[path[i]][path[j]];
                if (diff > 0) {
                    len -= diff;
                    reverse(next(path.begin(), i), next(path.begin(), j));
                }
            }
        }
    } while (prevLen > len);
    return len;
}

class CarSys {
public:
  CarSys(double x, double y) {
    strcpy(idxInfo, "c");
    updated = false;
    carPos.x = x;
    carPos.y = y;
  }

  void transaction() {
    int status;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof host_info);

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname.c_str(), port.c_str(), &host_info, &host_info_list);
    if (status != 0)
      std::cout << "Getaddrinfo error" << gai_strerror(status);
    else {
      for (struct addrinfo *cur = host_info_list; cur; cur = cur->ai_next) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)cur->ai_addr;
        //printf("Hostname: %s\n", inet_ntoa(saddr->sin_addr));
      }
    }

    //std::cout << "Creating a socket..."  << std::endl;
    //host_info_list = host_info_list->ai_next;
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socketfd == -1)
      std::cout << "Socket error";

    //std::cout << "Connecting..."  << std::endl;
    status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
      std::cout << "Connect error";

    //std::cout << "Sending message..."  << std::endl;
    char msg[50];
    generateInfo(msg);
    int len = strlen(msg);
    msg[len] = '\n';
    printf("%s", msg);
    ssize_t bytes_sent;
    bytes_sent = send(socketfd, msg, len + 1, 0);

    //std::cout << "Waiting to receive data..."  << std::endl;
    ssize_t bytes_received;
    char incoming_data_buf[1000];
    fill_n(incoming_data_buf, 1000, 0);
    bytes_received = recv(socketfd, incoming_data_buf, 1000, 0);
    //cout << bytes_received << " bytes received :" << endl;
    incoming_data_buf[bytes_received] = '\0';
    cout << incoming_data_buf << endl;
    if (!updated) {
      updatePos(incoming_data_buf);
      updated = true;
    }

    //std::cout << "Receiving complete. Closing socket..." << std::endl;

    freeaddrinfo(host_info_list);
    close(socketfd);
  }

  vector<Coordinate> getPath() {
    vector<Coordinate> points;
    points.push_back(carPos);
    for (auto e : pos)
      points.push_back(e.second);
    int n = points.size();
    vector<vector<double> > adjMat(n, vector<double>(n));
    createAdjMat(points, adjMat);
    vector<int> path;
    double greedyLen = createGreedyPath(adjMat, path);
    double optLen = twoOpt(adjMat, path, greedyLen);
    vector<Coordinate> result;
    for (int i : path)
      result.push_back(points[i]);
    return result;
  }

  void setCarPos(double x, double y) {
    carPos.x = x;
    carPos.y = y;
  }

  thread threading() {
    thread t(&CarSys::transaction, this);
    return t;
  }

private:
  char idxInfo[2];
  bool updated;
  Coordinate carPos;

  map<int, Coordinate> pos;

  void getPos(char *posInfo) {
    //need the API for getting the current position of the car
    //after getting the position, just create a string with the format "x,y"
    //and use strcpy to copy it into posInfo
    string posStr = to_string((int)carPos.x) + "," + to_string((int)carPos.y);
    strcpy(posInfo, posStr.c_str());
  }

  void generateInfo(char *msg) {
    fill_n(msg, 50, 0);
    strcpy(msg, idxInfo);
    int cur = strlen(msg);
    msg[cur] = '#';
    char posInfo[10];
    getPos(posInfo);
    strcpy(msg + cur + 1, posInfo);
  }

  void updatePos(char *info) {
    while (*info) {
      int len = 0;
      while (*(info + len) != '\n')
        ++len;
      string infoStr(info, len);
      int splitPos = infoStr.find('#');
      int idx = stoi(infoStr.substr(0, splitPos)); 
      string coord = infoStr.substr(splitPos + 1);
      splitPos = coord.find(',');
      pos[idx] = Coordinate(stod(coord.substr(0, splitPos)), stod(coord.substr(splitPos + 1)));
      cout << "Updated LED " << idx << " to (" << pos[idx].x << ',' << pos[idx].y << ')' << endl;
      info += len + 1;
    }
  }
};
