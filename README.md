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
* 分析模式 "analysisMode"：。

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
        "analysisMode": "intra",
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

## 项目进展

目前能够使用llvm内置的basic-aa 的指针分析判断别名，进行单函数bug report，目前存在的问题是
由于basic-aa精度有限因此可能误报率比较高，需要进一步优化。

对于内置API的处理是需要将其在参数处stole或borrow以及返回weak或strong的元信息硬编码到
pass中，对于没编码的函数认为是用户自定义函数，过程间分析也是类似，需要将函数的元信息返回给
调用者的分析，调用者根据元信息再进行分析。

以下是针对spammodule.c的分析结果，
UAF表示释放后使用，MLK表示内存泄漏，Memory:AAMemRef__No__0表示该ref指向的实际内存id，Function:PyInit_spam表示
函数名为PyInit_spam，Line:6128163511表示行号（），Variable:表示变量名，如果为空则表示某个临时变量。

```asm
<bugReport> Error:UAF;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:6128163511;Variable:;
<bugReport> Error:UAF;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:6128163511;Variable:;
<bugReport> Error:UAF;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:6128163511;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:Spam_Error;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call1;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:Spam_Error;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call1;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:Spam_Error;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call;
<bugReport> Error:MLK;Memory:AAMemRef__No__0;Function:PyInit_spam;Line:0;Variable:call1;

```


