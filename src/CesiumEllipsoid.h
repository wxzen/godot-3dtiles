#ifndef CESIUM_ELLIPSOID_H
#define CESIUM_ELLIPSOID_H

#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>
#include <optional>

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace CesiumGeospatial
{
    class Ellipsoid;
};

using namespace godot;

namespace CesiumForGodot
{
    class GCesiumEllipsoid
    {
    public:
        GCesiumEllipsoid( const Vector3 &radii );

        ~GCesiumEllipsoid();

        static GCesiumEllipsoid *create( const Vector3 &radii );
        /**
         * Gets the radii of the ellipsoid in its x-, y-, and z-directions in
         * meters.
         */
        Vector3 get_radii();
        /**
         * Sets the radii of this ellipsoid in its x-, y-, and z-directions in meters.
         *
         * Tilesets using this ellipsoid may have to be refreshed to see the changes
         * applied.
         */
        void set_radii( const Vector3 &radii );

        /**
         * Gets the maximum radius of the ellipsoid in any dimension, in meters.
         */
        double get_maximum_radius();

        /**
         * Gets the minimum radius of the ellipsoid in any dimension, in meters.
         */
        double get_minimum_radius();

        /**
         * Scale the given Ellipsoid-Centered, Ellipsoid-Fixed position along the
         * geodetic surface normal so that it is on the surface of the ellipsoid. If
         * the position is near the center of the ellipsoid, the result will have the
         * value (0,0,0) because the surface position is undefined.
         */
        Vector3 scale_to_geodetic_surface( const Vector3 &earth_centered_earth_fixed_position );

        /**
         * Computes the normal of the plane tangent to the surface of the ellipsoid
         * at the provided Ellipsoid-Centered, Ellipsoid-Fixed position.
         */
        Vector3 geodetic_surface_normal( const Vector3 &earth_centered_earth_fixed_position );

        /**
         * Convert longitude in degrees (X), latitude in degrees (Y), and height above
         * the ellipsoid in meters (Z) to Ellipsoid-Centered, Ellipsoid-Fixed (ECEF)
         * coordinates.
         */
        Vector3 longitude_latitude_height_to_ellipsoid_centered_ellipsoid_fixed(
            const Vector3 &longitude_latitude_height );

        /**
         * Convert Ellipsoid-Centered, Ellipsoid-Fixed (ECEF) coordinates to longitude
         * in degrees (X), latitude in degrees (Y), and height above the ellipsoid in
         * meters (Z). If the position is near the center of the Ellipsoid, the result
         * will have the value (0,0,0) because the longitude, latitude, and height are
         * undefined.
         */
        Vector3 ellipsoid_centered_ellipsoid_fixed_to_longitude_latitude_height(
            const Vector3 &earth_centered_earth_fixed_position );

        /**
         * Computes the transformation matrix from the local East-North-Up (ENU) frame
         * to Ellipsoid-Centered, Ellipsoid-Fixed (ECEF) at the specified ECEF
         * location.
         */
        Transform3D east_north_up_to_ellipsoid_centered_ellipsoid_fixed(
            const Vector3 &earth_centered_earth_fixed_position );

        /**
         * Returns a new {@link CesiumGeospatial::LocalHorizontalCoordinateSystem}
         * with the given scale, center, and ellipsoid.
         */
        CesiumGeospatial::LocalHorizontalCoordinateSystem create_coordinate_system(
            const Vector3 &Center, double Scale );

        /**
         * Returns the underlying {@link CesiumGeospatial::Ellipsoid}
         */
        const CesiumGeospatial::Ellipsoid &get_native_ellipsoid();

    protected:
        /**
         * The radii of this ellipsoid.
         *
         * The X coordinate of the vector should be the radius of the largest axis and
         * the Z coordinate should be the radius of the smallest axis.
         */
        Vector3 _radii;

    private:
        std::optional<CesiumGeospatial::Ellipsoid> _native_ellipsoid;
    };

}
#endif // CESIUM_ELLIPSOID_H