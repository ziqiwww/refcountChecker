# RefcountChecker

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
└── settings.json	项目配置文件。
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
 clang -emit-llvm -S -fno-discard-value-names ./test/spammodule.c -I /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/Headers -o ./test/spammodule.ll
```

注意把 `Python.h`的路径链接进来。

## 工具配置

使用json配置，分为ToolConfig和AnalyzeConfig

ToolConfig用于配制一些基本信息，比如使用的clang路径，生成.ll需要的动态库信息等。这些信息的使用在今后工具打包的时候可能需要使用，即输入单一module.c文件能够辅助生成.ll中间代码。目前该条目为空，后期可以集成更多的分析方法并给出选择。

AnalyzeConfig用于配制数据流分析的基本信息，包括

* 入口函数 "entryFunction"
* 分析模式 "analysesMode"：过程间inter，过程内intra，默认过程内。

## 下一阶段目标

针对不同语句给出引用计数增减情况以及数据流传播的定义。
