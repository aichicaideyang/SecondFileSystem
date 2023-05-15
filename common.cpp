#include "common.h"
void sys_log(string info)
{
    cout << "[INFO]:" << info << "\n";
}
void sys_err(std::string info)
{
    cout << "[ERROR]:" << info << "\n";
}
void handle_pipe(int sig)
{
}

bool isNumber(const string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}