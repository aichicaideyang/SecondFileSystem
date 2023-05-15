#ifndef COMMON_H
#define COMMON_H
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <pthread.h>
#include <cstdlib>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <map>
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#define PORT 1234
#define BACKLOG 128
#define NOERROR 0
using namespace std;
typedef int ErrorCode;
const int BUFSIZE = 1024;
// 文件开头位置
#define SYS_SEEK_SET 0
// 文件指针当前位置
#define SYS_SEEK_CUR 1
// 文件结尾位置
#define SYS_SEEK_END 2
// 文件描述符类型
typedef unsigned int FD;
void sys_log(string info);
void sys_err(string info);
bool isNumber(const string &str);
void handle_pipe(int sig);
#endif
