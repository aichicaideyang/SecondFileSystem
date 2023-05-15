#include "server_supprot.h"
#include "Kernel.h"
stringstream help_info;
map<string, string> user_table; // 这里是用户静态表 目前写死 可以按需要添加
void init_user_table()
{
    ifstream fin;
    fin.open("user.txt");
    if (!fin.is_open())
    {
        sys_err("Failed to open user.txt!");
        exit(-1);
    }
    int user_nums;
    fin >> user_nums;
    // cout << "user_nums is " << user_nums << "\n";
    for (int i = 0; i < user_nums; i++)
    {
        string user_name, user_pwd;
        fin >> user_name >> user_pwd;
        user_table[user_name] = user_pwd;
    }

    fin.close();
    sys_log("Initialize user_table complete!");
}
void init_help_info()
{
    help_info << "********************Welcome To Use Yuzhuo UNIX*********************"
              << "\n";
    help_info << "************Now The System Support The Follwing Command************"
              << "\n";
    help_info << "1. ls: Format: ls"
              << "\n";
    help_info << "2. cd: Format: cd [file_path]"
              << "\n";
    help_info << "3. pwd: Format: pwd"
              << "\n";
    help_info << "4. touch: Format: touch [file_name]"
              << "\n";
    help_info << "5. mkdir: Format: mkdir [dir_name]"
              << "\n";
    help_info << "6. vim: Format: vim [file_name] [file_content]"
              << "\n";
    help_info << "7. cat: Format: cat [file_name]"
              << "\n";
    help_info << "8. fileout: Format: fileout [infile_path] [outfile_path]"
              << "\n";
    help_info << "9. filein: Format: filein [infile_path] [outfile_path]"
              << "\n";
    help_info << "10. quit: Format: q"
              << "\n";
}
class SendInfo
{
private:
    int fd;
    string username;

public:
    int send_to_client(const stringstream &send_info)
    {
        auto send_bytes = send(fd, send_info.str().c_str(), send_info.str().length(), 0);
        cout << "Send to " << username << " " << send_bytes << " bytes"
             << "\n";
        return send_bytes;
    };
    int send_to_client(const string &send_info)
    {
        auto send_bytes = send(fd, send_info.c_str(), send_info.length(), 0);
        cout << "Send to " << username << " " << send_bytes << " bytes"
             << "\n";
        return send_bytes;
    };
    SendInfo(int fd, string username)
    {
        this->fd = fd;
        this->username = username;
    };
};
int deal_trans_bytes(const int &trans_bytes, const string info, int mode)
{
    if (mode == 0)
    {
        if (trans_bytes > 0)
        {
            sys_log("Send successfully!");
            sys_log(info);
            return 1;
        }
        else
        {
            sys_log("Send faild!");
            return 0;
        }
    }
    else
    {
        if (trans_bytes > 0)
        {
            sys_log("Receive successfully!");
            sys_log(info);
            return 1;
        }
        else
        {
            sys_log("Receive faild!");
            return 0;
        }
    }
}
void *client_thread(void *Client_fd)
{
    int client_fd = *(int *)Client_fd;               // 客户端socket fd
    char tmp_buf[BUFSIZE + 1];                       // 用来接收消息的临时buffer
    int trans_bytes;                                 // 传输和接收到的byte数
    int ret;                                         // 调用函数的返回值
    string para_1, para_2;                           // 目前支持的API 最多使用两个参数
    string command;                                  // 指令API
    string user_name = "";                           // 用户名
    string user_pwd = "";                            // 用户密码
    string prefix_info = user_name + "@skyoung:~/$"; // 前置提示信息
    string cur_parent_path = "~/";                   // 初始父亲目录为home目录
    string file_path = "";                           // 文件名称(路径)
    string file_content = "";                        // 要写入的文件内容
    string dir_path = "";                            // 目录名称(路径)
    string infile_path = "";                         // FileSystem内的文件路径
    string outfile_path = "";                        // 外部文件路径
                                                     /*
                                                     for (auto &e : user_table)
                                                         cout << e.first << " " << e.second << "\n";
                                                     */
    trans_bytes = send(client_fd, "Please input the user_name: ", sizeof("Please input the user_name: "), 0);
    if (deal_trans_bytes(trans_bytes, "Please input the user_name: ") != 1)
    {
        sys_err("Client quit!");
        return (void *)NULL;
    }
    while (1)
    {

        memset(tmp_buf, 0, sizeof(tmp_buf));
        trans_bytes = recv(client_fd, tmp_buf, BUFSIZE, 0);
        cout << "Received1 : " << tmp_buf << "\n";
        if (deal_trans_bytes(trans_bytes, tmp_buf, 1) != 1)
        {
            sys_err("Client quit!");
            return (void *)NULL;
        }
        if (trans_bytes <= -1) // 如果没有收到就返回
            return (void *)NULL;
        user_name = tmp_buf;
        trans_bytes = send(client_fd, "Please input the user_pwd: ", sizeof("Please input the user_pwd: "), 0);
        if (deal_trans_bytes(trans_bytes, "Please input the user_pwd: ") != 1)
        {
            sys_err("Client quit!");
            return (void *)NULL;
        }
        memset(tmp_buf, 0, sizeof(tmp_buf));
        trans_bytes = recv(client_fd, tmp_buf, BUFSIZE, 0);
        cout << "Received2 : " << tmp_buf << "\n";
        if (deal_trans_bytes(trans_bytes, tmp_buf, 1) != 1)
        {
            sys_err("Client quit!");
            return (void *)NULL;
        }
        if (trans_bytes <= -1) // 如果没有收到就返回
            return (void *)NULL;
        user_pwd = tmp_buf;
        cout << "user_name: " << user_name << "\n";
        cout << "user_pwd: " << user_pwd << "\n";
        if (user_table.count(user_name) == 0 || user_table[user_name] != user_pwd)
        {
            string info1 = "User is not find or the password is incorrect!\n";
            string info2 = "Please input the user_name: ";
            string cur_info = info1 + info2;
            trans_bytes = send(client_fd, cur_info.c_str(), cur_info.size(), 0);
            if (deal_trans_bytes(trans_bytes, cur_info.c_str()) != 1)
            {
                sys_err("Client quit!");
                return (void *)NULL;
            }
        }
        else
            break;
    }
    // cout << "go here !" << "\n";
    SendInfo si(client_fd, user_name);                // 绑定fd 以及username方便后续发送
    prefix_info = user_name + prefix_info;            // 带上用户名
    si.send_to_client(help_info.str() + prefix_info); // 初始状态下 发送帮助信息
    if (deal_trans_bytes(trans_bytes, help_info.str() + prefix_info) != 1)
    {
        sys_log("User disconnect!");
        Kernel::Instance().GetUserManager().Logout();
        return (void *)NULL;
    }
    // 这里需要在kernel中登录user ,初始化家目录，绑定线程号和user结构
    Kernel::Instance()
        .GetUserManager()
        .Login(user_name);
    User &u = Kernel::Instance().GetUser(); // 获得当前用户
    while (1)
    {
        // 进入循环交互
        // 前置提示信息
        stringstream send_info;       // 用来向客户端发送信息
        stringstream command_decoder; // 这里利用stringstream方便解析命令
        // 这俩变量不知道为啥不能清空会导致问题。。
        command = "";
        memset(tmp_buf, 0, sizeof(tmp_buf));
        trans_bytes = recv(client_fd, tmp_buf, BUFSIZE, 0);
        if (deal_trans_bytes(trans_bytes, tmp_buf, 1) != 1)
        {
            Kernel::Instance().GetUserManager().Logout();
            return (void *)NULL;
        }
        command_decoder << tmp_buf;
        cout << "Received : " << tmp_buf << "\n";
        cout << "CommandDecoder is : " << command_decoder.str() << "\n";
        command_decoder >> command; // 首先是command API
        cout << "Command is : " << command << "\n";
        if (command == "" || command == " ")
        {
            si.send_to_client(prefix_info);
            continue;
        }
        if (command == "pwd")
        {
            send_info << u.u_curdir << "\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "help")
        {
            si.send_to_client(help_info.str() + prefix_info);
            continue;
        }
        if (command == "vim")
        {
            // 这里需要先打开文件 然后再写入
            command_decoder >> file_path;    // 获取文件路径
            command_decoder >> file_content; // 获取文件内容
            auto fd = Kernel::Instance().Fopen(file_path, 2);
            if (fd < 0)
            {
                send_info << "Can't find the file!"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            if (file_content.length() > BUFSIZE)
            {

                send_info << "The content should be less than 1024 bytes"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            memset(tmp_buf, 0, sizeof(tmp_buf));
            strcpy(tmp_buf, file_content.c_str());
            ret = Kernel::Instance().Fwrite(fd, file_content.size(), tmp_buf);
            send_info << "Succeed to write " << ret << " bytes!\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "ls")
        {
            auto file_info = Kernel::Instance().Fls();
            send_info << file_info;
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "cd")
        {
            command_decoder >> dir_path;
            if (dir_path == "")
            {
                send_info << "You need to input the directory you want to cd"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            auto cur_dir_name = Kernel::Instance().Fcd(dir_path);
            prefix_info = prefix_info.substr(0, prefix_info.size() - 1) + dir_path + "/$";
            send_info << "Now you cd in the directory : " << cur_dir_name << "\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }

        if (command == "mkdir")
        {
            command_decoder >> dir_path;
            if (dir_path == "")
            {
                send_info << "You need to input the directory you want to create"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            // cout << "here 1"   << "\n";
            ret = Kernel::Instance().FcreateDir(dir_path);
            // cout << "here 2"  << "\n";
            send_info << "Now you have made the directory : " << dir_path << "\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "rm")
        {
            command_decoder >> file_path;
            if (file_path == "")
            {
                send_info << "You need to input the file_path you want to remove"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            ret = Kernel::Instance().Fdelete(file_path);
            send_info << "Now you have removed the file : " << file_path << "\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "touch")
        {
            command_decoder >> file_path;
            if (file_path == "")
            {
                send_info << "You need to input the file_path you want to create"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            ret = Kernel::Instance().Fcreate(file_path);
            send_info << "Now you have created the file : " << file_path << "\n";
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }

        if (command == "cat")
        {
            command_decoder >> file_path;
            if (file_path == "")
            {
                send_info << "You need to input the file_path you want to view"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            auto info = Kernel::Instance().Fcat(file_path);
            send_info << info;
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "filein")
        {
            command_decoder >> infile_path >> outfile_path;
            /*
            cout << "command is" << command << "\n";
            cout << "infile : " << infile_path << "\n";
            cout << "outfile : " << outfile_path << "\n";*/
            if (outfile_path == "" || infile_path == "")
            {
                send_info << "You need to input the infile_path and outfile_path"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            auto info = Kernel::Instance().Fin(infile_path, outfile_path);
            send_info << info;
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "fileout")
        {
            command_decoder >> infile_path >> outfile_path;
            if (outfile_path == "" || infile_path == "")
            {
                send_info << "You need to input the infile_path and outfile_path"
                          << "\n";
                si.send_to_client(send_info.str() + prefix_info);
                continue;
            }
            auto info = Kernel::Instance().Fout(infile_path, outfile_path);
            send_info << info;
            si.send_to_client(send_info.str() + prefix_info);
            continue;
        }
        if (command == "q")
        {
            Kernel::Instance().GetUserManager().Logout();
            send_info << "User logout!";
            si.send_to_client(send_info.str() + "\nGood Bye!\n");
            break;
        }
        si.send_to_client("Wrong command, you can use help to view more information!\n" + prefix_info);
    }
    close(client_fd); // 关闭客户端filedescriptor
    return (void *)NULL;
}
void initialize_listen(struct sigaction &action, int &listen_fd, int &sin_size, struct sockaddr_in &server, struct sockaddr_in &client)
{
    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);
    sin_size = sizeof(struct sockaddr_in);
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        sys_err("Failed to create listen fd!");
        exit(1);
    }
    int opt = SO_REUSEADDR;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        sys_err("Bind listen_fd error!");
        exit(1);
    }
    if (listen(listen_fd, BACKLOG) == -1)
    {
        sys_err("Listen error!");
        exit(1);
    }
}