/******************************************************************************
     * File: controller.hpp
     * Description: Файл представляет собой интерфейс класса Controller,
     *              который осуществляет многопоточный опрос серверов и вывод
     *              результатов.
     * Created: июль 2019
     * Author: Сапунов Антон
     * Email: fort.sav.28@gmail.com
******************************************************************************/
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "url_wrapper.hpp"

using std::string;
using std::vector;

class Controller
{
public:
    Controller(const vector<string> &urls, int n, int t, string &outFile);
    ~Controller();

    int startPolling();                     // Опрос серверов и управление url-обёртками

private:
    vector<UrlWrapper*> urlObjects;         // Объекты url-обёртки, осуществляющие опрос
    string outFile;

    int pollingResult();                    // Вывод результата
};

#endif // CONTROLLER_H
