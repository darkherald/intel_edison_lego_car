#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <map>
using namespace std;

string hostname = "172.20.10.4", port = "1222";

class CarSys {
public:
  CarSys() {
    strcpy(idxInfo, "c");
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
        printf("Hostname: %s\n", inet_ntoa(saddr->sin_addr));
      }
    }

    std::cout << "Creating a socket..."  << std::endl;
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socketfd == -1)
      std::cout << "Socket error";

    std::cout << "Connecting..."  << std::endl;
    status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
      std::cout << "Connect error";

    std::cout << "Sending message..."  << std::endl;
    char msg[50];
    generateInfo(msg);
    int len = strlen(msg);
    msg[len] = '\n';
    printf("%s", msg);
    ssize_t bytes_sent;
    bytes_sent = send(socketfd, msg, len + 1, 0);

    std::cout << "Waiting to receive data..."  << std::endl;
    ssize_t bytes_received;
    char incoming_data_buf[1000];
    fill_n(incoming_data_buf, 1000, 0);
    bytes_received = recv(socketfd, incoming_data_buf, 1000, 0);
    cout << bytes_received << " bytes received :" << endl;
    incoming_data_buf[bytes_received] = '\0';
    cout << incoming_data_buf << endl;
    updatePos(incoming_data_buf);

    std::cout << "Receiving complete. Closing socket..." << std::endl;

    freeaddrinfo(host_info_list);
    close(socketfd);
  }

private:
  char idxInfo[2];

  map<string, pair<int, int> > pos;

  void getPos(char *posInfo) {
    //need the API for getting the current position of the car
    //after getting the position, just create a string with the format "x,y"
    //and use strcpy to copy it into posInfo
    strcpy(posInfo, "11,12");
  }

  void generateInfo(char *msg) {
    fill_n(msg, 50, 0);
    strcpy(msg, idxInfo);
    int cur = strlen(msg);
    msg[cur] = '#';
    char posInfo[10];
    getPos(posInfo);
    strcpy(msg + cur + 1, posInfo);
    cur = strlen(msg);
    strcpy(msg + cur, "#Testing...");
  }

  void updatePos(char *info) {
    while (*info) {
      int len = 0;
      while (*(info + len) != '\n')
        ++len;
      string infoStr(info, len);
      int splitPos = infoStr.find('#');
      string idx = infoStr.substr(0, splitPos), coord = infoStr.substr(splitPos + 1);
      splitPos = coord.find(',');
      pos[idx] = make_pair(stoi(coord.substr(0, splitPos)), stoi(coord.substr(splitPos + 1)));
      cout << "Updated LED " << idx << " to (" << pos[idx].first << ',' << pos[idx].second << ')' << endl;
      info += len + 1;
    }
  }
};

int main() {
  CarSys car;
  car.transaction();
  return 0;
}
