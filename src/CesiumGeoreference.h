#ifndef CESIUM_GEOREFERENCE_H
#define CESIUM_GEOREFERENCE_H

#include "CesiumEllipsoid.h"
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>
#include <glm/mat4x4.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <optional>

using namespace godot;

namespace CesiumForGodot
{
    /**
     * Controls how global geospatial coordinates are mapped to coordinates in the
     * Unreal Engine level. Internally, Cesium uses a global Earth-centered,
     * Earth-fixed (ECEF) ellipsoid-centered coordinate system, where the ellipsoid
     * is usually the World Geodetic System 1984 (WGS84) ellipsoid. This is a
     * right-handed system centered at the Earth's center of mass, where +X is in
     * the direction of the intersection of the Equator and the Prime Meridian (zero
     * degrees longitude), +Y is in the direction of the intersection of the Equator
     * and +90 degrees longitude, and +Z is through the North Pole. This Actor is
     * used by other Cesium Actors and components to control how this coordinate
     * system is mapped into an Unreal Engine world and level.
     */
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
        GCesiumEllipsoid *ellipsoid;

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

        GCesiumEllipsoid *get_ellipsoid() const;

        void set_ellipsoid( GCesiumEllipsoid *p_ellipsoid );
    };

} // namespace CesiumForGodot

#endif