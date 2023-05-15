#ifndef KERNEL_H
#define KERNEL_H
#include "common.h"
#include "Machine.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "User.h"
#include "UserManager.h"
class Kernel
{
public:
    Kernel();
    ~Kernel();
    static Kernel &Instance();
    void Initialize();
    void Quit();
    Machine &GetMachine();
    BufferManager &GetBufferManager();
    FileSystem &GetFileSystem();
    FileManager &GetFileManager();
    User &GetSuperUser();
    User &GetUser();
    UserManager &GetUserManager();

    string Fcd(string &file_path);
    string Fls();
    string Fcat(string &file_path);
    string Fin(string &infile_path, string &outfile_path);
    string Fout(string &infile_path, string &outfile_path);
    int Fopen(string &file_path, int mode);
    int Fclose(FD fd);
    int Fread(FD fd, int size, void *data);
    int Fwrite(FD fd, int size, void *data);
    int Fseek(FD fd, int offset, int whence);
    int FcreateDir(string &file_path);
    int Fcreate(string &file_path);
    int Fdelete(string &file_path);

private:
    // Kernel子组件的初始化函数
    void InitMachine();
    void InitFileSystem();
    void InitBuffer();
    void InitUser();

private:
    static Kernel instance; // 单体实例
    const int DEFAULT_MODE = 040755;
    Machine *m_Machine;
    BufferManager *m_BufferManager;
    FileSystem *m_FileSystem;
    FileManager *m_FileManager;
    User *m_User;
    UserManager *m_UserManager;
};
#endif
