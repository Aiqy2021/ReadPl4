# ReadPl4



## 1 概述

**ReadPl4**是一款使用C++编写的用于读取**ATP-EMTP**仿真生成的 **.pl4** 格式数据文件的工具库，支持查看文件中所有变量的名称和查询指定变量的完整数据，可快速验证 **.pl4** 文件的可读性并提取关键仿真数据。





## 2 环境要求

1. 编译环境：支持 C++11 及以上标准的编译器（如 GCC 4.8+、MSVC 2015+、MinGW-w64）；

2. 运行环境：Windows/Linux/macOS（与编译平台一致）；

3. 依赖：无第三方库依赖，仅需标准 C++ 库。






## 3 Pl4文件格式

![pl4文件数据结构](../docs/assets/pl4文件的数据结构.png)

### 3.1 数据类型规范

- **文本类型**：采用 ASCII 编码，主要用于头部标识、变量名、参数名（如`tMax`、`Node From`）
- **数值类型**：采用 IEEE 754 单精度浮点数（32 位），用于存储时间和变量数值
- **填充类型**：采用十六进制`00`填充，用于数据结构对齐

### 3.2 字节序规则

- **浮点数存储**：严格采用**小端序（Little Endian）**，即低字节在前、高字节在后
- **文本存储**：ASCII 字符按正常顺序存储，无需字节序转换

### 3.3 区域划分规则

|  区域名称  |     行范围     |        功能        |
| :--------: | :------------: | :----------------: |
|   头部区   |     0-1 行     | 文件标识和版本信息 |
|   参数区   |     2-3 行     | 时间参数和文件属性 |
|   填充区   |      4 行      |    数据结构对齐    |
| 变量定义区 |   5->5+n 行    |  变量节点信息定义  |
|   数据区   | 5+n+1 行及以后 |  实际时间序列数据  |





## 4 PL4.h 头文件使用说明

### 4.1 头文件概述

**PL4.h** 是解析**.pl4** 文件的核心头文件，封装了 **PL4** 类，提供**.pl4** 文件加载、变量名称查询、变量数据提取等接口，无需依赖第三方库，可直接集成到 C++ 项目中。



### 4.2 核心结构说明

#### 4.2.1 PL4VarInfo（变量元信息结构体）

存储单个变量的类型和节点信息，仅内部解析使用：

| 成员 | 类型   | 说明                                                         |
| ---- | ------ | ------------------------------------------------------------ |
| type | int    | 变量类型：4（节点电压 V-node）、7（分支电压 E-bran）、8（分支电压差 V-bran）、9（分支电流 I-bran） |
| from | string | 起始节点名称（6 字符，自动去除尾部空格）                     |
| to   | string | 终止节点名称（6 字符，自动去除尾部空格）                     |

#### 4.2.2 PL4Data（文件完整数据结构体）

存储**.pl4** 文件的全部解析结果，仅内部解析使用：

| 成员   | 类型               | 说明                                                         |
| ------ | ------------------ | ------------------------------------------------------------ |
| deltaT | float              | 仿真时间步长（单位：s）                                      |
| nVar   | int                | 变量数量（不含时间变量）                                     |
| steps  | int                | 时间步数（含 0 时刻）                                        |
| tMax   | float              | 最大仿真时间（单位：s）                                      |
| vars   | vector<PL4VarInfo> | 所有变量的元信息列表                                         |
| data   | vector<float>      | 原始数据矩阵（行优先：每行 steps 个采样点，第 0 列为时间，1~nVar 列为变量数据） |



### 4.3 PL4 类核心接口

#### 4.3.1 加载**.pl4**文件

```cpp
bool loadPL4(std::string file_name);
```

**功能**: 加载并解析**.pl4** 文件，解析结果存储到类内部成员；
**参数**: `file_name` - **.pl4** 文件的绝对 / 相对路径；
**返回值**: 

`true`-解析成功；

`false`-解析失败，终端输出具体错误原因。
**示例**: 

```cpp
PL4 pl4Reader;
if (!pl4Reader.loadPL4("C:/test.pl4")) {
    std::cerr << "文件解析失败" << std::endl;
}
```

#### 4.3.2 获取所有变量名称列表

```cpp
std::vector<std::string> getVarNameList(void);
```

**功能**: 返回解析后所有变量的名称（包含时间变量 t）；
**返回值**: 变量名称列表。

变量名命名规则如下：

|    变量类型     |    命名规则     | 示例         |
| :-------------: | :-------------: | ------------ |
|      时间       |        t        | t            |
|  节点电压（4）  |   v:<节点名>    | v:bus1       |
|  分支电压（7）  | c:<起始>-<终止> | c:line1-bus2 |
| 分支电压差（8） | v:<起始>-<终止> | v:line1-bus2 |
|  分支电流（9）  | c:<起始>-<终止> | c:line1-bus2 |
|    **示例**:    |                 |              |

```cpp
std::vector<std::string> varList = pl4Reader.getVarNameList();
for (const auto& name : varList) {
    std::cout << "变量名：" << name << std::endl;
}
```

#### 4.3.3 获取指定变量的采样数据

```cpp
std::vector<double>& getDataOfVar(std::string var_name);
```

**功能**: 返回指定变量的所有采样数值（按时间步排序）；
**参数**: `var_name` - 变量名称（需与 getVarNameList 返回的名称完全一致）；
**返回值**: 

- 变量存在：返回该变量的采样数据向量（double 类型，长度为时间步数）；

- 变量不存在：返回时间变量 t 的数据向量（兼容空指针，避免程序崩溃）。

**示例**: 

```cpp
// 获取节点电压 v:bus1 的数据
std::vector<double>& bus1Voltage = pl4Reader.getDataOfVar("v:bus1");
for (size_t i = 0; i < bus1Voltage.size(); ++i) {
    std::cout << "第 " << i+1 << " 步：" << bus1Voltage[i] << std::endl;
}
```



### 4.4 集成示例

完整集成示例（读取文件 → 查变量 → 提数据）：

```
#include "PL4.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    // 1. 初始化 PL4 类
    PL4 pl4Reader;

    // 2. 加载 .pl4 文件
    std::string pl4Path = "C:/ATP_Sim/test.pl4";
    if (!pl4Reader.loadPL4(pl4Path)) {
        std::cerr << "加载 " << pl4Path << " 失败" << std::endl;
        return 1;
    }
    std::cout << "文件加载成功！" << std::endl;

    // 3. 获取所有变量名称
    std::vector<std::string> varList = pl4Reader.getVarNameList();
    std::cout << "变量列表：" << std::endl;
    for (size_t i = 0; i < varList.size(); ++i) {
        std::cout << i+1 << ". " << varList[i] << std::endl;
    }

    // 4. 提取指定变量数据
    std::string targetVar = "v:bus1";
    std::vector<double>& varData = pl4Reader.getDataOfVar(targetVar);
    std::cout << "\n变量 " << targetVar << " 的前10步数据：" << std::endl;
    for (int i = 0; i < 10 && i < varData.size(); ++i) {
        std::cout << "t=" << i*pl4Reader.getDataOfVar("t")[i] << "s: " << varData[i] << std::endl;
    }

    return 0;
}
```





## 5 ReadPl4库的测试

项目提供了对**PL4.h**的测试程序**test_PL4.cpp**，可生成可执行文件readPL4，使用方法如下。

1. 启动工具
   将`readPL4.exe`复制到某个目录下，打开`CMD`，进入`readPL4.exe`所在目录，执行以下命令：

```bash
./readPl4 <pl4文件路径>
```

> pl4文件的路径可以为相对路径，形如./example.pl4

2. 运行反馈
   成功：终端输出 成功：PL4 文件读取完成！，进入交互模式；
   失败：终端输出 错误：PL4 文件读取失败！，并提示具体原因（如文件无法打开、数据解析失败等），程序退出。

3. 交互命令
   进入交互模式后，支持以下命令：
   表格


|   命令格式   | 功能说明                                   | 示例                           |
| :----------: | ------------------------------------------ | ------------------------------ |
|     help     | 查看所有支持的命令及用法                   | help                           |
|     vars     | 列出文件中所有变量的完整名称               | vars                           |
| var:<变量名> | 输出指定变量的所有采样数值（按时间步排序） | var::c:bus1、var:c:line1-line2 |
|     exit     | 退出工具                                   | exit                           |

4. 命令使用示例

```bash
# 查看所有变量
请输入命令（输入 help 查看帮助，exit 退出）：vars
===== 所有变量名称 =====
1. t
2. v:bus1
3. c:line1-line2
========================

# 查看指定变量数据
请输入命令（输入 help 查看帮助，exit 退出）：var:v:bus1
===== 变量 "v:bus1" 的数值 =====
第 1 个采样点: 0.0
第 2 个采样点: 220.5
第 3 个采样点: 221.1
...
===============================

# 查看帮助
请输入命令（输入 help 查看帮助，exit 退出）：help
===== PL4 文件读取工具帮助 =====
1. vars       - 显示所有变量名称列表
2. var:<变量名> - 显示指定变量的所有数值（例：var:v:bus1）
3. help       - 显示此帮助信息
4. exit       - 退出程序
============================
```

