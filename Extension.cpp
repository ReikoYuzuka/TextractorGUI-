#include "Extension.h"
#include <fstream>
#include <string>
#include <vector>

// ȫ�ֱ���
std::ofstream outputFile;
std::vector<std::wstring> sentenceHistory;
int historyIndex = 0;

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // ������ļ���׷��ģʽ��
        outputFile.open("GUI���.txt", std::ios::app | std::ios::out);
        if (outputFile.is_open()) {
            outputFile << "=== ����ʼ===\n";
            outputFile.flush();
        }
        break;

    case DLL_PROCESS_DETACH:
        if (outputFile.is_open()) {
            outputFile << "=== ������� ===\n";
            outputFile.close();
        }
        break;
    }
    return TRUE;
}

bool ProcessSentence(std::wstring& sentence, SentenceInfo sentenceInfo)
{
    // ֻ�����û�ѡ��Ľ����е��ı�
    if (sentenceInfo["current select"])
    {
        // ��ӵ���ʷ��¼
        sentenceHistory.push_back(sentence);
        if (sentenceHistory.size() > 1000) {
            sentenceHistory.erase(sentenceHistory.begin());
        }
        historyIndex = sentenceHistory.size() - 1;

        // д���ļ�
        if (outputFile.is_open()) {
            // ת�����ַ��ַ���Ϊ���ֽ��ַ�����UTF-8��
            int size = WideCharToMultiByte(CP_UTF8, 0, sentence.c_str(), -1, NULL, 0, NULL, NULL);
            if (size > 0) {
                std::string utf8Sentence(size, 0);
                WideCharToMultiByte(CP_UTF8, 0, sentence.c_str(), -1, &utf8Sentence[0], size, NULL, NULL);

                // д���ļ�
                outputFile << utf8Sentence.c_str() << std::endl;
                outputFile.flush(); // ȷ������д�����
            }
        }
    }

    return false; // ���޸�ԭ����
}

extern "C" __declspec(dllexport) wchar_t* OnNewSentence(wchar_t* sentence, const InfoForExtension* sentenceInfo)
{
    try
    {
        // ����Ƿ����û�ѡ����ı�
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
        // ���������쳣����ֹ����
    }

    return sentence;
}