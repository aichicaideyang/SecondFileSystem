#include "Kernel.h"
#include "common.h"
#include "server_supprot.h"

int main()
{
    int listen_fd, client_fd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct sigaction action;
    int sin_size;
    init_user_table();                                              // 初始化用户表
    init_help_info();                                               // 初始化一下帮助信息
    Kernel::Instance().Initialize();                                // 初始化文件系统Kernel实例
    initialize_listen(action, listen_fd, sin_size, server, client); // 初始化监听操作
    sys_log("Wait for user to connect...");
    while (1)
    {
        if ((client_fd = accept(listen_fd, (struct sockaddr *)&client, (socklen_t *)&sin_size)) == -1)
        {
            sys_err("Failed to accept connection from client!");
            continue;
        }
        pthread_t cthread;
        cout << "Client : " << inet_ntoa(client.sin_addr) << " connected!"
             << "\n";
        // 为每个用户开启一个线程进行交互功能
        pthread_create(&cthread, NULL, client_thread, (void *)&client_fd);
    }
    close(listen_fd);
    return 0;
}
