#include "CesiumEllipsoid.h"
#include "CesiumGeoreference.h"
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/GlobeTransforms.h>
#include <CesiumUtility/Math.h>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace CesiumForGodot;
using namespace CesiumGeospatial;
using namespace CesiumUtility;

GCesiumEllipsoid *GCesiumEllipsoid::create( const Vector3 &radii )
{
    GCesiumEllipsoid *pEllipsoid = new GCesiumEllipsoid( radii );
    return pEllipsoid;
}

GCesiumEllipsoid::GCesiumEllipsoid( const Vector3 &radii ) : _radii( radii )
{
}

GCesiumEllipsoid::~GCesiumEllipsoid()
{
}

Vector3 GCesiumEllipsoid::get_radii()
{
    return this->_radii;
}

void GCesiumEllipsoid::set_radii( const Vector3 &p_radii )
{
    if ( this->_radii != p_radii )
    {

        this->_radii = p_radii;
    }
}

double GCesiumEllipsoid::get_maximum_radius()
{
    return this->_radii.x;
}

double GCesiumEllipsoid::get_minimum_radius()
{
    return this->_radii.z;
}

Vector3 GCesiumEllipsoid::scale_to_geodetic_surface(
    const Vector3 &earth_centered_earth_fixed_position )
{
    std::optional<glm::dvec3> result = this->get_native_ellipsoid().scaleToGeodeticSurface(
        glm::dvec3( earth_centered_earth_fixed_position.x, earth_centered_earth_fixed_position.y,
                    earth_centered_earth_fixed_position.z ) );
    if ( result )
    {
        return Vector3( result->x, result->y, result->z );
    }
    else
    {
        return Vector3( 0.0f, 0.0f, 0.0f );
    }
}

Vector3 GCesiumEllipsoid::geodetic_surface_normal(
    const Vector3 &earth_centered_earth_fixed_position )
{
    const CesiumGeospatial::Ellipsoid &ellipsoid = this->get_native_ellipsoid();
    glm::dvec3 vec3 = ellipsoid.geodeticSurfaceNormal(
        glm::dvec3( earth_centered_earth_fixed_position.x, earth_centered_earth_fixed_position.y,
                    earth_centered_earth_fixed_position.z ) );
    return Vector3( vec3.x, vec3.y, vec3.z );
}

Vector3 GCesiumEllipsoid::longitude_latitude_height_to_ellipsoid_centered_ellipsoid_fixed(
    const Vector3 &longitude_latitude_height )
{
    const CesiumGeospatial::Ellipsoid &ellipsoid = this->get_native_ellipsoid();
    glm::dvec3 vec3 = ellipsoid.cartographicToCartesian( Cartographic::fromDegrees(
        longitude_latitude_height.x, longitude_latitude_height.y, longitude_latitude_height.z ) );
    return Vector3( vec3.x, vec3.y, vec3.z );
}

Vector3 GCesiumEllipsoid::ellipsoid_centered_ellipsoid_fixed_to_longitude_latitude_height(
    const Vector3 &earth_centered_earth_fixed_position )
{
    const CesiumGeospatial::Ellipsoid &ellipsoid = this->get_native_ellipsoid();
    std::optional<Cartographic> result = ellipsoid.cartesianToCartographic(
        glm::dvec3( earth_centered_earth_fixed_position.x, earth_centered_earth_fixed_position.y,
                    earth_centered_earth_fixed_position.z ) );
    if ( result )
    {
        return Vector3( CesiumUtility::Math::radiansToDegrees( result->longitude ),
                        CesiumUtility::Math::radiansToDegrees( result->latitude ), result->height );
    }
    else
    {
        return Vector3( 0.0f, 0.0f, 0.0f );
    }
}

Transform3D GCesiumEllipsoid::east_north_up_to_ellipsoid_centered_ellipsoid_fixed(
    const Vector3 &earth_centered_earth_fixed_position )
{
    const CesiumGeospatial::Ellipsoid &ellipsoid = this->get_native_ellipsoid();
    glm::dmat4 mat4 = CesiumGeospatial::GlobeTransforms::eastNorthUpToFixedFrame(
        glm::dvec3( earth_centered_earth_fixed_position.x, earth_centered_earth_fixed_position.y,
                    earth_centered_earth_fixed_position.z ),
        ellipsoid );
    Transform3D transform3D;
    transform3D.basis = Basis( Vector3( mat4[0][0], mat4[0][1], mat4[0][2] ),
                               Vector3( mat4[1][0], mat4[1][1], mat4[1][2] ),
                               Vector3( mat4[2][0], mat4[2][1], mat4[2][2] ) );
    transform3D.origin = Vector3( mat4[3][0], mat4[3][1], mat4[3][2] );
    return transform3D;
}

CesiumGeospatial::LocalHorizontalCoordinateSystem GCesiumEllipsoid::create_coordinate_system(
    const Vector3 &center, double scale )
{
    return LocalHorizontalCoordinateSystem(
        glm::dvec3( center.x, center.y, center.z ), LocalDirection::East, LocalDirection::South,
        LocalDirection::Up, 1.0 / scale, this->get_native_ellipsoid() );
}

const CesiumGeospatial::Ellipsoid &GCesiumEllipsoid::get_native_ellipsoid()
{
    const double min_radii_value = 1e-10;
    if ( !_native_ellipsoid )
    {
        if ( _radii.x < min_radii_value || _radii.y < min_radii_value ||
             _radii.z < min_radii_value )
        {
            UtilityFunctions::printerr(
                "Radii must be greater than 0 - clamping to minimum value to avoid crashes." );
        }
        _native_ellipsoid.emplace( MAX( _radii.x, min_radii_value ),
                                   MAX( _radii.y, min_radii_value ),
                                   MAX( _radii.z, min_radii_value ) );
    }
    return *_native_ellipsoid;
}