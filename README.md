# RefcountChecker

## 项目组织

RCTool是后端入口，用于module的载入和IR的生成，依次调用`clang -emit-llvm`和`opt`
指令，因此项目构建和IR生成两节中的相关指令可以不用管，直接运行target/bin下的tool即可。注意如果运行tool需要确保unix环境，windows不能保证能够成功运行指令。

RefcntAnalysis是llvm pass的分析逻辑和数据结构。

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
* 分析模式 "analysesMode"：。

```json
{
  	// 配置RCTool
    "ToolConfig": {
      	// 指定clang路径，如果缺省会检测系统clang路径
        "clang": "/opt/homebrew/opt/llvm/bin/clang",
      	// 指定要分析的module.c文件
        "module": "/Users/ziqi/Work/pyc/workaround/refcountChecker/test/spammodule.c",
        "args": [
            "-I",
            "/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/Headers"
        ]
      	// 配置完成后将运行 IR生成 一节的指令
    },
    "AnalysisConfig": {
      	// 分析模式： 过程间inter，过程内intra，默认过程内
        "analysesMode": "intra",
      	// 入口函数
        "entryFunction": "PyInit_spam",
        // 详细打印
        "verbose": true
    }
}
```

## 下一阶段目标

针对不同语句给出引用计数增减情况以及数据流传播的定义。

受常量传播和污点分析的启发，前者可以用于跟踪引用计数变化，在交汇处可以检测出不一致（参考矛盾路径对那篇文章），在常量传播过程中把污点数据加入考量，在所有sink的函数处检查该函数的API类型，并更新计数或报告bug.

完成结果保存路径的设置

## 拾遗

1. refcount当且仅当调用点处被更新(call instruction)

