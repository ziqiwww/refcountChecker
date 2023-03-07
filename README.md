# refcountChecker

## 项目组织

```
.
├── CmakeLists.txt
├── LICENSE
├── README.md
├── RefcntAnalysis
│   ├── CmakeLists.txt	添加source，include等
│   ├── RefcntAnalysis.cpp
│   └── include		所有头文件
│       ├── RefcntPass.h
│       └── config.h
├── externals		使用到的外部库
│   └── nlohmann
│       └── json.hpp
└── settings.json	项目配置文件，包括分析模式，入口函数等信息，该文件路径和文件名不要改变
```

## 项目构建

```
mkdir build && cd build
cmake .. && make
```

生成.so文件位于 `target/lib`使用如下指令运行分析，`.ll`生成见下节

```
opt -load ./target/lib/libRefcntAnalysis.so -refcnt ./test/spammodule.ll -enable-new-pm=0
```

## IR生成

在 `./test`下有一个spammodule.c等文件，简单包含了一些与引用计数有关的API，构建IR指令为：

```shell
clang -emit-llvm -S ./test/spammodule.c -I /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/Headers -o ./test/spammodule.ll
```

注意把 `Python.h`的路径链接进来

## 下一阶段目标

针对不同语句给出引用计数增减情况或者分析方法的定义
