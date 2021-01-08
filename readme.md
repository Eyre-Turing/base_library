在g++编译通过。
在windows静态编译：
```
mingw32-make SYSTEM=windows RELEASE_MODE=static
```

在windows动态编译：
```
mingw32-make SYSTEM=windows RELEASE_MODE=shared
```

在linux静态编译：
```
make SYSTEM=linux RELEASE_MODE=static
```

在linux动态编译：
```
make SYSTEM=linux RELEASE_MODE=shared
```
