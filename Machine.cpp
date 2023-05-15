#include "Machine.h"
#include "Kernel.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <iostream>
using namespace std;

Machine::Machine()
{
}
Machine::~Machine()
{
}
void Machine::Initialize()
{
    this->m_BufferManager = &Kernel::Instance().GetBufferManager();
    int fd = open(devpath, O_RDWR); // 打开镜像文件
    if (fd == -1)
    {
        fd = open(devpath, O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            sys_err("Machine Init Error!");
            exit(-1);
        }
        // 初始化镜像文件
        this->init_img(fd);
    }
    // 用mmap映射进来
    mmap_img(fd);
    this->img_fd = fd;
}

void Machine::quit()
{
    struct stat st; // 定义文件信息结构体
    /*取得文件大小*/
    int r = fstat(this->img_fd, &st);
    if (r == -1)
    {
        sys_err("Failt to get the infomation of img!");
        close(this->img_fd);
        exit(-1);
    }
    int len = st.st_size;
    // 将mmap内容刷回去
    msync((void *)(this->m_BufferManager->GetMmapAddr()), len, MS_SYNC);
}

void Machine::init_spb(SuperBlock &sb)
{
    // 初始化sqb
    sb.s_isize = FileSystem::INODE_ZONE_SIZE;
    sb.s_fsize = FileSystem::DATA_ZONE_END_SECTOR + 1;
    sb.s_nfree = (FileSystem::DATA_ZONE_SIZE - 99) % 100;
    int start_last_datablk = FileSystem::DATA_ZONE_START_SECTOR;
    while (true)
    {
        if ((start_last_datablk + 100 - 1) < FileSystem::DATA_ZONE_END_SECTOR)
            start_last_datablk += 100;
        else
            break;
    }
    start_last_datablk--;
    for (int i = 0; i < sb.s_nfree; ++i)
        sb.s_free[i] = start_last_datablk + i;
    sb.s_ninode = 100;
    for (int i = 0; i < sb.s_ninode; ++i)
        sb.s_inode[i] = i;
    sb.s_fmod = 0;
    sb.s_ronly = 0;
}

void Machine::init_db(char *data)
{
    struct
    {
        int nfree;
        int free[100];
    } tmp_table;

    int last_datablk_num = FileSystem::DATA_ZONE_SIZE;
    for (int i = 0;; i++)
    {
        if (last_datablk_num >= 100)
            tmp_table.nfree = 100;
        else
            tmp_table.nfree = last_datablk_num;
        last_datablk_num -= tmp_table.nfree;

        for (int j = 0; j < tmp_table.nfree; j++)
        {
            if (i == 0 && j == 0)
                tmp_table.free[j] = 0;
            else
            {
                tmp_table.free[j] = 100 * i + j + FileSystem::DATA_ZONE_START_SECTOR - 1;
            }
        }
        memcpy(&data[99 * 512 + i * 100 * 512], (void *)&tmp_table, sizeof(tmp_table));
        if (last_datablk_num == 0)
            break;
    }
}

void Machine::init_img(int fd)
{
    SuperBlock spb;
    init_spb(spb); // 初始化SPB
    DiskInode *di_table = new DiskInode[FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR];

    di_table[0].d_mode = Inode::IFDIR;
    di_table[0].d_mode |= Inode::IEXEC;

    char *datablock = new char[FileSystem::DATA_ZONE_SIZE * 512];
    memset(datablock, 0, FileSystem::DATA_ZONE_SIZE * 512);
    init_db(datablock); // 初始化DB

    write(fd, &spb, sizeof(SuperBlock));
    write(fd, di_table, FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR * sizeof(DiskInode));
    write(fd, datablock, FileSystem::DATA_ZONE_SIZE * 512);

    sys_log("Disk Initialize complete!");
}

void Machine::mmap_img(int fd)
{
    struct stat st;
    int r = fstat(fd, &st);
    if (r == -1)
    {
        sys_err("Failt to get the info of img!");
        close(fd);
        exit(-1);
    }
    int len = st.st_size;
    // 使用mmap映射
    char *addr = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    this->m_BufferManager->SetMmapAddr(addr);
}
