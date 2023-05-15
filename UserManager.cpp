#include "UserManager.h"
#include "Kernel.h"
UserManager::UserManager()
{
    // 初始化操作
    for (int i = 0; i < USER_N; ++i)
    {
        (this->pusers)[i] = NULL;
    }
    this->user_addr.clear();
    pthread_t pid = pthread_self();           // 获取当前线程的ID号
    pusers[0] = (User *)malloc(sizeof(User)); // 为当前线程申请一个User结构
    user_addr[pid] = 0;                       // 建立映射关系
}
UserManager::~UserManager()
{
    // 主要完成对User结构的释放
    for (int i = 0; i < USER_N; ++i)
    {
        if ((this->pusers)[i] != NULL)
            free((this->pusers)[i]);
    }
}

bool UserManager::Login(string uname)
{
    pthread_t pthread_id = pthread_self();
    if (user_addr.find(pthread_id) != user_addr.end())
    {
        sys_log("Current thread is already online!");
        return false;
    }
    // 为当前线程分配一个User结构，那么首先需要找到一个空闲的User数组的下标
    int i;
    // 采用简单遍历的方式，因为一共只允许100个用户，所以这里的效率并不会太差
    // 如果说要支持更多的用户，那么可以考虑使用队列管理的方式，也就是把空闲的串起来之际额使用即可
    for (i = 0; i < USER_N; ++i)
    {
        if (pusers[i] == NULL)
        {
            break;
        }
    }
    if (i == USER_N)
    {
        // 找不到
        sys_err("Can't find free User!");
        return false;
    }

    // 这里找到的i就是User数组中存放当前线程user结构的位置
    pusers[i] = (User *)malloc(sizeof(User));
    if (pusers[i] == NULL)
    {
        sys_err("Alloc Memory for current thread failed!");
        return false;
    }
    user_addr[pthread_id] = i; // 建立映射关系
    pusers[i]->u_uid = 0;      // 初始的有效用户ID设置为root
    sys_log("Login Successfully!");
    // 初始化User结构
    // 关联根目录
    pusers[i]->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
    pusers[i]->u_cdir->NFrele();
    strcpy(pusers[i]->u_curdir, "/");
    // 创建家目录
    Kernel::Instance().FcreateDir(uname);
    // 转到家目录
    pusers[i]->u_error = NOERROR;
    char dirname[512] = {0};
    strcpy(dirname, uname.c_str());
    pusers[i]->u_dirp = dirname;
    pusers[i]->u_arg[0] = (unsigned long long)(dirname);
    FileManager &fimanag = Kernel::Instance().GetFileManager();
    fimanag.ChDir();
    return true;
}
bool UserManager::Logout()
{
    // 保存最近的更新
    Kernel::Instance().Quit();
    pthread_t pthread_id = pthread_self();
    // 判断当前线程是否是合法状态
    if (user_addr.find(pthread_id) == user_addr.end())
    {
        sys_err("Current thread is no need to log out!");
        return false;
    }
    int i = user_addr[pthread_id];
    // 释放当前线程的user结构
    free(pusers[i]);
    // 去除掉当前线程标号
    user_addr.erase(pthread_id);
    sys_log("Current thread logout successfully!");
    return true;
}

User *UserManager::GetUser()
{
    // 通过两个重要的字段来获得当前线程的User结构
    pthread_t pthread_id = pthread_self();
    if (user_addr.find(pthread_id) == user_addr.end())
    {
        sys_err("Can't find the User of current thread!");
        exit(1);
    }
    return pusers[user_addr[pthread_id]];
}
