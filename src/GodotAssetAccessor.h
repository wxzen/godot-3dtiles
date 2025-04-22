#ifndef GODOT_ASSET_ACCESSOR_H
#define GODOT_ASSET_ACCESSOR_H

#include "ThreadUtils.hpp"
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <godot_cpp/classes/http_client.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace CesiumForGodot
{
    struct AHttpResponse
    {
        godot::Dictionary headers;
        int32_t code;
        godot::PackedByteArray data;
    };

    class GodotAssetAccessor : public CesiumAsync::IAssetAccessor
    {
    public:
        GodotAssetAccessor();

        virtual CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> get(
            const CesiumAsync::AsyncSystem &asyncSystem, const std::string &url,
            const std::vector<THeader> &headers = {} ) override;

        virtual CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> request(
            const CesiumAsync::AsyncSystem &asyncSystem, const std::string &verb,
            const std::string &url, const std::vector<THeader> &headers = std::vector<THeader>(),
            const std::span<const std::byte> &contentPayload = {} ) override;

        virtual void tick() noexcept override;

    private:
        CesiumAsync::HttpHeaders _cesiumRequestHeaders;
        godot::String _userAgent;
        godot::Ref<godot::HTTPClient> _http_client;
    };

} // namespace CesiumForGodot

#endif