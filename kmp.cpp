#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring> // for std::strlen

#define MAX_PATTERN_LENGTH 1002
#define MAX_TEXT_LENGTH 1000000002

// KMP算法计算部分匹配表（前缀函数）
void computeLPSArray(const char* pattern, int* lps, int m) {
    int length = 0;
    lps[0] = 0;  // LPS[0] 总是 0
    int i = 1;

    while (i < m) {
        if (pattern[i] == pattern[length]) {
            length++;
            lps[i] = length;
            i++;
        } else {
            if (length != 0) {
                length = lps[length - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// KMP字符串匹配算法，返回匹配的数量和位置
int KMPsearch(const char* text, const char* pattern, int* matchPositions) {
    int n = std::strlen(text);
    int m = std::strlen(pattern);
    int matchCount = 0;  // 记录匹配的次数

    if (n == 0 || m == 0 || m > n) {
        std::cerr << "Invalid text or pattern length!" << std::endl;
        return 0;  // 安全检查，防止越界或不合理的输入
    }

    int* lps = new int[m];  // 动态分配LPS数组
    computeLPSArray(pattern, lps, m);

    int i = 0;  // text 的索引
    int j = 0;  // pattern 的索引
    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }

        if (j == m) {
            // 找到匹配，记录匹配的位置
            matchPositions[matchCount++] = i - j;
            j = lps[j - 1];
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    delete[] lps; // 释放动态分配的内存
    return matchCount;  // 返回匹配次数
}

// 主程序：从命令行参数指定文件并运行 KMP
int main(int argc, char* argv[]) {

    clock_t global_start = clock();

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    const char* inputFilename = argv[1];  
    const char* outputFilename = argv[2]; 

    // 动态分配文本和模式串的内存
    char* text = new char[MAX_TEXT_LENGTH + 1];   
    char* pattern = new char[MAX_PATTERN_LENGTH + 1];  
    int* matchPositions = new int[MAX_TEXT_LENGTH]; // 用于存储匹配位置

    // 从文件中读取数据
    std::ifstream inFile(inputFilename);
    if (!inFile) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        delete[] text;
        delete[] pattern;
        delete[] matchPositions;
        return 1;
    }

    inFile.getline(text, MAX_TEXT_LENGTH);
    text[MAX_TEXT_LENGTH] = '\0';  
    inFile.getline(pattern, MAX_PATTERN_LENGTH);
    pattern[MAX_PATTERN_LENGTH] = '\0';  
    inFile.close();

    clock_t start = clock();
    int matchCount = KMPsearch(text, pattern, matchPositions);
    clock_t end = clock();
    double time_taken = double(end - start) / CLOCKS_PER_SEC;

    std::ofstream outFile(outputFilename);
    if (!outFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        delete[] text;
        delete[] pattern;
        delete[] matchPositions;
        return 1;
    }

    outFile << "Time taken by KMP: " << time_taken << " seconds" << std::endl;
    std::cout << "Time taken by KMP: " << time_taken << " seconds" << std::endl;
    outFile << "Total matches found: " << matchCount << std::endl;

    if (matchCount > 0) {
        outFile << "Match positions: ";
        for (int i = 0; i < matchCount; i++) {
            outFile << matchPositions[i] << " ";
        }
        outFile << std::endl;
    } else {
        outFile << "No matches found." << std::endl;
    }

    outFile.close();
    std::cout << "Results written to " << outputFilename << std::endl;

    // 释放动态分配的内存
    delete[] text;
    delete[] pattern;
    delete[] matchPositions;

    clock_t global_end = clock();
    double global_time = double(global_end - global_start)/CLOCKS_PER_SEC;
    std::cout << "Global time: " << global_time << " seconds" << std::endl;

    return 0;
}
