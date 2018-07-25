#include "url_wrapper.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <url_parser.hpp>
#include <netdb.h>
#include <fcntl.h>

using std::cout;
using std::endl;

using std::chrono::high_resolution_clock;

UrlWrapper::UrlWrapper(std::string gettedUrl, int n, int t) :
    url(gettedUrl), requestsNumber(n), delay(t)
{
    httpRequest = "HEAD " + url.path() +" HTTP/1.1\r\nHost: "
                             + url.host() + "\r\nConnection: close\r\n\r\n";
}

int UrlWrapper::sendAll(int sock, const std::string &message)
{
   int totalSended = 0;
   int sended = 0;
   int messageLength = message.length();

   while(totalSended < messageLength)
   {
       sended = send(sock, message.c_str() + totalSended, messageLength - totalSended, 0);
       if(sended == -1)
       {
           break;
       }
       totalSended += sended;
   }

   return sended < 0 ? -1 : totalSended;
}

int UrlWrapper::serverPolling()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        std::perror("Socket error");
        exit(1);
    }

    // Делаем неблокирующим
    if(fcntl(sock, F_SETFL, O_NONBLOCK) != 0)
    {
        perror("Fctnl couldn`t set socket nonblocking");
        exit(5);
    }

    // DNS-игрища
    struct hostent *siteData = gethostbyname(url.host().c_str());
    if(siteData == nullptr)
    {
        herror("Wrong DNS resolve");
        exit(3);
    }

    // Структура с данными сервера
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    memcpy(&addr.sin_addr.s_addr, siteData->h_addr, siteData->h_length); // Это такая запись IP в нужную структуру

    // Установка tcp-соединения
    int ress = connect(sock, (struct sockaddr *)&addr, sizeof(addr)) ;
    if(ress < 0 && errno != EINPROGRESS)
    {
        std::perror("Connect error");
        exit(1);
    }

    // Работка с соединением на неблокирующем сокете
    fd_set readSet, writeSet;
    timeval timeout;
    socklen_t len;
    int error;

    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);
    writeSet = readSet;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if((ress = select(sock + 1, &readSet, &writeSet, NULL, &timeout)) == 0)
    {
        close(sock); // timeout
        errno = ETIMEDOUT;
        return(-1);
    }

    if(FD_ISSET(sock, &readSet) || FD_ISSET(sock, &writeSet))
    {
        len = sizeof(error);
        if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return(-1);
    }
    else
    {
        perror("Select error << socket not set");
    }


    high_resolution_clock::time_point timeBefore = high_resolution_clock::now();
    int sended = sendAll(sock, httpRequest);

    cout << sended << endl;

    // Получаем
    char buf[1024];    // Больше байта и не нужно

    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    if(select(sock + 1, &readSet, NULL, NULL, &timeout) <= 0)
    {
        perror("Select");
        exit(4);
    }

    if(FD_ISSET(sock, &readSet))
    {
        long int getted = recv(sock, buf, 1024, 0);
    }


    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);
    writeSet = readSet;

    if(select(sock + 1, &readSet, &writeSet, NULL, &timeout) <= 0)
    {
        perror("Select");
        exit(4);
    }

    high_resolution_clock::time_point timeAfter;
    if(FD_ISSET(sock, &readSet) || FD_ISSET(sock, &writeSet))
    {
         timeAfter = high_resolution_clock::now();
    }

    close(sock);

    // Время отклика сервера в секундах
    std::chrono::duration<double> timeDiff = std::chrono::duration_cast<std::chrono::duration<double>>(timeAfter - timeBefore);

    cout << httpRequest << endl;
    cout << buf << endl;
    cout << timeDiff.count() << endl;
    cout << url.str() << endl;
    cout << url.host() << endl;
    cout << url.path() << endl;

    return 0;
}
