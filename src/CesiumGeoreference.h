#ifndef CESIUM_GEOREFERENCE_H
#define CESIUM_GEOREFERENCE_H

#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>
#include <glm/mat4x4.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <optional>

using namespace godot;

namespace CesiumForGodot
{

    class CesiumGeoreference : public Node3D
    {
        GDCLASS( CesiumGeoreference, Node3D )

    private:
        Ref<Resource> origin_authority;
        String origin_authority_name;
        double scale;
        std::optional<CesiumGeospatial::LocalHorizontalCoordinateSystem> coordinate_system;
        glm::dmat4 localToEcef;
        glm::dmat4 ecefToLocal;

    protected:
        static void _bind_methods();

    public:
        CesiumGeoreference();
        ~CesiumGeoreference();

        void set_originAuthority( const Ref<Resource> p_origin_authority );
        Ref<Resource> get_originAuthority() const;

        void set_scale( const double p_scale );
        double get_scale() const;

        CesiumGeospatial::LocalHorizontalCoordinateSystem createCoordinateSystem();

        const CesiumGeospatial::LocalHorizontalCoordinateSystem &getCoordinateSystem();

        void computeLocalToEarthCenteredEarthFixedTransformation();

        void updateGeoreference();

    };

} // namespace CesiumForGodot

#endif