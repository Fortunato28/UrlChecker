#ifndef URLWRAPPER_H
#define URLWRAPPER_H

#include <string>
#include <vector>
#include <iostream>
#include <url_parser.hpp>

class UrlWrapper
{
public:
    UrlWrapper(std::string gettedUrl, int n, int t);
    ~UrlWrapper();
    int serverPolling(); // Опрос сервера
    std::string getResult();

private:
    const Url url;
    std::string httpRequest;
    const int requestsNumber;       // Количество запросов
    const int delay;                // Задержка перед запросами
    int sock;                       // Сокет, через который идёт работа с сетью
    struct addrinfo *serverInfo;    // Структура, хранящая данные о сервере

    int initSocket();
    int sendFullMessage();

    std::vector<int> responseTime;
    int noResponse;     // Количество неотработавших запросов

    int getAverage();
    int getMax();
    int getMin();
};

#endif // URLWRAPPER_H
