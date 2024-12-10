#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

#define MAX_PATTERN_LENGTH 1000
#define MAX_TEXT_LENGTH 1000000000000000000

// 随机生成指定长度的字符串
std::string generateRandomString(long long length) {
    std::string result;
    for (int i = 0; i < length; i++) {
        char randomChar = 'a' + rand() % 26;  // 生成 a-z 范围的随机字符
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
    const size_t textLength = std::atoll(argv[2]); // 每个文件的文本长度
    const size_t  patternLength = std::atoi(argv[3]);  // 每个文件的模式长度
    const size_t  numInserts = std::atoi(argv[4]);       // 每个文件中插入的模式数量

    std::cout<<textLength<<patternLength<<numInserts<<std::endl;

    if (textLength > MAX_TEXT_LENGTH || patternLength > MAX_PATTERN_LENGTH || numInserts > (textLength/patternLength)){
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

        // 分割文件，分别输入文件，防止内存爆掉
        const long long maxPerInput = 1000000000;
        int InputCount = textLength/maxPerInput;
        if(textLength % maxPerInput != 0) {
            InputCount = InputCount++;
            
        }

        // 生成随机模式
        std::string pattern = generateRandomString(patternLength);

        for(int j=0;j<InputCount-1;j++) {
            std::cout << j << std::endl;
            std::string text = generateRandomString(textLength);
            int new_insert = rand() % numInserts;
            // 随机插入模式串到文本中
            std::string modifiedText = insertPatternIntoText(text, pattern, new_insert);
            outFile << modifiedText; // 写入文本
            std::cout << j << std::endl;
        }

        std::string text = generateRandomString(textLength);
        int new_insert = rand() % numInserts;
        // 随机插入模式串到文本中
        if(textLength % maxPerInput == 0)
        {std::string modifiedText = insertPatternIntoText(text, pattern, new_insert);
        outFile << modifiedText << std::endl;} // 写入文本
        else {
            outFile << text << std::endl;
        }
        // 生成随机文本




        // 将生成的文本和模式写入文件

        outFile << pattern << std::endl;      // 写入模式

        outFile.close();
    }

    std::cout << "Generated " << numFiles << " test case files." << std::endl;
    return 0;
}
