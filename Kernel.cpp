#include "Kernel.h"
// 全局单体实例
Kernel Kernel::instance;

Machine g_Machine;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
FileManager g_FileManager;
User g_User;
UserManager g_UserManager;

char returnBuf[256];

// 构建与析构
Kernel::Kernel()
{
}
Kernel::~Kernel()
{
}

// 初始化函数：子组件和Kernel
void Kernel::InitMachine()
{
    this->m_Machine = &g_Machine;
    this->m_Machine->Initialize(); // 进行img的初始化和mmap
}
void Kernel::InitBuffer()
{
    this->m_BufferManager = &g_BufferManager;
    this->m_BufferManager->Initialize(); // 进行img的初始化和mmap
}
void Kernel::InitFileSystem()
{
    this->m_FileSystem = &g_FileSystem;
    this->m_FileSystem->Initialize();
    this->m_FileManager = &g_FileManager;
    this->m_FileManager->Initialize();
}
void Kernel::InitUser()
{
    this->m_User = &g_User;
    this->m_UserManager = &g_UserManager;
}

void Kernel::Initialize()
{
    InitBuffer();
    InitMachine(); // 包括初始化模拟磁盘
    InitFileSystem();
    InitUser();

    // 进入系统初始化
    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
    fileMgr.rootDirInode->i_flag &= (~Inode::ILOCK);
    pthread_mutex_unlock(&fileMgr.rootDirInode->mutex);

    Kernel::Instance().GetFileSystem().LoadSuperBlock();
    User &us = Kernel::Instance().GetUser();
    us.u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
    us.u_cdir->i_flag &= (~Inode::ILOCK);
    pthread_mutex_unlock(&us.u_cdir->mutex);
    strcpy(us.u_curdir, "/");
    sys_log("FileSystem Initialize Complete!");
}

void Kernel::Quit()
{
    this->m_BufferManager->Bflush();
    this->m_FileManager->m_InodeTable->UpdateInodeTable();
    this->m_FileSystem->Update();
    this->m_Machine->quit();
}

// 其他文件获取单体实例对象的接口函数
Kernel &Kernel::Instance()
{
    return Kernel::instance;
}

Machine &Kernel::GetMachine()
{
    return *(this->m_Machine);
}
BufferManager &Kernel::GetBufferManager()
{
    return *(this->m_BufferManager);
}
FileSystem &Kernel::GetFileSystem()
{
    return *(this->m_FileSystem);
}
FileManager &Kernel::GetFileManager()
{
    return *(this->m_FileManager);
}

User &Kernel::GetSuperUser()
{
    return *(this->m_User);
}

User &Kernel::GetUser()
{
    return *(this->m_UserManager->GetUser());
}

UserManager &Kernel::GetUserManager()
{
    return *(this->m_UserManager);
}
string Kernel::Fcd(string &file_path)
{
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    char dirname[300] = {0};
    strcpy(dirname, file_path.c_str());
    u.u_dirp = dirname;
    u.u_arg[0] = (unsigned long long)(dirname);
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.ChDir();
    return dirname;
}
string Kernel::Fls()
{
    string return_info = "";
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    string cur_path = u.u_curdir;
    FD fd = Kernel::Instance().Fopen(cur_path, (File::FREAD));
    return_info += "current path:";
    return_info += cur_path;
    return_info += "\n";
    char tmp_buf[BUFSIZE] = {0};
    while (1)
    {
        if (Kernel::Instance().Fread(fd, 32, tmp_buf) == 0)
            break;
        else
        {
            DirectoryEntry *mm = (DirectoryEntry *)tmp_buf;
            if (mm->m_ino == 0)
                continue;
            return_info += mm->m_name;
            return_info += "\n";
            memset(tmp_buf, 0, 32);
        }
    }
    Kernel::Instance().Fclose(fd);
    return return_info;
}
string Kernel::Fcat(string &file_path)
{
    string return_info = "";
    FD fd = Kernel::Instance().Fopen(file_path, 0x1);
    if (fd < 0)
    {
        return_info += "open file failure!\n";
        return return_info;
    }
    char tmp_buf[BUFSIZE + 1] = {0};
    while (true)
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int ret = Kernel::Instance().Fread(fd, BUFSIZE, tmp_buf);
        if (ret <= 0)
        {
            break;
        }
        return_info += tmp_buf;
    }
    return_info += "\n";
    Kernel::Instance().Fclose(fd);
    return return_info;
}
string Kernel::Fin(string &infile_path, string &outfile_path)
{
    // 打开外部文件
    string return_info = "";
    int outfiled = open(outfile_path.c_str(), O_RDONLY);
    if (outfiled < 0)
    {
        return_info += "Failed to open outfile\n";
        return return_info;
    }
    // 创建内部文件
    Kernel::Instance().Fcreate(infile_path);
    int infiled = Kernel::Instance().Fopen(infile_path, 0x1 | 0x2);
    if (infiled < 0)
    {
        close(outfiled);
        return_info += "Failed to open infile\n";
        return return_info;
    }
    // 开始拷贝，一次 1024 字节
    char tmp_buf[BUFSIZE + 1] = {0};
    int all_read_num = 0;
    int all_write_num = 0;
    while (true)
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int read_num = read(outfiled, tmp_buf, BUFSIZE);
        if (read_num <= 0)
        {
            break;
        }
        all_read_num += read_num;
        int write_num =
            Kernel::Instance().Fwrite(infiled, read_num, tmp_buf);
        if (write_num <= 0)
        {
            return_info += "Failed to write infile\n";
            return return_info;
        }
        all_write_num += write_num;
    }
    return_info += "Copy outFile Successfully!\n";
    close(outfiled);
    Kernel::Instance().Fclose(infiled);
    return return_info;
}
string Kernel::Fout(string &infile_path, string &outfile_path)
{
    string return_info = "";
    // 创建外部文件
    int outfiled = open(outfile_path.c_str(), O_WRONLY | O_TRUNC | O_CREAT); // 截断写入方式打开外部文件
    // 值得注意的是这里需要设置文件的读写权限，否则会打不开！
    if (chmod(outfile_path.c_str(), 0644) == -1)
    {
        return_info += "Failed to chmod outfile\n";
        return return_info;
    }
    if (outfiled < 0)
    {
        return_info += "Failed to open outfile\n";
        return return_info;
    }
    // 打开内部文件
    int infiled = Kernel::Instance().Fopen(infile_path, 0x1 | 0x2);
    if (infiled < 0)
    {
        close(outfiled);
        return_info += "Failed to open infile\n";
        return return_info;
    }
    // 开始拷贝，一次 1024 字节
    char tmp_buf[BUFSIZE + 1] = {0};
    int all_read_num = 0;
    int all_write_num = 0;
    // return_info += "get to copyout\n";
    /*cout << "get to copyout"
         << "\n";*/
    while (true)
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int read_num =
            Kernel::Instance().Fread(infiled, BUFSIZE, tmp_buf);
        // cout << "read num is " << read_num << "\n";
        if (read_num <= 0)
        {
            break;
        }
        // cout << tmp_buf << "\n";
        all_read_num += read_num;
        int write_num = write(outfiled, tmp_buf, read_num);
        if (write_num <= 0)
        {
            return_info += "Failed to write outfile\n";
            return return_info;
        }
        all_write_num += write_num;
    }
    return_info += "Copy inFile Successfully!\n";
    close(outfiled);
    Kernel::Instance().Fclose(infiled);
    return return_info;
}
int Kernel::Fopen(string &file_path, int mode)
{
    User &u = Kernel::Instance().GetUser();
    char path[256];
    strcpy(path, file_path.c_str());
    u.u_dirp = path;
    u.u_arg[1] = mode;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Open();
    return u.u_ar0[User::EAX];
}
int Kernel::Fclose(FD fd)
{
    User &u = Kernel::Instance().GetUser();
    u.u_arg[0] = fd;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Close();
    return u.u_ar0[User::EAX];
}
int Kernel::Fread(FD fd, int size, void *data)
{
    User &u = Kernel::Instance().GetUser();
    u.u_arg[0] = fd;
    u.u_arg[1] = (long long)data;
    u.u_arg[2] = size;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Read();
    return u.u_ar0[User::EAX];
}
int Kernel::Fwrite(FD fd, int size, void *data)
{
    User &u = Kernel::Instance().GetUser();
    u.u_arg[0] = fd;
    u.u_arg[1] = (unsigned long long)data;
    u.u_arg[2] = size;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Write();
    return u.u_ar0[User::EAX];
}
int Kernel::Fseek(FD fd, int offset, int whence)
{

    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    u.u_ar0[0] = 0;
    u.u_ar0[1] = 0;
    u.u_arg[0] = fd;
    u.u_arg[1] = offset;
    u.u_arg[2] = whence;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Seek();
    return u.u_ar0[User::EAX];
}
int Kernel::FcreateDir(string &file_path)
{
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    char filename_char[300] = {0};
    strcpy(filename_char, file_path.c_str());
    u.u_dirp = filename_char;
    u.u_arg[1] = DEFAULT_MODE;
    u.u_arg[2] = 0;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.MkNod();
    return u.u_ar0[User::EAX];
}
int Kernel::Fcreate(string &file_path)
{
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    char filename_char[512];
    strcpy(filename_char, file_path.c_str());
    u.u_dirp = filename_char;
    u.u_ar0[0] = 0;
    u.u_ar0[1] = 0;
    u.u_arg[1] = Inode::IRWXU;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.Creat();
    return u.u_ar0[User::EAX];
}
int Kernel::Fdelete(string &file_path)
{
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    u.u_ar0[0] = 0;
    u.u_ar0[1] = 0;
    char filename_char[512];
    strcpy(filename_char, file_path.c_str());
    u.u_dirp = filename_char;
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.UnLink();
    return u.u_ar0[User::EAX];
}