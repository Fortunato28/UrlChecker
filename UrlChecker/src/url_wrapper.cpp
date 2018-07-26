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
#include <thread>
#include <algorithm>


using std::cout;
using std::endl;

using std::chrono::high_resolution_clock;

UrlWrapper::UrlWrapper(std::string gettedUrl, int n, int t) :
    url(gettedUrl), requestsNumber(n), delay(t)
{
    httpRequest = "HEAD " + url.path() +" HTTP/1.1\r\nHost: "
                             + url.host() + "\r\nConnection: close\r\n\r\n";
//    initSocket();
}

int UrlWrapper::initSocket()
{
    // Новые игрища с DNS и структурой данных сервера
    int status;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;                // Хоть IPv4, хоть IPv6
    hints.ai_socktype = SOCK_STREAM;            // TCP-соединение

    // С https сервера сразу разрывают соединение.
    // Думаю потому, что нужно устанавливать защищённое соединение сначала.
    // Впрочем, на работу программы это не влияет, ведь сервера всё же отвечают.
    if((status = getaddrinfo(url.host().c_str(), url.scheme().c_str(), &hints, &serverInfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
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

    return 0;
}

int UrlWrapper::tcpConnect()
{
    // Установка tcp-соединения
    int ress = connect(sock, serverInfo->ai_addr, serverInfo->ai_addrlen) ;
    if(ress < 0 && errno != EINPROGRESS)
    {
        perror("Connect:");
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
    timeout.tv_sec = delay;
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
        exit(0);
    }

}

int UrlWrapper::sendFullMessage()
{
   int totalSended = 0;
   int sended = 0;
   int messageLength = httpRequest.length();

   while(totalSended < messageLength)
   {
       sended = send(sock, httpRequest.c_str() + totalSended, messageLength - totalSended, 0);
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
    for(int i = 0; i < requestsNumber; ++i)
    {
        initSocket();
        tcpConnect();

        fd_set readSet, writeSet;
        timeval timeout;
        timeout.tv_sec = delay;
        timeout.tv_usec = 0;

        high_resolution_clock::time_point timeBefore = high_resolution_clock::now();
        int sended = sendFullMessage();

        // Получаем
        char buf[1024];    // Больше байта и не нужно

        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);

        if(select(sock + 1, &readSet, NULL, NULL, &timeout) <= 0)
        {
            perror("Select here");
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

        // Время отклика сервера в секундах
        std::chrono::duration<double> timeDiff = std::chrono::duration_cast<std::chrono::duration<double>>(timeAfter - timeBefore);
        responseTime.push_back(timeDiff.count() * 1000);

        close(sock);
        // Защита от лишнего ожидания после последней итерации
        if(i < (requestsNumber - 1))
            std::this_thread::sleep_for(std::chrono::seconds(delay));
    }

    return 0;
}

std::string UrlWrapper::getResult()
{
    std::string result;
    result += url.str() + " " +
            std::to_string(getMax()) + "/" +
            std::to_string(getAverage()) + "/" +
            std::to_string(getMin()) + " " +
            std::to_string(noResponse);

    return result;
}

int UrlWrapper::getMax()
{
    return *std::max_element(responseTime.begin(), responseTime.end());
}

int UrlWrapper::getAverage()
{
    int result = 0;

    for(auto &time: responseTime)
    {
        result += time;
    }

    result /= responseTime.size();

    return result;
}

int UrlWrapper::getMin()
{
    return *std::min_element(responseTime.begin(), responseTime.end());
}

UrlWrapper::~UrlWrapper()
{
   freeaddrinfo(serverInfo);
}
