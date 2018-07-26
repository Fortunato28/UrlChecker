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

    int startPolling();

private:
    vector<UrlWrapper*> urlObjects;
    string outFile;

    int pollingResult();
};

#endif // CONTROLLER_H
