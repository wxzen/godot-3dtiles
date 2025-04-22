#include "CesiumGeoreference.h"
#include "CesiumOriginAuthority.h"
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>
#include <CesiumUtility/Math.h>
#include <godot_cpp/core/class_db.hpp>

using namespace CesiumGeospatial;
using namespace CesiumUtility;
using namespace CesiumForGodot;

void CesiumGeoreference::_bind_methods()
{
    ClassDB::bind_method( D_METHOD( "get_scale" ), &CesiumGeoreference::get_scale );
    ClassDB::bind_method( D_METHOD( "set_scale", "p_scale" ), &CesiumGeoreference::set_scale );
    ClassDB::add_property( "CesiumGeoreference", PropertyInfo( Variant::FLOAT, "scale" ),
                           "set_scale", "get_scale" );

    ClassDB::bind_method( D_METHOD( "get_originAuthority" ),
                          &CesiumGeoreference::get_originAuthority );
    ClassDB::bind_method( D_METHOD( "set_originAuthority", "p_originAuthority" ),
                          &CesiumGeoreference::set_originAuthority );
    ClassDB::add_property( "CesiumGeoreference",
                           PropertyInfo( Variant::OBJECT, "originAuthority",
                                         PROPERTY_HINT_RESOURCE_TYPE,
                                         "LongitudeLatitudeHeight, EarthCenteredEarthFixed" ),
                           "set_originAuthority", "get_originAuthority" );
    ClassDB::bind_method( D_METHOD( "updateGeoreference" ),
                          &CesiumGeoreference::updateGeoreference );

    ADD_SIGNAL( MethodInfo( "scale_changed", PropertyInfo( Variant::OBJECT, "node" ),
                            PropertyInfo( Variant::FLOAT, "scale" ) ) );
}

CesiumGeoreference::CesiumGeoreference() :
    origin_authority_name( "" ), scale( 1.0f ), localToEcef( glm::dmat4( 1.0f ) ),
    ecefToLocal( glm::dmat4( 1.0f ) ), coordinate_system()
{
    ellipsoid =
        GCesiumEllipsoid::create( Vector3( 6378137.0f, 6378137.0f, 6356752.3142451793f ) ); // WGS84
}

CesiumGeoreference::~CesiumGeoreference()
{
}

void CesiumGeoreference::set_originAuthority( const Ref<Resource> p_origin_authority )
{
    if ( origin_authority != p_origin_authority )
    {
        if ( origin_authority.is_valid() )
        {
            if ( origin_authority_name == "LongitudeLatitudeHeight" )
            {
                origin_authority->disconnect( "lngLatH_changed",
                                              Callable( this, "updateGeoreference" ) );
            }
            else if ( origin_authority_name == "EarthCenteredEarthFixed" )
            {
                origin_authority->disconnect( "ecef_changed",
                                              Callable( this, "updateGeoreference" ) );
            }
        }
        origin_authority = p_origin_authority;
        Ref<LongitudeLatitudeHeight> lngLatH =
            Ref<LongitudeLatitudeHeight>( p_origin_authority.ptr() );
        if ( !p_origin_authority.is_valid() )
        {
            this->origin_authority_name = "";
            return;
        }
        if ( lngLatH.is_valid() )
        {
            this->origin_authority_name = "LongitudeLatitudeHeight";
            p_origin_authority->connect( "lngLatH_changed",
                                         Callable( this, "updateGeoreference" ) );
        }
        else
        {
            this->origin_authority_name = "EarthCenteredEarthFixed";
            p_origin_authority->connect( "ecef_changed", Callable( this, "updateGeoreference" ) );
        }
    }
}

Ref<Resource> CesiumGeoreference::get_originAuthority() const
{
    return origin_authority;
}

void CesiumGeoreference::set_scale( const double p_scale )
{
    if ( scale != p_scale )
    {
        scale = p_scale;
         if ( scale < 1e-8 )
         {
             scale = 1e-8;
         }
        emit_signal( "scale_changed" );
        if ( origin_authority.is_valid() )
        {
            this->updateGeoreference();
        }
    }
}

double CesiumGeoreference::get_scale() const
{
    return scale;
}

LocalHorizontalCoordinateSystem CesiumGeoreference::createCoordinateSystem()
{
    double scaleToMeters = 1 / scale;
    if ( origin_authority.is_valid() )
    {
        Ref<LongitudeLatitudeHeight> lngLatH =
            Ref<LongitudeLatitudeHeight>( origin_authority.ptr() );
        if ( lngLatH.is_valid() )
        {
            this->origin_authority_name = "LongitudeLatitudeHeight";
            return LocalHorizontalCoordinateSystem(
                Cartographic::fromDegrees( lngLatH->get_longitude(), lngLatH->get_latitude(),
                                           lngLatH->get_height() ),
                LocalDirection::East, LocalDirection::Up, LocalDirection::South, scaleToMeters,
                ellipsoid->get_native_ellipsoid() );
        }
        else
        {
            this->origin_authority_name = "EarthCenteredEarthFixed";
            Ref<EarthCenteredEarthFixed> p_ecef =
                Ref<EarthCenteredEarthFixed>( origin_authority.ptr() );
            return LocalHorizontalCoordinateSystem(
                glm::dvec3( p_ecef->get_ecefX(), p_ecef->get_ecefY(), p_ecef->get_ecefZ() ),
                LocalDirection::East, LocalDirection::Up, LocalDirection::South, scaleToMeters,
                ellipsoid->get_native_ellipsoid() );
        }
    }
}

const CesiumGeospatial::LocalHorizontalCoordinateSystem &CesiumGeoreference::getCoordinateSystem()
{
    if ( !coordinate_system )
    {
        this->computeLocalToEarthCenteredEarthFixedTransformation();
    }
    return *coordinate_system;
}

void CesiumGeoreference::computeLocalToEarthCenteredEarthFixedTransformation()
{
    this->coordinate_system = this->createCoordinateSystem();
    this->localToEcef = this->coordinate_system->getLocalToEcefTransformation();
    this->ecefToLocal = this->coordinate_system->getEcefToLocalTransformation();
}

void CesiumGeoreference::updateGeoreference()
{
    this->computeLocalToEarthCenteredEarthFixedTransformation();
}

GCesiumEllipsoid *CesiumGeoreference::get_ellipsoid() const
{
    return ellipsoid;
}

void CesiumGeoreference::set_ellipsoid( GCesiumEllipsoid *p_ellipsoid )
{
    if ( ellipsoid != p_ellipsoid )
    {
        ellipsoid = p_ellipsoid;
        if ( origin_authority.is_valid() )
        {
            this->updateGeoreference();
        }
    }
}