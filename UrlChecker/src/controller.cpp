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
    vector<std::thread> threads;
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
    for(auto &urlInstance: urlObjects)
    {
        resultToOutput = urlInstance->getResult() + "\n";
        output << resultToOutput;
//        std::cout << resultToOutput << std::endl;
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
