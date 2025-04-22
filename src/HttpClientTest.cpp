#include "HttpClientTest.h"

using namespace godot;

namespace
{
    std::tuple<std::string, std::string, std::string> extract_url_parts( const std::string &url )
    {
        std::string host;
        std::string port;
        std::string path;

        size_t protocol_end = url.find( "//" ) + 2;
        size_t path_start = url.find( '/', protocol_end );

        std::string host_port = url.substr( protocol_end, path_start - protocol_end );

        size_t port_start = host_port.find( ':' );
        if ( port_start != std::string::npos )
        {
            host = host_port.substr( 0, port_start );
            port = host_port.substr( port_start + 1 );
        }
        else
        {
            host = host_port;
            port = "";
        }

        if ( path_start != std::string::npos )
        {
            path = url.substr( path_start );
        }
        if ( host == "localhost" )
        {
            host = "127.0.0.1";
        }

        return { host, port, path };
    }

    Ref<HTTPClient> httpClient;

} // namespace

namespace CesiumForGodot
{

    namespace HttpUtils
    {

        void request( godot::String url )
        {

            if ( httpClient.is_null() )
            {
                httpClient.instantiate();
            }

            std::string url_( url.utf8().get_data() );
            auto [host, port, path] = extract_url_parts( url_ );
            String host_( host.c_str() );
            int32_t port_( static_cast<int32_t>( std::stoi( port ) ) );
            String path_( path.c_str() );
            HTTPClient::Status status = httpClient->get_status();

            if ( status == HTTPClient::STATUS_DISCONNECTED )
            {
                godot::Error err = httpClient->connect_to_host( host_, port_ );
                if ( err != Error::OK )
                {
                    UtilityFunctions::printerr( "Connect to host failed." );
                }
                status = httpClient->get_status();
                while ( status == HTTPClient::STATUS_CONNECTING ||
                        status == HTTPClient::STATUS_RESOLVING )
                {
                    // UtilityFunctions::print("current status = ", status, ", target status is ",
                    // HTTPClient::STATUS_CONNECTED);
                    httpClient->poll();
                    UtilityFunctions::print( "Connecting..." );
                    status = httpClient->get_status();
                    // UtilityFunctions::print("update status = ", status);
                }
            }

            PackedStringArray packed_headers;
            packed_headers.push_back( "X-Cesium-Client: Cesium For Godot" );
            packed_headers.push_back( "user-agent: Pirulo/1.0 (Godot)" );
            packed_headers.push_back( "Accept: */*" );

            UtilityFunctions::print( "HttpClient Params->", "path: ", path_,
                                     " headers: ", packed_headers );

            godot::Error err = httpClient->request( HTTPClient::METHOD_GET, path_, packed_headers );

            UtilityFunctions::print( "httpclient request err = ", err );
            if ( err != Error::OK )
            {
                UtilityFunctions::printerr( "Request failed." );
            }

            while ( httpClient->get_status() == HTTPClient::STATUS_REQUESTING )
            {
                httpClient->poll();
                UtilityFunctions::print( "Requesting..." );
            }

            status = httpClient->get_status();
            UtilityFunctions::print( " request ok, status -> ", status );

            if ( status == HTTPClient::STATUS_CONNECTED || status == HTTPClient::STATUS_BODY )
            {
                if ( httpClient->has_response() )
                {
                    UtilityFunctions::print( "Response code: ", httpClient->get_response_code() );
                    if ( httpClient->is_response_chunked() )
                    {
                        UtilityFunctions::print( "Response is chunked." );
                    }
                    else
                    {
                        int64_t bl = httpClient->get_response_body_length();
                        UtilityFunctions::print( "Response Length: ", bl );
                    }

                    PackedByteArray rb;
                    while ( httpClient->get_status() == HTTPClient::STATUS_BODY )
                    {
                        httpClient->poll();
                        PackedByteArray chunk = httpClient->read_response_body_chunk();
                        if ( chunk.size() != 0 )
                        {
                            rb.append_array( chunk );
                        }
                    }

                    UtilityFunctions::print( "bytes got: ", rb.size() );
                }
            }
        }

    } // namespace HttpUtils

} // namespace CesiumForGodot
