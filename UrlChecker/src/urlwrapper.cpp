#include "urlwrapper.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <url_parser.hpp>
#include <netdb.h>

//172.217.16.164

using std::chrono::high_resolution_clock;

UrlWrapper::UrlWrapper(std::string gettedUrl, int n, int t) :
    url(gettedUrl), requestsNumber(n), delay(t)
{
}

int UrlWrapper::sendAll(int sock, const std::string &message)
{
   int totalSended = 0;
   int sended = 0;
   int messageLength = message.length();

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

    struct hostent *siteData = gethostbyname(url.host().c_str());
    cout << url.str() << endl;
    if(siteData == nullptr)
    {
        herror("Wrong DNS resolve");
        exit(3);
    }

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    memcpy(&addr.sin_addr.s_addr, siteData->h_addr, siteData->h_length); // Это такая запись IP в нужную структуру
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::perror("Connect error");
        exit(1);
    }

    std::string request = "HEAD " + url.path() +" HTTP/1.1\r\nHost: "
                             + url.host() + "\r\nConnection: close\r\n\r\n";
    high_resolution_clock::time_point timeBefore = high_resolution_clock::now();
    size_t sended = sendAll(sock, request);

    std::string gettedMessage;
    char buf[1024];    // Больше байта и не нужно
    long int getted = recv(sock, buf, 1024, 0);

    high_resolution_clock::time_point timeAfter = high_resolution_clock::now();

    close(sock);

    // Время отклика сервера в секундах
    std::chrono::duration<double> timeDiff = std::chrono::duration_cast<std::chrono::duration<double>>(timeAfter - timeBefore);

    cout << request << endl;
    cout << buf << endl;
    cout << timeDiff.count() << endl;
    cout << url.str() << endl;
    cout << url.host() << endl;
    cout << url.path() << endl;

    return 0;
}
