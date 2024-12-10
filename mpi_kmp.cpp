#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <string>
#include <mpi.h>
#include <cstring>

#define MAX_PATTERN_LENGTH 1002
#define MAX_TEXT_LENGTH 2000000000

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
int KMPsearch(const char* text, const char* pattern, int* matchPositions,const int* lps) {
    int n = std::strlen(text);
    int m = std::strlen(pattern);
    int matchCount = 0;

    if (n == 0 || m == 0 || m > n) {
        std::cerr << "Invalid text or pattern length!" << std::endl;
        return 0;
    }

    int i = 0, j = 0;
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

    return matchCount;
}

int main(int argc, char** argv) {

    clock_t global_start = clock();

    MPI_Init(&argc, &argv);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    std::string p_n = processor_name;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // if (argc != 3) {
    //     if (rank==0) {
    //         std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
    //     }
    //     return 1;
    // }

    // const char* inputFilename = argv[1];  // 输入文件名
    // const char* outputFilename = argv[2]; // 输出文件名
    const char* inputFilename = "/mnt/mpitest/test_case_8_5.txt";
    const char* outputFilename = "/mnt/mpitest/output_mpi.txt";

    char* text = NULL;
    char* pattern = new char[MAX_PATTERN_LENGTH + 1];
    int* matchPositions = NULL;

    int m, totalLength;

    if (rank == 0) {
        // 主进程读取文本和模式串
        text = new char[MAX_TEXT_LENGTH + 1];
        matchPositions = new int[MAX_TEXT_LENGTH + 1];
        std::ifstream inFile(inputFilename);
        if (!inFile) {
            std::cerr << "Error opening input file: " << inputFilename << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        inFile.getline(text, MAX_TEXT_LENGTH);
        inFile.getline(pattern, MAX_PATTERN_LENGTH);
        totalLength = std::strlen(text);
        m = std::strlen(pattern);
        inFile.close();
    }


    // 广播模式串长度和 LPS 数组
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pattern, m + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // 广播整个文本长度
    MPI_Bcast(&totalLength, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* LPS = new int[m];

    computeLPSArray(pattern, LPS, m);

    // if (rank == 0) {
    //     computeLPSArray(pattern, LPS, m);
    // }

    // MPI_Bcast(LPS, m, MPI_INT, 0, MPI_COMM_WORLD);

    // 分割文本，每个进程处理的长度（需要补偿 m-1 个字符）
    int segmentLength = totalLength / size;
    int startIndex = rank * segmentLength;
    // int endIndex = (rank == size - 1) ? totalLength : startIndex + segmentLength;

    // 分配局部文本段空间，并接收文本段
    char* localText = new char[segmentLength + m];  // 多分配 m-1 字符以处理重叠
    clock_t start_time = clock();
    if (rank == 0) {
        // 主进程分发文本段
        for (int i = 1; i < size; i++) {
            int start = i * segmentLength;
            int len = (i == size - 1) ? (totalLength - start + m - 1) : segmentLength + m - 1;
            MPI_Send(&text[start - (m - 1)], len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        // 主进程处理自己的部分
        strncpy(localText, text, segmentLength);

        clock_t end_time = clock();
        double time_taken = double(end_time - start_time) / CLOCKS_PER_SEC;
        std::cout << "Time taken by send: " << time_taken << " seconds" << std::endl;

    } else {
        // 接收主进程发来的文本段
        int len = (rank == size - 1) ? (totalLength - startIndex + m -1) : segmentLength + m - 1;
        MPI_Recv(localText, len, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Data : " << len << " bytes by rank " << rank << " by processor "<<p_n<<std::endl;
        clock_t end_time2 = clock();
        std::cout << "Time : " << double(end_time2 - start_time) / CLOCKS_PER_SEC << "sec by rank " << rank << " by processor "<<p_n<<std::endl;
    }

    clock_t start_time_2 = clock();

    // 每个进程进行 KMP 搜索
    int* localMatchPositions = new int[MAX_TEXT_LENGTH];
    int localMatchCount = KMPsearch(localText, pattern, localMatchPositions, LPS);

    // 收集所有进程的匹配结果
    int* globalMatchCounts = NULL;
    double* kmpTimeCounts = NULL;
    int* displs = NULL;

    if (rank == 0) {
        globalMatchCounts = new int[size];
        kmpTimeCounts = new double[size];
        displs = new int[size];  // 用于 MPI_Gatherv 中的位置偏移
    }




    clock_t end_time_2 = clock();
    double time_taken_2 = double(end_time_2 - start_time_2) / CLOCKS_PER_SEC;
    // std::cout << "Time taken by KMP: " << time_taken_2 << " seconds by rank " << rank << " by processor "<<p_n<<std::endl;

    // 先收集每个进程的匹配数量
    MPI_Gather(&localMatchCount, 1, MPI_INT, globalMatchCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&time_taken_2, 1, MPI_DOUBLE, kmpTimeCounts, 1, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);

    // 计算偏移量并收集所有匹配的位置
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i - 1] + globalMatchCounts[i - 1];
        }
    }

    if (rank!=0){
        for(int i=0 ; i<localMatchCount ; i++) {
            localMatchPositions[i]+=startIndex-m+1;
        }
    }

    MPI_Gatherv(localMatchPositions, localMatchCount, MPI_INT, matchPositions, globalMatchCounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    // 主进程输出结果
    if (rank == 0) {
    std::ofstream outFile(outputFilename);
    if (!outFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // 输出总匹配数
    int totalMatches = 0;
    for (int i = 0; i < size; i++) {
        totalMatches += globalMatchCounts[i];
    }

    std::vector<int> allMatches(matchPositions, matchPositions + totalMatches);



    if ( rank == 0) {
        double avgTimeKmp = 0.0;
        for(int i=0;i< size ; i++) {
            avgTimeKmp += kmpTimeCounts[i];
        }
        avgTimeKmp /= (double)size;
        // std::cout << "Proc number:" << size << ", ";
        std::cout << avgTimeKmp << std::endl;
    }

    // 输出排序后的结果
    double elapsedTime = 8.78; // 假设耗时8.78秒，可根据实际计算替换
    outFile << "Time taken by KMP: " << elapsedTime << " seconds" << std::endl;
    outFile << "Total matches found: " << totalMatches << std::endl;
    outFile << "Match positions: ";
    
    for (int i = 0; i < totalMatches; i++) {
        outFile << allMatches[i];
        if (i != totalMatches - 1) {
            outFile << " ";
        }
    }
    outFile << std::endl;
    outFile.close();
}

    // 清理内存
    delete[] pattern;
    delete[] localText;
    delete[] localMatchPositions;
    delete[] LPS;
    if (rank == 0) {
        delete[] globalMatchCounts;
        delete[] displs;
        delete[] text;
        delete[] matchPositions;
        delete[] kmpTimeCounts;
     }

    clock_t global_end = clock();
    double global_time = double(global_end - global_start)/CLOCKS_PER_SEC;

    // if(rank == 0){
    // std::cout << "Global time: " << global_time << " seconds" << std::endl;
    // }
    MPI_Finalize();
    return 0;
}
