在g++编译通过，在windows centos ubuntu raspberrypi(raspbian)测试可用。

在windows静态编译：
```bash
mingw32-make SYSTEM=windows RELEASE_MODE=static
```

在windows动态编译：
```bash
mingw32-make SYSTEM=windows RELEASE_MODE=shared
```

在linux静态编译：
```bash
make SYSTEM=linux RELEASE_MODE=static
```

在linux动态编译：
```bash
make SYSTEM=linux RELEASE_MODE=shared
```

SYSTEM默认为：windows，RELEASE_MODE默认为：static。

清理.o文件命令为：
```bash
mingw32-make clean  # windows
make clean          # linux
```

删除生成的库命令为：
```bash
mingw32-make remove-lib                           # windows下删除静态库（.a文件）
mingw32-make RELEASE_MODE=shared remove-lib       # windows下删除动态库（.dll文件）
make SYSTEM=linux remove-lib                      # linux下删除静态库（.a文件）
make SYSTEM=linux RELEASE_MODE=shared remove-lib  # linux下删除动态库（.so文件）
```
