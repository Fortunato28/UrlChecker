#ifndef URLWRAPPER_H
#define URLWRAPPER_H

#include <string>
#include <vector>
#include <iostream>
#include <url_parser.hpp>

using std::cout;
using std::endl;

class UrlWrapper
{
public:
    UrlWrapper(std::string gettedUrl, int n, int t);
    int serverPolling(); // Опрос сервера
    std::string getResult();

private:
    const Url url;
    const int requestsNumber; // Количество запросов
    const int delay;          // Задержка перед запросами

    std::vector<int> responseTime;
    int noResponse;     // Количество неотработавших запросов

    int sendAll(int sock, const std::string &message);

    int getAverage();
    int getMax();
    int getMin();
};

#endif // URLWRAPPER_H
