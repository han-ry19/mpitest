#include <iostream>
#include <fstream>
#include <vector>

// 从文件中读取匹配结果
bool readResultFromFile(const char* filename, int& matchCount, std::vector<int>& matchPositions) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error opening result file: " << filename << std::endl;
        return false;
    }

    std::string label;
    inFile >> label >> label >> label >> label >> label >> label;
    inFile >> label >> label >> label >> matchCount;  // 读取匹配总数

    if (matchCount > 0) {
        inFile >> label >> label;  // 跳过 "Match positions:" 标签
        matchPositions.resize(matchCount);
        for (int i = 0; i < matchCount; i++) {
            inFile >> matchPositions[i];
        }
    }

    inFile.close();
    return true;
}

// 比较两个结果是否一致
bool compareResults(const char* file1, const char* file2) {
    int matchCount1, matchCount2;
    std::vector<int> matchPositions1, matchPositions2;

    // 读取第一个文件的结果
    if (!readResultFromFile(file1, matchCount1, matchPositions1)) return false;

    // 读取第二个文件的结果
    if (!readResultFromFile(file2, matchCount2, matchPositions2)) return false;

    // 比较匹配数量
    if (matchCount1 != matchCount2) {
        std::cout << "Mismatch in match count!" << std::endl;
        return false;
    }

    // 比较匹配位置
    for (int i = 0; i < matchCount1; i++) {
        if (matchPositions1[i] != matchPositions2[i]) {
            std::cout << "Mismatch in match positions at index " << i << "!" << std::endl;
            return false;
        }
    }

    std::cout << "Results are consistent." << std::endl;
    return true;
}

// 主程序：验证两个文件的结果是否一致
int main(int argc, char** argv) {

    if (argc!=3)
    {
         std::cerr << "Usage: " << argv[0] << " <input_file_1> <input_file_2>" << std::endl;
    }

    const char* resultFile1 = argv[1];
    const char* resultFile2 = argv[2];  // 另一个版本的结果文件

    if (compareResults(resultFile1, resultFile2)) {
        std::cout << "Both results are identical." << std::endl;
    } else {
        std::cout << "Results differ." << std::endl;
    }

    return 0;
}
