# EDA_Challenge

![Version](https://img.shields.io/badge/Version-1.0.1-brightgreen.svg)&nbsp;&nbsp;![Build](https://img.shields.io/badge/Build-Passed-success.svg)&nbsp;&nbsp;![License](https://img.shields.io/badge/License-AGPL-blue.svg)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[English](https://github.com/ZhuYanzhen1/EDA_Challenge/blob/master/README.md) / 中文

&nbsp;&nbsp;&nbsp;&nbsp;本仓库是集成电路EDA设计精英挑战赛《数字集成电路动态功耗优化策略分析与评估》赛题的代码仓。作为衡量数字芯片设计的重要的指标（PPA）之一，低功耗设计的分析与优化显得尤为重要。

&nbsp;&nbsp;&nbsp;&nbsp;本项目要实现的是具有对VCD文件中的信号翻转次数、翻转率、电平持续时间、毛刺次数统计和GUI翻转次数显示的工具，并尝试对其大文件解析和信号统计速度进行优化。

***

### 用法指南

&nbsp;&nbsp;&nbsp;&nbsp;我们提供了基于Ubuntu22.04.1 LTS操作系统的deb安装包，你可以在[Release](https://github.com/ZhuYanzhen1/EDA_Challenge/releases/latest)页面找到。下载安装包后使用`sudo dpkg -i EDA_Challenge.deb`进行安装，安装完成后即可使用本程序。程序基本用法：

```bash
$ EDA_Challenge -h                               # 获取程序帮助信息
$ EDA_Challenge -v                               # 获取程序版本信息
$ EDA_Challenge --gui                            # 以GUI的形式启动程序
$ EDA_Challenge --file <VCD file path>           # 指定待统计的VCD文件
$ EDA_Challenge --glitch --file <VCD file path>  # 使能毛刺统计
```

&nbsp;&nbsp;&nbsp;&nbsp;程序的高级用法为指定统计时间范围和指定模块统计。高级用法只能通过命令行实现，你无法通过GUI界面来进行指定统计时间范围和指定模块统计。

```bash
# 指定csv文件的输出路径：
$ EDA_Challenge --file <VCD file path> --output <Output directory>
# 指定待统计信号的模块：
$ EDA_Challenge --file <VCD file path> --scope <Scope name>
# 指定待统计信号的统计时间范围：
$ EDA_Challenge --file <VCD file path> --begin <Begin time> --end <End time>
# 指定待统计信号的模块和时间范围：
$ EDA_Challenge --file <VCD file path> --begin <Begin time> --end <End time> --scope <Scope name>
```

***

### 编译指南

&nbsp;&nbsp;&nbsp;&nbsp;由于本程序是在Ubuntu22.04.1 LTS环境下开发，所以本指南不能保证在其它操作系统下适用。我们提供了一键配置脚本用于安装编译依赖：

```bash
$ git clone https://github.com/ZhuYanzhen1/EDA_Challenge.git --depth=1
$ cd EDA_Challenge/script && ./configure
```

&nbsp;&nbsp;&nbsp;&nbsp;无错误地运行完上述脚本后，你可以在build文件夹下看到编译好的EDA_Challenge二进制文件以及用于测试的Google_Test程序。你可以运行Google_Test以检验程序输出的统计值是否正确。

***

### 开发环境

+ 集成开发环境：CLion CL-222.4167.35
+ 操作系统：Ubuntu 22.04.1 LTS 5.15.0-48-generic x86_64
+ Make工具：GNU Make 4.3 x86_64-pc-linux-gnu
+ CMake工具：CMake version 3.22.1
+ 编译工具链：gcc version 11.3.0 (Ubuntu 11.3.0-1ubuntu1~22.04)
+ 文档生成工具：Doxygen GUI 1.9.4 (Qt version 5.15.2)

***

### 许可证

**组织：AcmeTech <br>
作者：朱彦臻，宫艺心，周梓杰<br>
维护人：朱彦臻,  2208213223@qq.com**

&nbsp;&nbsp;&nbsp;&nbsp;本项目的源代码根据[AGPL-3.0许可证](https://github.com/ZhuYanzhen1/EDA_Challenge/blob/master/LICENSE)发布。本项目已经在Ubuntu 20.04和22.04下进行了测试，并确保其可正常运行。这是一个学习型代码，希望它经常更改，并且不承认任何特定用途的适用性。