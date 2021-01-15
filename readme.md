在g++编译通过，在windows centos ubuntu raspberrypi(raspbian)测试可用。

# 功能简介
framework里是一些开发应用中通用的类，有：字节数组（ByteArray），字符串（String），文件（File），配置文件读写（IniSettings），配置文本读写（IniParse）。字符串支持编码转换，目前只实现GBK和UTF8两种编码之间互相转换。字符串内部采用UTF8编码储存数据，不使用系统编码储存数据，实现windows和linux通信不会出现中文乱码。
network里是一些通用的网络编程类，有：TCP服务器（TcpServer），TCP客户端（TcpSocket），UDP（UdpSocket）。其中TCP服务器采用select多路复用，用户连接、断开、发送信息均以回调函数的形式反馈。TCP客户端和UDP接收信息也为回调函数反馈。均采用多线程，不会对主线程造成阻塞。

# 在windows静态编译
```bash
mingw32-make SYSTEM=windows RELEASE_MODE=static
```

# 在windows动态编译
```bash
mingw32-make SYSTEM=windows RELEASE_MODE=shared
```

# 在linux静态编译
```bash
make SYSTEM=linux RELEASE_MODE=static
```

# 在linux动态编译
```bash
make SYSTEM=linux RELEASE_MODE=shared
```

# 默认值
SYSTEM默认为：windows，RELEASE_MODE默认为：static。

# 清理.o文件命令
```bash
mingw32-make clean  # windows
make clean          # linux
```

# 删除生成的库命令
```bash
mingw32-make remove-lib                           # windows下删除静态库（.a文件）
mingw32-make RELEASE_MODE=shared remove-lib       # windows下删除动态库（.dll文件）
make SYSTEM=linux remove-lib                      # linux下删除静态库（.a文件）
make SYSTEM=linux RELEASE_MODE=shared remove-lib  # linux下删除动态库（.so文件）
```

# 附带demo的编译方法
编译TCP服务器例程：
```bash
# windows下
mingw32-make clean
mingw32-make TEST_USE_FOR=TCP_SERVER

# linux下
make SYSTEM=linux clean
make SYSTEM=linux TEST_USE_FOR=TCP_SERVER
```

编译TCP客户端例程：
```bash
# windows下
mingw32-make clean
mingw32-make TEST_USE_FOR=TCP_CLIENT

# linux下
make SYSTEM=linux clean
make SYSTEM=linux TEST_USE_FOR=TCP_CLIENT
```

编译UDP例程：
```bash
# windows下
mingw32-make clean
mingw32-make TEST_USE_FOR=UDP

# linux下
make SYSTEM=linux clean
make SYSTEM=linux TEST_USE_FOR=UDP
```
