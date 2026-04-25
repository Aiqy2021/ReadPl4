/**
 * @file PL4.h
 * @author Aiqy (Aiqy2021@163.com)
 * @brief 读取ATP-EMTP的数据文件(.pl4)
 * @version 1.0.0
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef READPL4_H
#define READPL4_H

#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <iostream>


/**
 * @brief 存储单个变量的头部信息
 * @param type  4: V-node, 7: E-bran, 8: V-bran, 9: I-bran
 * @param from  起始节点（6字符，去除尾部空格）
 * @param to    终止节点（6字符，去除尾部空格）
 * 
 */
struct PL4VarInfo{
    int type;
    std::string from;
    std::string to;
};


/**
 * @brief 存储整个PL4文件的内容
 * @param deltaT    时间步长 [s]
 * @param nVar      变量数量
 * @param steps     时间步数（含0时刻）
 * @param tMax      最大时间 [s]
 * @param vars      变量头部列表
 * @param data      数据矩阵，按行优先存储：
 *                  每行 steps 个采样点，每列 (nVar+1) 个元素
 *                  第0列为时间向量，第1..nVar列为对应的变量数据
 * 
 */
struct PL4Data {
    float deltaT;
    int nVar; 
    int steps; 
    float tMax; 
    std::vector<PL4VarInfo> vars; 
    std::vector<float> data;
};

/**
 * @brief 
 * 
 */
class PL4
{
public:
    /**
     * @brief 加载文件
     * @param file_name pl4 文件全路径名称，[例] C:/example.pl4
     * @return 成功返回 true
     */
    bool loadFile(std::string file_name)
    {
        PL4Data pl4Data;
        if (!readPL4(file_name, pl4Data)) {
            std::cerr << "读取 PL4 文件失败" << std::endl;
            return false;
        }
        var_name_list.clear();
        sim_data.clear();
        const float* base = pl4Data.data.data();
        std::string tName = "t";
        std::vector<double> dataVec(pl4Data.steps);
        for (int i = 0; i < pl4Data.steps; ++i) {
            dataVec[i] = static_cast<double>(base[1 + i*(pl4Data.nVar + 1)]);
        }
        var_name_list.push_back(tName);
        sim_data.push_back(dataVec);
        for (int i = 0; i < pl4Data.nVar; ++i) {
            std::string varName = makeVarName(pl4Data.vars[i].type,
                                            pl4Data.vars[i].from,
                                            pl4Data.vars[i].to);
            std::vector<double> dataVec(pl4Data.steps);
            for (int j = 0; j < pl4Data.steps; ++j) {
                dataVec[j] = static_cast<double>(base[2 + i + j*(pl4Data.nVar+1)]);
            }
            var_name_list.push_back(varName);
            sim_data.push_back(dataVec);
        }
        return true;
    }

    /**
     * @brief 获取所有变量的名称列表
     * @return 变量名称列表
     */
    std::vector<std::string> getVarNameList(void)
    {
        return var_name_list;
    }

    /**
     * @brief 获取某个变量的数据（引用）
     * @param var_name 变量名
     * @return 数据向量引用
     */
    std::vector<double>& getDataOfVar(std::string var_name)
    {
        bool found=false;
        int i;
        for(i=0; i<var_name_list.size(); i++){
            if(var_name_list[i]==var_name){
                found = true;
                break;
            }
        }
        if(found){
            return sim_data[i];
        }
        else{
            return sim_data[0];
        }
    }

private:
    /**
     * @brief 从字节流中读取小端整数
     * 
     * @param buf 字符串指针
     * @return uint32_t 整型数据
     */
    static inline uint32_t readU32LE(const char* buf) {
        uint32_t val;
        std::memcpy(&val, buf, 4);
        return val;
    }


    /**
     * @brief 从字节流中读取小端浮点型数据
     * 
     * @param buf 字符串指针
     * @return float 浮点型数据
     */
    static inline float readFloatLE(const char* buf) {
        float val;
        std::memcpy(&val, buf, 4);
        return val;
    }


    /**
     * @brief 去除字符串末尾的空格和空字符
     * 
     * @param s 待处理字符串
     * @return std::string 已处理字符串
     */
    static std::string trimRight(const std::string& s) {
        auto end = s.find_last_not_of(" \0");
        if (end == std::string::npos) return "";
        return s.substr(0, end + 1);
    }


    /**
     * @brief 读取PL4文件
     * 
     * @param filename  文件全路径名称，[例] C:/example.pl4
     * @param result    结果数据——结构体PL4Data
     * @return true     读取正常
     * @return false    读取异常
     */
    static bool readPL4(const std::string& filename, PL4Data& result) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return false;
        }
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        file.seekg(40);
        char buf4[4];
        file.read(buf4, 4);
        if (file.gcount() != 4) return false;
        result.deltaT = readFloatLE(buf4);
        file.seekg(48);
        file.read(buf4, 4);
        if (file.gcount() != 4) return false;
        uint32_t tmp = readU32LE(buf4);
        result.nVar = static_cast<int>(tmp / 2);
        file.seekg(56);
        file.read(buf4, 4);
        if (file.gcount() != 4) return false;
        uint32_t pl4sizeRaw = readU32LE(buf4);
        result.vars.clear();
        result.vars.reserve(result.nVar);
        const std::streampos headerStart = 5 * 16;
        for (int i = 0; i < result.nVar; ++i) {
            std::streampos pos = headerStart + static_cast<std::streamoff>(i * 16);
            file.seekg(pos);
            char header[16];
            file.read(header, 16);
            if (file.gcount() != 16) return false;
            int type = static_cast<unsigned char>(header[3]) - '0';
            std::string from(header + 4, 6);
            std::string to(header + 10, 6);
            from = trimRight(from);
            to = trimRight(to);
            result.vars.push_back({type, from, to});
        }
        std::streampos searchStart = headerStart + static_cast<std::streamoff>(result.nVar * 16);
        std::streampos dataOffset = searchStart;
        bool found = false;
        const int maxSearchBytes = 10000;
        for (int offset = 0; offset < maxSearchBytes; offset += 4) {
            file.seekg(searchStart + static_cast<std::streamoff>(offset));
            float f;
            file.read((char*)&f, 4);
            if (file.gcount() != 4) break;
            if (f != 0.0f) {
                if (offset >= 4) {
                    file.seekg(searchStart + static_cast<std::streamoff>(offset - 4));
                    float prev;
                    file.read((char*)&prev, 4);
                    if (prev == 0.0f) {
                        dataOffset = searchStart + static_cast<std::streamoff>(offset - 4);
                    } else {
                        dataOffset = searchStart + static_cast<std::streamoff>(offset);
                    }
                } else {
                    dataOffset = searchStart + static_cast<std::streamoff>(offset);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "未找到有效数据起始位置" << std::endl;
            return false;
        }
        std::streampos remainingBytes = fileSize - dataOffset;
        if (remainingBytes <= 0) {
            std::cerr << "数据起始位置超出文件" << std::endl;
            return false;
        }
        size_t bytesPerStep = (result.nVar + 1) * sizeof(float);
        size_t estimatedSteps = static_cast<size_t>(remainingBytes) / bytesPerStep;
        if (estimatedSteps <= 0) {
            std::cerr << "剩余字节不足以容纳一个完整时间步" << std::endl;
            return false;
        }
        result.steps = static_cast<int>(estimatedSteps);
        result.tMax = (result.steps - 1) * result.deltaT;
        size_t totalFloats = static_cast<size_t>(result.steps) * (result.nVar + 1);
        result.data.resize(totalFloats);
        file.seekg(dataOffset);
        file.read(reinterpret_cast<char*>(result.data.data()), totalFloats * sizeof(float));
        if (file.gcount() != static_cast<std::streamsize>(totalFloats * sizeof(float))) {
            std::cerr << "数据读取不完整: 预期 " << totalFloats * sizeof(float)
                    << " 字节, 实际读取 " << file.gcount() << " 字节" << std::endl;
            return false;
        }
        return true;
    }
    // 从 PL4 变量信息生成字符串名称（模仿 Pl42mat.exe 的命名规则）
    static std::string makeVarName(int type, const std::string& from, const std::string& to)
    {
        switch (type) {
        case 4:  // 节点电压 (V-node)
            return "v:" + from;
        case 7:  // 分支电压 (E-bran)
            return "E" + from + "-" + to;
        case 8:  // 分支电压差 (V-bran)
            return "V" + from + "-" + to;
        case 9:  // 分支电流 (I-bran)
            return "c:" + from + "-" + to;
        default:
            return std::to_string(type) + "_" + from + "_" + to;
        }
    }
    // 仿真数据
    std::vector<std::vector<double>> sim_data;
    // 变量名称列表
    std::vector<std::string> var_name_list; 
};

#endif // READPL4_H