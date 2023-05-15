#ifndef USERMG_H
#define USERMG_H
#define NOERROR 0
#include "FileManager.h"
#include "User.h"
#include "File.h"
#include "INode.h"
#include "common.h"
class UserManager
{
public:
    static const int USER_N = 100; // 最多支持100个用户同时在线
    UserManager();
    ~UserManager();
    // 用户登录
    bool Login(string uname);
    // 用户登出
    bool Logout();
    // 得到当前线程的User结构
    User *GetUser();

public:
    map<pthread_t, int> user_addr; // 用来记录当前线程用户的ID号
    User *pusers[USER_N];          // 用来存储各个User结构
};

#endif
