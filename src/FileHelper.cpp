#include "FileHelper.h"

namespace FileHelper
{
    bool loadFile( std::vector<std::byte> &data, const std::string &filename )
    {
        std::ifstream file( filename, std::ios::binary | std::ios::ate );
        if ( !file.is_open() )
        {
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg( 0, std::ios::beg );

        data.resize( size );

        if ( !file.read( reinterpret_cast<char *>( data.data() ), size ) )
        {
            data.clear();
            return false;
        }
        return true;
    }

    bool isWindowsFilePath( const std::string &url )
    {
        if ( url.size() < 3 )
        {
            return false;
        }
        if ( !std::isalpha( static_cast<unsigned char>( url[0] ) ) )
        {
            return false;
        }
        if ( url[1] != ':' )
        {
            return false;
        }
        if ( url[2] != '/' )
        {
            return false;
        }
        return true;
    }

    bool isUnixFilePath( const std::string &url )
    {
        return !url.empty() && url[0] == '/';
    }

    bool isFile( const std::string &url )
    {
        return url.compare( 0, sizeof( fileProtocol ) - 1, fileProtocol ) == 0;
    }

} // FileHelper