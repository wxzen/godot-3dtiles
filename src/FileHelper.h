#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <cstddef> // for std::byte
#include <fstream>
#include <system_error> // for std::error_code and std::make_error_code
#include <vector>

namespace FileHelper
{

    bool loadFile( std::vector<std::byte> &data, const std::string &filename );

    bool isWindowsFilePath( const std::string &url );

    bool isUnixFilePath( const std::string &url );

    const char fileProtocol[] = "file:///";
    bool isFile( const std::string &url );

} // namespace FileHelper

#endif // FILEHELPER_H