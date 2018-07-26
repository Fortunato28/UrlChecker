#include "controller.hpp"
#include <thread>

Controller::Controller(const vector<string> &urls, int n, int t)
{
    // Заполнение вектора объектов-ссылочных обёрток
    for(auto &url : urls)
    {
        UrlWrapper *urlInstance = new UrlWrapper(url, n, t);
        urlOjects.push_back(urlInstance);
    }
}

int Controller::startPolling()
{
    vector<std::thread> threads;
    for(auto &urlInstance: urlOjects)
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

    return 0;
}

Controller::~Controller()
{
    // Освобождаем выделенную память для каждого объекта-обёртки
    for(auto &urlInstance: urlOjects)
    {
        delete urlInstance;         // Это указатель
    }
}
