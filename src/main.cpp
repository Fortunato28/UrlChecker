/******************************************************************************
     * File: main.cpp
     * Description: Файл обрабатывает входные параметры и вызывает объект Controller`a
     * Created: июль 2019
     * Author: Сапунов Антон
     * Email: fort.sav.28@gmail.com
******************************************************************************/
#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>
#include "url_wrapper.hpp"
#include "controller.hpp"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
    int n; // Number of connect-attempts
    int t; // Delay before next connect
    string inputFile;
    string outputFile;
    vector<string> urls;

    try
    {
        // Парсинг аргументов командной строки
        po::options_description desc("Options");
        desc.add_options()
                ("help,h", "Display this help and exit")
                ("input,i", po::value<string>(&inputFile), "File conteining urls")
                ("number,n", po::value<int>(&n)->default_value(3), "Number of connection attempts")
                ("time-wait,t", po::value<int>(&t)->default_value(5), "Delau before next connect")
                ("output,o", po::value<string>(&outputFile)->default_value("output.txt"), "File with report")
                ;
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc) , vm);
        po::notify(vm);

        // Подсказка
        if(vm.count("help"))
        {
            cout << "Usage: " << argv[0] << " -i FILE -n NUMBER -t DELAY -o OUTPUT" << endl;
            cout << desc << endl;
            return 1;
        }
        // Отсутствие обязательного параметра
        if(!vm.count("input"))
        {
            cout<< "There is no input file" << endl;
            return 1;
        }

        // Парсинг конфигурационного файла
        po::options_description fileDesc("Options in config file");
        fileDesc.add_options()
                ("url", po::value<vector<string> >(&urls), "url that be checked")
                ;

        // А конфигурационного файла-то и нет
        std::fstream configFile(inputFile);
        if(!configFile.is_open())
        {
            cout << "Can`t open config file" << endl;
            return 1;
        }
        else
        {
            po::store(po::parse_config_file(configFile, fileDesc), vm);
            po::notify(vm);
        }

    }
    catch(std::exception &e)
    {
        cout << e.what() << endl;
        return 1;
    }

    // Проверка логики опроса конкретного сайта (без многопоточности)
//    UrlWrapper a(urls.at(5), n, t);
//    a.serverPolling();

    Controller controller(urls, n, t, outputFile);
    controller.startPolling();

    return 0;
}
