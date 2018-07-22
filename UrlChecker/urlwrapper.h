#ifndef URLWRAPPER_H
#define URLWRAPPER_H

#include <string>
#include <vector>
#include <iostream>

using std::cout;
using std::endl;

class UrlWrapper
{
public:
    UrlWrapper(std::string gettedUrl, int n, int t);
    std::string getResult();
private:
    const std::string url;
    const int requestsNumber; // Количество запросов
    const int delay;          // Задержка перед запросами

    std::vector<int> responseTime;
    int noResponse;     // Количество неотработавших запросов

    int sendAll(int sock, const std::string &message);
    int recvAll(int sock, std::string *gettedMessage);
    int serverPolling(); // Опрос сервера

    int getAverage();
    int getMax();
    int getMin();
};

#endif // URLWRAPPER_H
