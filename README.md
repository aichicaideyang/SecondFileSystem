# 多用户二级文件系统
同济大学 计算机科学与技术 操作系统课程设计

## 参考
本系统参考学长学姐代码
Link: https://github.com/fffqh/MultiUser-secondFileSystem
主要改动为shell交互和部分代码重构(文件系统内部已经写的太完美了)


## 系统启动

主目录下：

```bash
make
```

得到可执行文件 yuzhuoLinux

启动服务端:

```bash
./yuzhuoLinux
```

启动客户端：

```bash
cd client
make
./client localhost 1234
```



## 系统运行情况

这里逐项演示文件系统的各个操作。

首先我们启动服务端：

![image-20230515085612728](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515085612728.png)

可以看到server端初始化已经成功，在等待用户接入。

接下来我们启动客户端：

![image-20230515085658864](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515085658864.png)

可以看到客户端成功连接到服务端，需要我们输入用户名和密码，这里我们可以测试一下如果输入错误的用户名和密码会发送什么：

![image-20230515085741494](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515085741494.png)

这里我输入了错误的用户名和密码(用户名和密码可见文件user.txt)，服务端会提示用户不存在或者密码不正确。

下面我们输入正确的用户名和密码：

![image-20230515085856044](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515085856044.png)

可以看到我们成功进入到了文件系统之中，并且一开始在文件系统中我们接收到了help信息(可以获取支持的指令及其格式)。

并且从前置标识符：root@skyoung:~/$ 中我们可以得知当前用户名为root,机器名为skyoung,路径为~/即当前用户的家目录。

下面我们开始指令的测试：

首先测试pwd指令：

![image-20230515090039280](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090039280.png)

可以看到pwd成功打印了当前工作目录信息

测试 ls 指令：

![image-20230515090113705](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090113705.png)

可以看到当前目录下没有任何文件，所以没有显示内容

测试mkdir指令：

![image-20230515090155791](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090155791.png)

可以看到成功创建了文件夹a

测试cd指令：

![image-20230515090221731](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090221731.png)

可以看到成功进入了文件夹a

测试touch指令：

![image-20230515090251183](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090251183.png)

可以看到成功创建了文件testout.txt

测试vim指令：

![image-20230515090326517](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090326517.png)

可以看到成功向testout.txt写入了5个字节，下面我们用cat指令查看其中内容：

测试cat指令：

![image-20230515090400865](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090400865.png)

可以看到写入到testout.txt内的内容是正确的

下面我们测试fileout指令：

![image-20230515090531384](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090531384.png)

![image-20230515090547353](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090547353.png)

在当前文件目录下发现成功生成了testout.txt文件，打开查看：

![image-20230515090618565](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090618565.png)

内容正确！

下面测试filein指令：

我在当前文件目录下创建了文件testin.txt:

![image-20230515090650440](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090650440.png)

下面将其移动到文件系统内部：

![image-20230515090726849](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090726849.png)

可以看到当前目录下确实存在testin.txt，用 cat指令查看后发现内容一致！

多用户测试：

![image-20230515091721348](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515091721348.png)

可以看到我们还可以同时登录skyoung用户！

最后我们测试help指令及q指令：

![image-20230515090826190](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090826190.png)

![image-20230515090840831](https://cdn.jsdelivr.net/gh/aichicaideyang/PicUpload@main/image-20230515090840831.png)

可以看到均成功执行，测试完毕！

注：虽然自己也做了一些关于错误情况的处理，但可能系统中或多或少仍存在一些BUG。。。

