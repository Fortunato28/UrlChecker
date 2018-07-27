/******************************************************************************
     * File: controller.cpp
     * Description: Файл представляет собой реализацию функций класса Controller,
     *              который осуществляет многопоточный опрос серверов и вывод
     *              результатов.
     * Created: июль 2019
     * Author: Сапунов Антон
     * Email: fort.sav.28@gmail.com
******************************************************************************/

#include "controller.hpp"
#include <thread>
#include <fstream>

Controller::Controller(const vector<string> &urls, int n, int t, string &outFile):
    outFile(outFile)
{
    // Заполнение вектора объектов-ссылочных обёрток
    for(auto &url : urls)
    {
        UrlWrapper *urlInstance = new UrlWrapper(url, n, t);
        urlObjects.push_back(urlInstance);
    }
}

int Controller::startPolling()
{
    vector<std::thread> threads ;
    std::cout << "Start polling" << std::endl;
    for(auto &urlInstance: urlObjects)
    {
        // Без ref объект копируется, что крашит сокеты
        std::thread threadInstance(&UrlWrapper::serverPolling, std::ref(*urlInstance));
        threads.push_back(std::move(threadInstance));
    }

    for(auto &thread : threads)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }

    pollingResult();
    return 0;
}

int Controller::pollingResult()
{
    std::string resultToOutput;
    std::ofstream output(outFile);
    output << "url, max/avg/min (ms), bad _request\n";

    // Выходную информацию от каждого объекта пишем в файл
    for(auto &urlInstance: urlObjects)
    {
        resultToOutput = urlInstance->getResult() + "\n";
        output << resultToOutput;
    }

    output.close();
    return 0;
}

Controller::~Controller()
{
    // Освобождаем выделенную память для каждого объекта-обёртки
    for(auto &urlInstance: urlObjects)
    {
        delete urlInstance;         // Это указатель
    }
}
