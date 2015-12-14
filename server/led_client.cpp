#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
using namespace std;

string hostname = "localhost", port = "1222";

class LEDSys {
public:
  LEDSys() {
    strcpy(idxInfo, "1");
  }

  void sendInfo() {
    int status;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof host_info);

    cout << "Setting up the structs..."  << std::endl;

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname.c_str(), port.c_str(), &host_info, &host_info_list);
    if (status != 0)
      cout << "Getaddrinfo error" << gai_strerror(status);
    else {
      for (struct addrinfo *cur = host_info_list; cur; cur = cur->ai_next) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)cur->ai_addr;
        printf("Hostname: %s\n", inet_ntoa(saddr->sin_addr));
      }
    }

    cout << "Creating a socket..."  << std::endl;
    //host_info_list = host_info_list->ai_next;
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socketfd == -1)
      cout << "Socket error ";

    cout << "Connecting..."  << std::endl;
    status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
      cout << "Connect error";

    cout << "Sending message..."  << std::endl;
    char msg[20];
    fill_n(msg, 20, 0);
    generateInfo(msg);
    int len = strlen(msg);
    msg[len] = '\n';
    printf("%s", msg);
    ssize_t bytes_sent;
    bytes_sent = send(socketfd, msg, len + 1, 0);

    freeaddrinfo(host_info_list);
    close(socketfd);
  }

private:
  char idxInfo[2];

  void getPos(char *posInfo) {
    //need the API for getting the current position of the car
    //after getting the position, just create a string with the format "x,y"
    //and use strcpy to copy it into posInfo
    strcpy(posInfo, "-80, 0");
  }

  void generateInfo(char *msg) {
    strcpy(msg, idxInfo);
    int cur = strlen(msg);
    msg[cur] = '#';
    char posInfo[10];
    getPos(posInfo);
    strcpy(msg + cur + 1, posInfo);
  }
};

int main() {
  LEDSys led;
  led.sendInfo();
  return 0;
}
