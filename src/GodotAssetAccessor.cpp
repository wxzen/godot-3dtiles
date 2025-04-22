#include "GodotAssetAccessor.h"
#include "Cesium.h"
#include "FileHelper.h"

#include <CesiumAsync/IAssetResponse.h>

#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/core/version.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <uriparser/Uri.h>

#include <cstddef>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#define STRINGIFY_HELPER( x ) #x
#define STRINGIFY( x ) STRINGIFY_HELPER( x )

#define ENGINE_VERSION                                                                             \
    "Godot " STRINGIFY( GODOT_VERSION_MAJOR ) "." STRINGIFY( GODOT_VERSION_MINOR ) "." STRINGIFY(  \
        GODOT_VERSION_PATCH )

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

    class GodotAssetResponse : public CesiumAsync::IAssetResponse
    {
    public:
        GodotAssetResponse( CesiumForGodot::AHttpResponse response ) : _pResponse( response )
        {
            this->_statusCode = static_cast<uint16_t>( response.code );
            godot::Dictionary responseHeaders = response.headers;
            if ( !responseHeaders.is_empty() )
            {
                godot::Array keys = responseHeaders.keys();
                for ( int i = 0; i < keys.size(); ++i )
                {
                    godot::String key = keys[i];
                    std::string key_str = key.utf8().get_data();
                    godot::Variant value = responseHeaders[key];
                    godot::String value_str = value;
                    std::string value_cstr = value_str.utf8().get_data();

                    if ( value.get_type() == godot::Variant::STRING )
                    {
                        this->_headers[key_str] = value_cstr;
                    }

                    if ( key == "Content-Type" )
                    {
                        this->_contentType = value_cstr;
                    }
                }
            }
            this->_data.resize( response.data.size() );
            const uint8_t *data_ptr = reinterpret_cast<const uint8_t *>( response.data.ptr() );
            std::memcpy( this->_data.data(), data_ptr, response.data.size() );
        }

        virtual uint16_t statusCode() const override
        {
            return _statusCode;
        }

        virtual std::string contentType() const override
        {
            return _contentType;
        }

        virtual const CesiumAsync::HttpHeaders &headers() const override
        {
            return _headers;
        }

        virtual std::span<const std::byte> data() const override
        {
            return this->_data;
        }

    private:
        uint16_t _statusCode = 0;
        std::string _contentType;
        CesiumAsync::HttpHeaders _headers;
        std::vector<std::byte> _data;
        CesiumForGodot::AHttpResponse _pResponse;
    };

    class GodotAssetRequest : public CesiumAsync::IAssetRequest
    {
    public:
        GodotAssetRequest( const std::string &method, const std::string &url,
                           const CesiumAsync::HttpHeaders &headers,
                           CesiumForGodot::AHttpResponse pResponse ) :
            _method( method ), _url( url ), _headers( headers ),
            _pResponse( std::make_unique<GodotAssetResponse>( std::move( pResponse ) ) )
        {
        }

        virtual const std::string &method() const override
        {
            return _method;
        }

        virtual const std::string &url() const override
        {
            return _url;
        }

        virtual const CesiumAsync::HttpHeaders &headers() const override
        {
            return _headers;
        }

        virtual const CesiumAsync::IAssetResponse *response() const override
        {
            return _pResponse.get();
        }

    private:
        std::string _method;
        std::string _url;
        CesiumAsync::HttpHeaders _headers;
        std::unique_ptr<GodotAssetResponse> _pResponse;
    };
} // namespace

namespace
{
    std::string convertFileUriToFilename( const std::string &url )
    {
        // According to the uriparser docs, both uriUriStringToWindowsFilenameA and
        // uriUriStringToUnixFilenameA require an output buffer with space for at most
        // length(url)+1 characters.
        // https://uriparser.github.io/doc/api/latest/Uri_8h.html#a4afbc8453c7013b9618259bc57d81a39
        std::string result( url.size() + 1, '\0' );

#ifdef _WIN32
        int errorCode = uriUriStringToWindowsFilenameA( url.c_str(), result.data() );
#else
        int errorCode = uriUriStringToUnixFilenameA( url.c_str(), result.data() );
#endif

        // Truncate the string if necessary by finding the first null character.
        size_t end = result.find( '\0' );
        if ( end != std::string::npos )
        {
            result.resize( end );
        }

        // Remove query parameters from the URL if present, as they are no longer
        // ignored by Unreal.
        size_t pos = result.find( "?" );
        if ( pos != std::string::npos )
        {
            result.erase( pos );
        }

        return result;
    }

    class GodotFileAssetRequestResponse : public CesiumAsync::IAssetRequest,
                                          public ::CesiumAsync::IAssetResponse
    {
    public:
        GodotFileAssetRequestResponse( std::string &&url, uint16_t statusCode,
                                       std::vector<std::byte> &&data ) :
            _url( std::move( url ) ), _statusCode( statusCode ), _data( data )
        {
        }

        virtual const std::string &method() const
        {
            return getMethod;
        }

        virtual const std::string &url() const
        {
            return this->_url;
        }

        virtual const CesiumAsync::HttpHeaders &headers() const override
        {
            return emptyHeaders;
        }

        virtual const CesiumAsync::IAssetResponse *response() const override
        {
            return this;
        }

        virtual uint16_t statusCode() const override
        {
            return this->_statusCode;
        }

        virtual std::string contentType() const override
        {
            return std::string();
        }

        virtual std::span<const std::byte> data() const override
        {
            return this->_data;
        }

    private:
        static const std::string getMethod;
        static const CesiumAsync::HttpHeaders emptyHeaders;

        std::string _url;
        uint16_t _statusCode;
        std::vector<std::byte> _data;
    };

    const std::string GodotFileAssetRequestResponse::getMethod = "GET";
    const CesiumAsync::HttpHeaders GodotFileAssetRequestResponse::emptyHeaders{};

    class GodotReadFileTask
    {
    public:
        static thread_pool pool;

    public:
        GodotReadFileTask( const std::string &url, const CesiumAsync::AsyncSystem &asynSystem ) :
            _url( url ),
            _promise( asynSystem.createPromise<std::shared_ptr<CesiumAsync::IAssetRequest>>() )
        {
        }

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> getFuture()
        {
            return this->_promise.getFuture();
        }

        void startBackgroundTask()
        {
            pool.enqueueWork( [this] { this->doTask(); } );
        }

        void doTask()
        {
            std::string fileName = convertFileUriToFilename( this->_url );
            std::vector<std::byte> data;
            if ( FileHelper::loadFile( data, fileName ) )
            {
                _promise.resolve( std::make_shared<GodotFileAssetRequestResponse>(
                    std::move( this->_url ), 200, std::move( data ) ) );
            }
            else
            {
                _promise.resolve( std::make_shared<GodotFileAssetRequestResponse>(
                    std::move( this->_url ), 404, std::vector<std::byte>() ) );
            }
        }

    private:
        std::string _url;
        CesiumAsync::Promise<std::shared_ptr<CesiumAsync::IAssetRequest>> _promise;
    };

    thread_pool GodotReadFileTask::pool;

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> getFromFile(
        const CesiumAsync::AsyncSystem &asyncSystem, const std::string &url,
        const std::vector<std::pair<std::string, std::string>> &headers )
    {
        if ( url.empty() )
        {
            throw std::invalid_argument( "URL cannot be empty" );
        }
        auto pTaskOwner = std::make_unique<GodotReadFileTask>( url, asyncSystem );
        GodotReadFileTask *pTask = pTaskOwner.get();
        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> future =
            pTask->getFuture().thenInWorkerThread(
                [pTaskOwner = std::move( pTaskOwner )](
                    std::shared_ptr<CesiumAsync::IAssetRequest> &&pRequest ) {
                    // This lambda, via its capture, keeps the task instance alive until
                    // it is complete.
                    return pRequest;
                } );
        pTask->startBackgroundTask();
        return future;
    }

}

namespace CesiumForGodot
{

    GodotAssetAccessor::GodotAssetAccessor() :
        _cesiumRequestHeaders(), _userAgent( "Mozilla 5.0/ Cesium Godot Plugin" )
    {
        std::string project_name = "CesiumForGodotProject";
        std::string engine = ENGINE_VERSION;
        std::string os_version = "Windows";

        this->_cesiumRequestHeaders.insert( { "X-Cesium-Client", "Cesium For Godot" } );
        this->_cesiumRequestHeaders.insert(
            { "X-Cesium-Client-Version", Cesium::version().utf8().get_data() } );
        this->_cesiumRequestHeaders.insert( { "X-Cesium-Client-Project", project_name } );
        this->_cesiumRequestHeaders.insert( { "X-Cesium-Client-Engine", engine } );
        this->_cesiumRequestHeaders.insert( { "X-Cesium-Client-OS", os_version } );

        this->_http_client.instantiate();
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> GodotAssetAccessor::get(
        const CesiumAsync::AsyncSystem &asyncSystem, const std::string &url,
        const std::vector<CesiumAsync::IAssetAccessor::THeader> &headers )
    {
        if ( FileHelper::isWindowsFilePath( url ) || FileHelper::isUnixFilePath( url ) ||
             FileHelper::isFile( url ) )
        {
            UtilityFunctions::print( "file url->", String( url.c_str() ) );
            auto result = getFromFile( asyncSystem, url, headers );
            return result;
        }

        CesiumAsync::HttpHeaders cesiumRequestHeaders = this->_cesiumRequestHeaders;
        godot::String userAgent = this->_userAgent;
        Ref<HTTPClient> httpClient = this->_http_client;

        return asyncSystem.createFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>(
            [&url, &headers, &userAgent, &cesiumRequestHeaders,
             &httpClient]( const auto &promise ) {
                auto [host, port, path] = extract_url_parts( url );
                String _host( host.c_str() );
                int32_t _port( static_cast<int32_t>( std::stoi( port ) ) );
                String _path( path.c_str() );
                HTTPClient::Status status = httpClient->get_status();

                if ( status == HTTPClient::STATUS_DISCONNECTED ||
                     status == HTTPClient::STATUS_CONNECTION_ERROR )
                {
                    godot::Error err = httpClient->connect_to_host( _host, _port );
                    if ( err != Error::OK )
                    {
                        promise.reject( std::runtime_error( "Connect to host failed." ) );
                    }
                    status = httpClient->get_status();
                    while ( status == HTTPClient::STATUS_CONNECTING ||
                            status == HTTPClient::STATUS_RESOLVING )
                    {
                        httpClient->poll();
                        // SPDLOG_INFO( "Connecting..." );
                        status = httpClient->get_status();
                    }
                }

                PackedStringArray packed_headers;
                for ( const auto &header : headers )
                {
                    std::string hs = header.first + ":" + header.second;
                    // if ( header.first == "If-None-Match" )
                    // {
                    //     continue;
                    // }
                    packed_headers.push_back( godot::String( hs.c_str() ) );
                }
                for ( const auto &header : cesiumRequestHeaders )
                {
                    std::string hs = header.first + ":" + header.second;
                    packed_headers.push_back( godot::String( hs.c_str() ) );
                }
                packed_headers.push_back( "User-Agent: " + userAgent );
                packed_headers.push_back( "Accept: */*" );

                UtilityFunctions::print( "Requrest url -> ", _path );

                godot::Error err =
                    httpClient->request( HTTPClient::METHOD_GET, _path, packed_headers );

                if ( err != Error::OK )
                {
                    promise.reject( std::runtime_error( "Request failed." ) );
                }

                while ( httpClient->get_status() == HTTPClient::STATUS_REQUESTING )
                {
                    httpClient->poll();
                    UtilityFunctions::print( "Requesting..." );
                }

                status = httpClient->get_status();
                UtilityFunctions::print( " Request status -> ", status );

                if ( status == HTTPClient::STATUS_CONNECTED || status == HTTPClient::STATUS_BODY )
                {

                    if ( httpClient->has_response() )
                    {
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

                        AHttpResponse pResponse;
                        pResponse.code = httpClient->get_response_code();
                        pResponse.headers = httpClient->get_response_headers_as_dictionary();
                        pResponse.data = rb;

                        UtilityFunctions::print( "Response code: ", pResponse.code );
                        promise.resolve( std::make_unique<GodotAssetRequest>(
                            "GET", url, cesiumRequestHeaders, std::move( pResponse ) ) );
                    }
                }
                promise.reject( std::runtime_error( "Requesting error or no response." ) );
            } );
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> GodotAssetAccessor::request(
        const CesiumAsync::AsyncSystem &asyncSystem, const std::string &verb,
        const std::string &url, const std::vector<THeader> &headers,
        const std::span<const std::byte> &contentPayload )
    {
        AHttpResponse pResponse;
        std::shared_ptr<CesiumAsync::IAssetRequest> value = std::make_shared<GodotAssetRequest>(
            verb, url, CesiumAsync::HttpHeaders{}, std::move( pResponse ) );
        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> result =
            asyncSystem.createResolvedFuture(
                std::shared_ptr<CesiumAsync::IAssetRequest>( value ) );
        return result;
    }

    void GodotAssetAccessor::tick() noexcept
    {
    }

} // namespace CesiumForGodot
