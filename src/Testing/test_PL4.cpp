/**
 * @file testPL4.h
 * @author Aiqy (Aiqy2021@163.com)
 * @brief 测试PL4
 * @version 1.0.0
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "PL4.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// 打印帮助信息
void printHelp() {
    std::cout << "===== PL4 文件读取工具帮助 =====" << std::endl;
    std::cout << "1. vars       - 显示所有变量名称列表" << std::endl;
    std::cout << "2. var:<变量名> - 显示指定变量的所有数值（例：var:v:bus1）" << std::endl;
    std::cout << "3. help       - 显示此帮助信息" << std::endl;
    std::cout << "4. exit       - 退出程序" << std::endl;
    std::cout << "================================" << std::endl;
}

// 分割字符串（用于解析 var:xxx 命令）
std::vector<std::string> splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    // 1. 检查命令行参数
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <PL4文件路径>" << std::endl;
        std::cerr << "示例: " << argv[0] << " C:/test.pl4" << std::endl;
        return 1;
    }

    // 2. 初始化 PL4 类并读取文件
    PL4 pl4Reader;
    std::string pl4FilePath = argv[1];
    std::cout << "正在读取 PL4 文件: " << pl4FilePath << std::endl;
    
    if (!pl4Reader.loadFile(pl4FilePath)) {
        std::cerr << "错误：PL4 文件读取失败！" << std::endl;
        return 1;
    }
    std::cout << "成功：PL4 文件读取完成！" << std::endl;

    // 3. 命令行交互逻辑
    std::string inputCmd;
    printHelp();

    while (true) {
        std::cout << "\n请输入命令（输入 help 查看帮助，exit 退出）：";
        std::getline(std::cin, inputCmd);

        // 去除命令首尾空格
        inputCmd.erase(inputCmd.find_last_not_of(" \t\n\r") + 1);
        inputCmd.erase(0, inputCmd.find_first_not_of(" \t\n\r"));

        if (inputCmd.empty()) {
            continue;
        }

        // 处理 exit 命令
        if (inputCmd == "exit") {
            std::cout << "程序退出..." << std::endl;
            break;
        }

        // 处理 help 命令
        else if (inputCmd == "help") {
            printHelp();
        }

        // 处理 vars 命令
        else if (inputCmd == "vars") {
            std::cout << "===== 所有变量名称 =====" << std::endl;
            std::vector<std::string> varList = pl4Reader.getVarNameList();
            for (size_t i = 0; i < varList.size(); ++i) {
                std::cout << i + 1 << ". " << varList[i] << std::endl;
            }
            std::cout << "========================" << std::endl;
        }

        // 处理 var:<变量名> 命令
        else if (inputCmd.substr(0, 4) == "var:") {
            std::vector<std::string> cmdParts = splitString(inputCmd, ':');
            if (cmdParts.size() < 2) {
                std::cerr << "错误：命令格式错误！正确格式：var:<变量名>" << std::endl;
                continue;
            }

            std::string varName = inputCmd.substr(4); // 提取变量名（跳过 "var:"）
            // 检查变量名是否存在
            std::vector<std::string> varList = pl4Reader.getVarNameList();
            bool varExist = false;
            for (const auto& name : varList) {
                if (name == varName) {
                    varExist = true;
                    break;
                }
            }

            if (!varExist) {
                std::cerr << "错误：变量名 \"" << varName << "\" 不存在！" << std::endl;
                continue;
            }

            // 输出变量的所有数值
            std::vector<double>& varData = pl4Reader.getDataOfVar(varName);
            std::cout << "===== 变量 \"" << varName << "\" 的数值 =====" << std::endl;
            for (size_t i = 0; i < varData.size(); ++i) {
                std::cout << "第 " << i + 1 << " 个采样点: " << varData[i] << std::endl;
            }
            std::cout << "================================" << std::endl;
        }

        // 未知命令
        else {
            std::cerr << "错误：未知命令！输入 help 查看可用命令。" << std::endl;
        }
    }

    return 0;
}