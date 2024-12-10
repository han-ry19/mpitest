#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

#define MAX_PATTERN_LENGTH 1000
#define MAX_TEXT_LENGTH 10000000000

// 随机生成指定长度的字符串
std::string generateRandomString(int length) {
    std::string result;
    for (int i = 0; i < length; i++) {
        char randomChar = '0' + rand() % 2;  // 生成 0-1 范围的随机字符
        result += randomChar;
    }
    return result;
}

// 在随机生成的文本中插入模式串
std::string insertPatternIntoText(const std::string& text, const std::string& pattern, int numInserts) {
    std::string modifiedText = text;  // 拷贝一份原始文本
    int textLength = modifiedText.length();
    
    for (int i = 0; i < numInserts; i++) {
        // 随机选择插入位置
        int insertPos = rand() % (textLength - pattern.length());
        modifiedText.replace(insertPos, pattern.length(), pattern);  // 插入模式串
    }
    
    return modifiedText;
}

// 主程序：生成多个测试用例
int main(int argc, char* argv[]) {

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <num_files> <text_length> <pattern_length> <num_inserts>" << std::endl;
        return 1;
    }


    srand(static_cast<unsigned int>(time(0)));  // 初始化随机数种子

    const int numFiles = std::atoi(argv[1]);        // 生成 10 个文件
    const int textLength = std::atoi(argv[2]); // 每个文件的文本长度
    const int patternLength = std::atoi(argv[3]);  // 每个文件的模式长度
    const int numInserts = std::atoi(argv[4]);       // 每个文件中插入的模式数量

    if (textLength>MAX_TEXT_LENGTH || patternLength > MAX_PATTERN_LENGTH || numInserts > (textLength/patternLength)){
        std::cerr << "Invalid parameters" << std::endl;
        return 1;
    }

    for (int i = 0; i < numFiles; i++) {
        char filename[30]; // 用于存储文件名
        sprintf(filename, "test_case_%d.txt", i); // 生成文件名

        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error creating file!" << std::endl;
            return 1;
        }

        // 生成随机模式
        std::string pattern = generateRandomString(patternLength);
        // 生成随机文本
        std::string text = generateRandomString(textLength);

        int new_insert = rand() % numInserts;
        // 随机插入模式串到文本中
        std::string modifiedText = insertPatternIntoText(text, pattern, new_insert);

        // 将生成的文本和模式写入文件
        outFile << modifiedText << std::endl; // 写入文本
        outFile << pattern << std::endl;      // 写入模式

        outFile.close();
    }

    std::cout << "Generated " << numFiles << " test case files." << std::endl;
    return 0;
}
