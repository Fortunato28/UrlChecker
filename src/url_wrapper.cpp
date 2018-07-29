/******************************************************************************
     * File: url_wrapper.cpp
     * Description: Файл представляет собой реализацию класса UrlWrapper,
     *              объекты которого опрашивают конкретный сервера и собирают
     *              результаты об опросе.
     * Created: июль 2019
     * Author: Сапунов Антон
     * Email: fort.sav.28@gmail.com
******************************************************************************/
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

using std::chrono::high_resolution_clock;

UrlWrapper::UrlWrapper(std::string gettedUrl, int n, int t) :
    url(gettedUrl), requestsNumber(n), delay(t)
{
    httpRequest = "HEAD " + url.path() +" HTTP/1.1\r\nHost: "
                             + url.host() + "\r\nConnection: close\r\n\r\n";
    noResponse = 0;
    responseTime.clear();
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
        fprintf(stderr, "getaddrinfo error: %s, check %s to correctness\n", gai_strerror(status), url.str().c_str());
//        exit(0); // DNS-resolv на одном из параметров не сработал
        return -1;
    }

    sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if(sock < 0)
    {
        std::perror("Socket error");
        return 1;
    }

    // Делаем неблокирующим
    if(fcntl(sock, F_SETFL, O_NONBLOCK) != 0)
    {
        perror("Fctnl couldn`t set socket nonblocking");
        return 1;
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
        return 1;
    }

    // Работа с соединением на неблокирующем сокете
    fd_set readSet, writeSet;
    timeval timeout;
    socklen_t len;
    int error;

    // Данные для select`a
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);
    writeSet = readSet;
    timeout.tv_sec = delay;
    timeout.tv_usec = 0;

    if((ress = select(sock + 1, &readSet, &writeSet, NULL, &timeout)) == 0)
    {
        perror("Timeout:");
        close(sock); // timeout
        errno = ETIMEDOUT;
        return 1;
    }

    if(FD_ISSET(sock, &readSet) || FD_ISSET(sock, &writeSet))
    {
        len = sizeof(error);
        if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            perror("getsockopt error:");
            return 1;
        }
    }
    else
    {
        perror("Select error << socket not set");
        return 1;
    }

    return 0;
}

int UrlWrapper::sendFullMessage()
{
   int totalSended = 0;
   int sended = 0;
   int messageLength = httpRequest.length();

   // Отправляем, пока не улетит всё сообщение
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
        std::cout << "Please wait..." << std::endl;         // Чтобы было понятно, что программа работает

        int initSock = initSocket();
        if(initSock == -1)      // Ошибка DNS-resolv
        {
            // Возврат с ошибкой
            return 1;
        }
        if(initSock)        // Какая-то ошибка, давайте подождём попробуем ещё раз
        {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            continue;
        }

        if(tcpConnect())        // Какая-то ошибка, давайте подождём попробуем ещё раз
        {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            continue;
        }

        high_resolution_clock::time_point timeBefore = high_resolution_clock::now();
        int sended = sendFullMessage();

        // Получаем
        char buf[1];    // Больше байта и не нужно

        // Данные для select`a
        fd_set readSet, writeSet;
        timeval timeout;
        timeout.tv_sec = delay;
        timeout.tv_usec = 0;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);

        if(select(sock + 1, &readSet, NULL, NULL, &timeout) <= 0)
        {
            // Если ошибка здесь, то это локальная, сервер ни при чём
            perror("Select:");
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            continue;
        }

        // Если сокет доступен для чтения
        if(FD_ISSET(sock, &readSet))
        {
            long int getted = recv(sock, buf, 1, 0);
            if(getted == -1)
            {
                perror("Recv error:");
                ++noResponse;       // Что-то плохое произошло во время получения ответа
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                continue;
            }
        }

        // Если сокет доступен
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);
        writeSet = readSet;

        if(select(sock + 1, &readSet, &writeSet, NULL, &timeout) <= 0)
        {
            // Если ошибка здесь, то это локальная, сервер ни чём
            perror("Select:");
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            continue;
        }

        // Когда уже приняли, возьмём второе время
        high_resolution_clock::time_point timeAfter;
        if(FD_ISSET(sock, &readSet) || FD_ISSET(sock, &writeSet))
        {
             timeAfter = high_resolution_clock::now();
        }

        // Время отклика сервера в секундах
        std::chrono::duration<double> timeDiff = std::chrono::duration_cast<std::chrono::duration<double>>(timeAfter - timeBefore);
        responseTime.push_back(timeDiff.count() * 1000);

        // Освобождаем использованное
        close(sock);
        freeaddrinfo(serverInfo);
        // Защита от лишнего ожидания после последней итерации
        if(i < (requestsNumber - 1))
            std::this_thread::sleep_for(std::chrono::seconds(delay));
    }

    return 0;
}

std::string UrlWrapper::getResult()
{
    std::string result;
    // Где-то произошли неисправимые ошибки
    if(responseTime.empty())
    {
        std::cout << url.str() + " " + "Can not be polled" << std::endl;
        return url.str() + " " + "Can not be polled";
    }
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
