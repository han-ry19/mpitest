#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <string>
#include <mpi.h>
#include <cstring>

#define MAX_PATTERN_LENGTH 1002
#define MAX_TEXT_LENGTH 10000000000

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
    const char* inputFilename = "/mnt/mpitest/test_case_0.txt";
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
        // std::cout << m << totalLength;
        inFile.close();
    }

    // 广播模式串长度和 LPS 数组
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pattern, m + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    int* LPS = new int[m];

    if (rank == 0) {
        computeLPSArray(pattern, LPS, m);
    }

    MPI_Bcast(LPS, m, MPI_INT, 0, MPI_COMM_WORLD);

    // 广播整个文本长度
    MPI_Bcast(&totalLength, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int segmentLength;

    if (totalLength%size == 0)
        segmentLength = totalLength / size;
    else
        segmentLength = totalLength / size + 1;

    int startIndex = rank * segmentLength;
    int loc_len = (rank == size - 1) ? (totalLength - startIndex) : segmentLength;

    char* localText = new char[segmentLength + m]; 

    MPI_File file;
    MPI_File_open(MPI_COMM_WORLD, inputFilename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

    // 获取文件的大小
    MPI_Offset file_size;
    MPI_File_get_size(file, &file_size);

    MPI_Offset start_offset = startIndex;

    // 计算要读取的数据大小

    // 读取文件的相应部分
    MPI_File_read_at(file, start_offset,(rank==0 ? localText : localText+m-1), loc_len, MPI_CHAR, MPI_STATUS_IGNORE);

    // 关闭文件
    MPI_File_close(&file);


    //  //传递文本串的重叠部分
    char* send_value = localText + loc_len - m;

    // 发送给下一个进程
    int next_rank = (rank + 1) % size;  // 下一个进程的rank，最后一个进程将向第一个进程发送数据
    int prev_rank = (rank - 1 + size) % size;  // 上一个进程的rank，考虑循环的情况

    if (rank != 0) {
        // 进程1以外的进程接收并发送数据
        MPI_Recv(localText, m-1, MPI_CHAR, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // std::cout << "进程 " << rank << " 收到来自进程 " << prev_rank << " 的数据 " << std::endl;
    }

    // 所有进程发送数据给下一个进程
    MPI_Send(send_value, m-1, MPI_CHAR, next_rank, 0, MPI_COMM_WORLD);

    // // 第一个进程不接收数据，只负责发送
    // if (rank == 0) {
    //     std::cout << "进程 " << rank << " 不接收数据，直接发送给进程 " << next_rank << std::endl;
    // }

    // std::cout << "进程 " << rank << " 读取的数据: ";
    // for (int i = 0; i < (rank==0 ? loc_len : loc_len+m-1); ++i) {  // 只输出前100个字符
    //     std::cout << localText[i];
    // }
    // std::cout << std::endl;

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

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    std::string p_n = processor_name;

    // clock_t end_time_2 = clock();
    // double time_taken_2 = double(end_time_2 - start_time_2) / CLOCKS_PER_SEC;
    // std::cout << "Time taken by KMP: " << time_taken_2 << " seconds by rank " << rank << " by processor "<<p_n<<std::endl;

    // 先收集每个进程的匹配数量
    MPI_Gather(&localMatchCount, 1, MPI_INT, globalMatchCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Gather(&time_taken_2, 1, MPI_DOUBLE, kmpTimeCounts, 1, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);

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

    // std::cout << totalMatches << std::endl;

    // for(int i = 0 ; i<totalMatches ;i++)
    // {
    //     std::cout << allMatches[i] << " ";
    // }
    // std::cout << std::endl;

    // if ( rank == 0) {
    //     double avgTimeKmp = 0.0;
    //     for(int i=0;i< size ; i++) {
    //         avgTimeKmp += kmpTimeCounts[i];
    //     }
    //     avgTimeKmp /= (double)size;
    //     std::cout << "Proc number:" << size << ", ";
    //     std::cout << "Time: " << avgTimeKmp << "seconds" << std::endl;
    // }

    outFile << totalMatches << std::endl;
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

    if(rank == 0){
    std::cout << global_time << std::endl;
    }

    MPI_Finalize();
    return 0;
}