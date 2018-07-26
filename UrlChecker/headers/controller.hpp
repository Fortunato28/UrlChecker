#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "url_wrapper.hpp"

using std::string;
using std::vector;

class Controller
{
public:
    Controller(const vector<string> &urls, int n, int t);
    ~Controller();

    int startPolling();

private:
    vector<UrlWrapper*> urlOjects;
    int n;
    int t;
};

#endif // CONTROLLER_H
