/******************************************************************************
     * File: url_wrapper.hpp
     * Description: Файл представляет собой описание класса UrlWrapper,
     *              реализующего опрос конкретного сервера и сбор результатов
     *              для него.
     * Created: июль 2019
     * Author: Сапунов Антон
     * Email: fort.sav.28@gmail.com
******************************************************************************/
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
    int serverPolling();            // Опрос сервера
    std::string getResult();

private:
    const Url url;
    std::string httpRequest;        // Запрос серверу
    const int requestsNumber;       // Количество запросов
    const int delay;                // Задержка перед запросами
    int sock;                       // Сокет, через который идёт работа с сетью
    struct addrinfo *serverInfo;    // Структура, хранящая данные о сервере

    int initSocket();
    int tcpConnect();
    int sendFullMessage();          // Отправка полного сообщения с помощью send()

    std::vector<int> responseTime;  // Время всех ответов сервера (ms)
    int noResponse;                 // Количество неотработавших запросов

    int getAverage();
    int getMax();
    int getMin();
};

#endif // URLWRAPPER_H
