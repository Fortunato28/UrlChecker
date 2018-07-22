#include "urlwrapper.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>

//172.217.16.164

using std::chrono::high_resolution_clock;

UrlWrapper::UrlWrapper(std::string gettedUrl, int n, int t) :
    url(gettedUrl), requestsNumber(n), delay(t)
{
    serverPolling();
}

int UrlWrapper::sendAll(int sock, const std::string &message)
{
   size_t totalSended = 0;
   size_t sended;
   size_t messageLength = message.length();

   while(totalSended < messageLength)
   {
       sended = send(sock, message.c_str() + totalSended, messageLength - totalSended, 0);
       if(sended < 0)
       {
           break;
       }
       totalSended += sended;
   }

   return sended < 0 ? -1 : totalSended;
}

int UrlWrapper::recvAll(int sock, std::string *gettedMessage)       // Скорее всего удалить
{
    size_t getted;
    char buf[500];

    while(getted != 0)
    {
        getted = recv(sock, buf, 1, MSG_WAITALL);
    }

    *gettedMessage = buf;
}

int UrlWrapper::serverPolling()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);             // Сделать неблокирующим
    if(sock < 0)
    {
        std::perror("Socket error");
        exit(1);
    }

    std::string googleCom = "172.217.16.164";
    struct in_addr *inAddr;

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    addr.sin_addr.s_addr = htonl(0xACD910A4);       // Адрес гугла, прикрутить получение по DNS
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::perror("Connect error");
        exit(1);
    }

    std::string getRequest = "HEAD / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n"; // Парсинг урл? + нужная конкатенация
    high_resolution_clock::time_point timeBefore = high_resolution_clock::now();
    size_t sended = sendAll(sock, getRequest);

    std::string gettedMessage;
    char buf[1];
    size_t getted = recv(sock, buf, 1, 0);       // Нужны ли вообще получаемые данные?

    high_resolution_clock::time_point timeAfter = high_resolution_clock::now();

    close(sock);

    // Время отклика сервера
    std::chrono::duration<double> timeDiff = std::chrono::duration_cast<std::chrono::duration<double>>(timeAfter - timeBefore);

    cout << getted << endl;
    cout << buf << endl;
    cout << timeDiff.count() << endl;

    return 0;
}
