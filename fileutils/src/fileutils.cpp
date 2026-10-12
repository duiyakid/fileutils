#include "pch.h"
#include "fileutils.h"
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>
#include <stdexcept>   // std::runtime_error

// 声明 fs 命名空间（C++17 filesystem 必须）
namespace fs = std::filesystem; 
namespace fu = FileUtils;

// ====================== 成员函数实现 ======================

namespace FileUtils {

    fs::path exe_path() {
        std::wstring buf(MAX_PATH, L'\0');
        DWORD n = 0;
        while (true) {
            n = GetModuleFileNameW(nullptr, buf.data(), (DWORD)buf.size());
            if (n == 0) throw std::runtime_error("GetModuleFileNameW 失败");
            if (n < buf.size()) {
                buf.resize(n);
                break;
            }
            buf.resize(buf.size() * 2);   // 路径超长，扩大再试
        }
        return fs::path(buf);
    }

    // ============ 1b. exe 所在目录 ============
    fs::path exe_dir() {
        return exe_path().parent_path();
    }

    // ============ 2. 目录 ============
    fs::path path_dir(const fs::path& p) {
        return p.parent_path();          // 可能为空（相对文件名）
    }

    // ============ 3. 文件名（带扩展） ============
    fs::path path_filename(const fs::path& p) {
        return p.filename();             // "file.tar.gz"
    }

    // ============ 4. 真文件名（主名，去所有扩展名） ============
    std::string path_basename(const fs::path& p) {
        std::string name = p.filename().string();
        auto pos = name.find('.');       // 第一个 '.' 之前
        return (pos == std::string::npos) ? name : name.substr(0, pos);
    }

    // ============ 5. 扩展名 ============
    // 5a. 最后一个扩展名：".gz"
    std::string path_ext(const fs::path& p) {
        return p.extension().string();
    }

    // 5b. 全部扩展名：".tar.gz"
    std::string path_all_ext(const fs::path& p) {
        std::string name = p.filename().string();
        auto pos = name.find('.');
        return (pos == std::string::npos) ? "" : name.substr(pos);
    }

    // ============ 6. 完整路径 ============
    fs::path full_path(const fs::path& p) {
        return fs::absolute(p);          // 转绝对路径
    }

} // namespace FileDLL

// ====================== 目录 / 文件操作 ======================

// 读取当前目录文件：增加异常捕获，避免崩溃
void fu::list_cwd_files_path(std::vector<fs::path>& file_vector) {
    fs::path work_path = fs::current_path();
    file_vector.clear();  // 清空旧数据，避免重复
    // 遍历当前目录（非递归，仅当前层）
    try {
        for (const auto& entry : fs::directory_iterator(work_path)) {
            // 只保留普通文件（跳过文件夹、符号链接等）
            if (fs::is_regular_file(entry)) {
                // 存入「仅文件名+后缀」（如 "photo.jpg"，不含路径）
                file_vector.push_back(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[读取目录失败] " << e.what() << std::endl;
    }
}

// 提取纯数字串：截取前14位，避免超长
std::vector<std::string> fu::digits_from_files(
    const std::vector<fs::path>& file_vector) {
    std::vector<std::string> pureNums;
    fs::path work_path = fs::current_path();
    std::string currentNum;
    for (const auto& filename : file_vector) {
        currentNum.clear();
        // 遍历文件名的「主干」（不含后缀），提取纯数字
        for (char ch : filename.stem().string()) {
            unsigned char c = static_cast<unsigned char>(ch);
            if (isdigit(c)) {
                currentNum += c;
            }
        }
        if (!currentNum.empty()) {
            pureNums.push_back(currentNum);
        }
        // 找到14位数字串（YYYYMMDDHHMMSS），调用写入 EXIF
        if (currentNum.length() >= 14) {
            // 传入「完整文件路径」（当前目录 + 文件名）
            std::string fullFilePath =
                work_path.string() + "\\" + filename.string();
        }
        else {
            std::cout << "[跳过] 文件名中未找到14位数字串，无法写入EXIF: "
                << filename.string() << std::endl;
        }
    }

    std::cout << std::endl;  // 换行，格式更整洁
    return pureNums;
}

// ====================== 编码转换 ======================

// UTF-8 转宽字符（替代废弃的 wstring_convert）
std::wstring fu::utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) return {};

    // 获取所需宽字符长度（不含结束符）
    int wideLen = MultiByteToWideChar(
        CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), nullptr, 0
    );
    if (wideLen <= 0) {
        std::cerr << "[UTF-8转宽字符失败] " << utf8Str
            << " 错误码：" << GetLastError() << std::endl;
        return {};
    }

    std::wstring wideStr(wideLen, L'\0');
    MultiByteToWideChar(
        CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(),
        &wideStr[0], wideLen
    );
    return wideStr;
}

// UTF-8 转 ANSI（适配 Windows API）
std::string fu::utf8ToAnsi(const std::string& utf8Str) {
    // 第一步：UTF-8 → 宽字符串（自定义函数，无废弃接口）
    std::wstring wideStr = utf8ToWide(utf8Str);
    if (wideStr.empty()) {
        std::cerr << "[编码转换失败] UTF-8转宽字符失败，使用原路径" << std::endl;
        return utf8Str;
    }

    // 第二步：宽字符串 → ANSI（中文系统为 GBK，不含结束符）
    int ansiLen = WideCharToMultiByte(
        CP_ACP, 0, wideStr.c_str(), (int)wideStr.size(),
        nullptr, 0, nullptr, nullptr
    );
    if (ansiLen <= 0) {
        std::cerr << "[宽字符转ANSI失败] " << utf8Str
            << " 错误码：" << GetLastError() << std::endl;
        return utf8Str;
    }

    std::string ansiStr(ansiLen, 0);
    WideCharToMultiByte(
        CP_ACP, 0, wideStr.c_str(), (int)wideStr.size(),
        &ansiStr[0], ansiLen, nullptr, nullptr
    );
    return ansiStr;
}

// 短路径转换函数（适配中文路径）
std::string fu::getShortPath(const std::string& longPathUtf8) {
    // 步骤1：UTF-8 转 ANSI（适配 Windows API）
    std::string longPathAnsi = utf8ToAnsi(longPathUtf8);
    if (longPathAnsi.empty()) {
        std::cerr << "[路径转换警告] 编码转换失败，使用原路径" << std::endl;
        return longPathUtf8;
    }

    // 步骤2：调用 Windows API 转换短路径
    char shortPathBuffer[MAX_PATH] = { 0 };
    DWORD result = GetShortPathNameA(
        longPathAnsi.c_str(), shortPathBuffer, MAX_PATH
    );

    if (result > 0 && result <= MAX_PATH) {
        return std::string(shortPathBuffer);
    }
    else {
        std::cerr << "[警告] 路径转换失败（错误码：" << GetLastError()
            << "），使用原路径：" << longPathUtf8 << std::endl;
        return longPathUtf8;
    }
}