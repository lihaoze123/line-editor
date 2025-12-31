#include "editor.h"
#include "error.h"
#include <iostream>
#include <string>
#include <exception>
#include <new>

using namespace line_editor;

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " [输入文件] [输出文件]\n";
    std::cout << "\n参数:\n";
    std::cout << "  输入文件     - 可选的要编辑的输入文件（空表示新建文件）\n";
    std::cout << "  输出文件     - 用于保存结果的输出文件\n";
    std::cout << "\n示例:\n";
    std::cout << "  " << programName << " input.txt output.txt\n";
}

int main(int argc, char* argv[]) {
    try {
        std::string inputFile, outputFile;

        if (argc > 1) {
            if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
                printUsage(argv[0]);
                return 0;
            }
            inputFile = argv[1];
        }
        if (argc > 2) {
            outputFile = argv[2];
        }

        if (inputFile.empty() && outputFile.empty()) {
            std::cout << "未指定文件。请输入输入文件名（留空表示无）: ";
            std::getline(std::cin, inputFile);

            if (!inputFile.empty()) {
                std::cout << "请输入输出文件名: ";
                std::getline(std::cin, outputFile);
            }
        }

        Editor editor;

        if (!editor.init(inputFile, outputFile)) {
            std::cerr << "初始化编辑器失败。\n";
            return 1;
        }

        editor.run();

        return 0;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "致命错误: 内存不足 - " << e.what() << "\n";
        std::cerr << "请保存您的工作并关闭其他应用程序。\n";
        return 2;
    }
    catch (const EditorException& e) {
        std::cerr << "编辑器错误: " << e.what() << "\n";
        return 3;
    }
    catch (const std::exception& e) {
        std::cerr << "意外错误: " << e.what() << "\n";
        return 4;
    }
    catch (...) {
        std::cerr << "发生了未知的致命错误。\n";
        return 5;
    }
}
