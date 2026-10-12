#pragma once
#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>  
#include <vector>

namespace fs = std::filesystem;

#ifdef FILEUTILS_EXPORTS
#define FILEUTILS_API __declspec(dllexport)
#else
#define FILEUTILS_API __declspec(dllimport)
#endif

namespace FileUtils {

    // ====================== 路径解析工具 ======================

    // 当前 exe 所在路径
	FILEUTILS_API fs::path exe_path();

	// 当前 exe 所在目录
    FILEUTILS_API fs::path exe_dir();

    // path所在目录（父路径）
    FILEUTILS_API fs::path path_dir(const fs::path& p);

    // path的文件名（带扩展，如 "file.tar.gz"）
    FILEUTILS_API fs::path path_filename(const fs::path& p);

    // path的文件基本名（主名，去所有扩展名，如 "file"）
    FILEUTILS_API std::string path_basename(const fs::path& p);

    // path的最后一个扩展名（如 ".gz"）
    FILEUTILS_API std::string path_ext(const fs::path& p);

    // path的全部扩展名（如 ".tar.gz"）
    FILEUTILS_API std::string path_all_ext(const fs::path& p);

    // path完整绝对路径
    FILEUTILS_API fs::path     full_path(const fs::path& p);

    // ====================== 目录 / 文件操作 ======================

    // 读取当前目录下的普通文件列表（仅文件名，不含路径）
    FILEUTILS_API void list_cwd_files_path(std::vector<fs::path>& file_vector);

    // 从文件名中提取纯数字串（前 14 位用于 EXIF）
    FILEUTILS_API std::vector<std::string> digits_from_files(
        const std::vector<fs::path>& file_vector);

    // ====================== 编码转换 ======================

    // UTF-8 → 宽字符（替代废弃的 wstring_convert）
    FILEUTILS_API std::wstring utf8ToWide(const std::string& utf8Str);

    // UTF-8 → ANSI（中文系统为 GBK）
    FILEUTILS_API std::string utf8ToAnsi(const std::string& utf8Str);

    // 长路径 → 短路径（8.3 格式），中文路径可用
    FILEUTILS_API std::string getShortPath(const std::string& longPathUtf8);
}

#endif // !FILEUTILS_H