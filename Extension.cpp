#include "Extension.h"
#include <fstream>
#include <string>
#include <vector>

// 全局变量
std::ofstream outputFile;
std::vector<std::wstring> sentenceHistory;
int historyIndex = 0;

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // 打开输出文件（追加模式）
        outputFile.open("GUI输出.txt", std::ios::app | std::ios::out);
        if (outputFile.is_open()) {
            outputFile << "=== 捕获开始===\n";
            outputFile.flush();
        }
        break;

    case DLL_PROCESS_DETACH:
        if (outputFile.is_open()) {
            outputFile << "=== 捕获结束 ===\n";
            outputFile.close();
        }
        break;
    }
    return TRUE;
}

bool ProcessSentence(std::wstring& sentence, SentenceInfo sentenceInfo)
{
    // 只处理用户选择的进程中的文本
    if (sentenceInfo["current select"])
    {
        // 添加到历史记录
        sentenceHistory.push_back(sentence);
        if (sentenceHistory.size() > 1000) {
            sentenceHistory.erase(sentenceHistory.begin());
        }
        historyIndex = sentenceHistory.size() - 1;

        // 写入文件
        if (outputFile.is_open()) {
            // 转换宽字符字符串为多字节字符串（UTF-8）
            int size = WideCharToMultiByte(CP_UTF8, 0, sentence.c_str(), -1, NULL, 0, NULL, NULL);
            if (size > 0) {
                std::string utf8Sentence(size, 0);
                WideCharToMultiByte(CP_UTF8, 0, sentence.c_str(), -1, &utf8Sentence[0], size, NULL, NULL);

                // 写入文件
                outputFile << utf8Sentence.c_str() << std::endl;
                outputFile.flush(); // 确保立即写入磁盘
            }
        }
    }

    return false; // 不修改原句子
}

extern "C" __declspec(dllexport) wchar_t* OnNewSentence(wchar_t* sentence, const InfoForExtension* sentenceInfo)
{
    try
    {
        // 检查是否是用户选择的文本
        bool isSelected = false;
        for (auto info = sentenceInfo; info->name; ++info) {
            if (std::string(info->name) == "current select" && info->value != 0) {
                isSelected = true;
                break;
            }
        }

        if (isSelected) {
            std::wstring sentenceCopy(sentence);
            ProcessSentence(sentenceCopy, SentenceInfo{ sentenceInfo });
        }
    }
    catch (...) {
        // 忽略所有异常，防止崩溃
    }

    return sentence;
}