# EDA_Challenge

![Version](https://img.shields.io/badge/Version-1.0.1-brightgreen.svg)&nbsp;&nbsp;![Build](https://img.shields.io/badge/Build-Passed-success.svg)&nbsp;&nbsp;![License](https://img.shields.io/badge/License-AGPL-blue.svg)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;English / [中文](https://github.com/ZhuYanzhen1/EDA_Challenge/blob/master/README_CN.md)

&nbsp;&nbsp;&nbsp;&nbsp;This repository is a source code for the IC EDA Design Elite Challenge "Analysis and Evaluation of Dynamic Power Consumption Optimization Strategies for Digital Integrated Circuits" competition. As one of the important indicators (PPA) to measure digital chip design, the analysis and optimization of low-power design is particularly important. 

&nbsp;&nbsp;&nbsp;&nbsp;This project is to implement a tool that has the ability to count the number of signal flips, flip rate, level duration, glitch count and flip number display on GUI in VCD files, and try to optimize its large file parsing and signal count speed.

***

### Usage guide

&nbsp;&nbsp;&nbsp;We provide a deb installation package based on the Ubuntu 22.04.1 LTS operating system, which you can find on the [Release](https://github.com/ZhuYanzhen1/EDA_Challenge/releases/latest) page. After downloading the installation package, use `sudo dpkg -i EDA_Challenge.deb` to install it, and you can use this program after the installation is complete. Basic usage of the program:

```bash
$ EDA_Challenge -h									# Get help information
$ EDA_Challenge -v									# Get version information
$ EDA_Challenge --gui								# Start the program as a GUI
$ EDA_Challenge --file <VCD file path>				# Specify the VCD file to be counted
$ EDA_Challenge --glitch --file <VCD file path>		# Enable glitch counting
```

&nbsp;&nbsp;&nbsp;&nbsp;The advanced usage of the program is to specify the statistical time range and specify the module statistics. The advanced usage can only be achieved through the command line, you cannot specify the statistical time range and specify the module statistics through the GUI interface: 

```bash
# Specify the output path of the csv file:
$ EDA_Challenge --file <VCD file path> --output <Output directory>
# Specify the module for which the signal is to be counted:
$ EDA_Challenge --file <VCD file path> --scope <Scope name>
# Specify the time range of the signal to be counted:
$ EDA_Challenge --file <Specify the file path> --begin <Begin time> --end <End time>
# Specify the module and time range of the signal to be counted:
$ EDA_Challenge --file <Specify the file path> --begin <Begin time> --end <End time> --scope <Scope name>
```

***

### Compilation Guide

&nbsp;&nbsp;&nbsp;&nbsp;Since this program was developed under Ubuntu 22.04.1 LTS environment, this guide is not guaranteed to work under other operating systems. A one-click configuration script is provided to install the dependencies:

```bash
$ git clone https://github.com/ZhuYanzhen1/EDA_Challenge.git --depth=1
$ cd EDA_Challenge/script && ./configure
```

&nbsp;&nbsp;&nbsp;&nbsp;After running the above script without errors, you can see the compiled EDA_Challenge binary and the Google_Test program for testing in the build folder. You can run Google_Test to check if the statistics output from the program are correct.

***

### Develop Environment

+ Integrated Development Environment: CLion CL-222.4167.35
+ OS: Ubuntu 22.04.1 LTS 5.15.0-48-generic x86_64
+ Make tool: GNU Make 4.3 x86_64-pc-linux-gnu
+ CMake tool: CMake version 3.22.1
+ Compiler tool chain: gcc 11.3.0 (Ubuntu 11.3.0-1ubuntu1~22.04)
+ Documentation tool: Doxygen GUI 1.9.4 (Qt version 5.15.2)

***

### License

**Affiliation：AcmeTech<br>
Author: Yanzhen Zhu, Yixin Gong, Zijie Chou<br>
Maintainer：Yanzhen Zhu, 2208213223@qq.com**

&nbsp;&nbsp;&nbsp;&nbsp;The source code is released under a [AGPL-3.0 License](https://github.com/ZhuYanzhen1/EDA_Challenge/blob/master/LICENSE).The product has been tested under Ubuntu 20.04 and 22.04. This is research code, expect that it changes often and any fitness for a particular purpose is disclaimed.
