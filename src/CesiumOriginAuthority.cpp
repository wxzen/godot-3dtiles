#include "CesiumOriginAuthority.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;
using namespace CesiumForGodot;

void LongitudeLatitudeHeight::_bind_methods()
{
    ClassDB::bind_method( D_METHOD( "get_longitude" ), &LongitudeLatitudeHeight::get_longitude );
    ClassDB::bind_method( D_METHOD( "set_longitude", "p_longitude" ),
                          &LongitudeLatitudeHeight::set_longitude );
    ClassDB::add_property( "LongitudeLatitudeHeight", PropertyInfo( Variant::FLOAT, "longitude" ),
                           "set_longitude", "get_longitude" );

    ClassDB::bind_method( D_METHOD( "get_latitude" ), &LongitudeLatitudeHeight::get_latitude );
    ClassDB::bind_method( D_METHOD( "set_latitude", "p_latitude" ),
                          &LongitudeLatitudeHeight::set_latitude );
    ClassDB::add_property( "LongitudeLatitudeHeight", PropertyInfo( Variant::FLOAT, "latitude" ),
                           "set_latitude", "get_latitude" );

    ClassDB::bind_method( D_METHOD( "get_height" ), &LongitudeLatitudeHeight::get_height );
    ClassDB::bind_method( D_METHOD( "set_height", "p_height" ),
                          &LongitudeLatitudeHeight::set_height );
    ClassDB::add_property( "LongitudeLatitudeHeight", PropertyInfo( Variant::FLOAT, "height" ),
                           "set_height", "get_height" );

    ADD_SIGNAL( MethodInfo( "lngLatH_changed" ) );
}

LongitudeLatitudeHeight::LongitudeLatitudeHeight()
{
}

LongitudeLatitudeHeight::~LongitudeLatitudeHeight()
{
}

void LongitudeLatitudeHeight::set_longitude( const double p_longitude )
{
    if ( longitude != p_longitude )
    {
        longitude = p_longitude;
        emit_signal( "lngLatH_changed" );
    }
}

double LongitudeLatitudeHeight::get_longitude() const
{
    return longitude;
}

void LongitudeLatitudeHeight::set_latitude( const double p_latitude )
{
    if ( latitude != p_latitude )
    {
        latitude = p_latitude;
        emit_signal( "lngLatH_changed" );
    }
}

double LongitudeLatitudeHeight::get_latitude() const
{
    return latitude;
}

void LongitudeLatitudeHeight::set_height( const double p_height )
{
    if ( height != p_height )
    {
        height = p_height;
        emit_signal( "lngLatH_changed" );
    }
}

double LongitudeLatitudeHeight::get_height() const
{
    return height;
}

String LongitudeLatitudeHeight::get_name() const
{
    return name;
}

void EarthCenteredEarthFixed::_bind_methods()
{
    ClassDB::bind_method( D_METHOD( "get_ecefX" ), &EarthCenteredEarthFixed::get_ecefX );
    ClassDB::bind_method( D_METHOD( "set_ecefX", "p_ecefX" ), &EarthCenteredEarthFixed::set_ecefX );
    ClassDB::add_property( "EarthCenteredEarthFixed", PropertyInfo( Variant::FLOAT, "ecefX" ),
                           "set_ecefX", "get_ecefX" );

    ClassDB::bind_method( D_METHOD( "get_ecefY" ), &EarthCenteredEarthFixed::get_ecefY );
    ClassDB::bind_method( D_METHOD( "set_ecefY", "p_ecefY" ), &EarthCenteredEarthFixed::set_ecefY );
    ClassDB::add_property( "EarthCenteredEarthFixed", PropertyInfo( Variant::FLOAT, "ecefY" ),
                           "set_ecefY", "get_ecefY" );

    ClassDB::bind_method( D_METHOD( "get_ecefZ" ), &EarthCenteredEarthFixed::get_ecefZ );
    ClassDB::bind_method( D_METHOD( "set_ecefZ", "p_ecefZ" ), &EarthCenteredEarthFixed::set_ecefZ );
    ClassDB::add_property( "EarthCenteredEarthFixed", PropertyInfo( Variant::FLOAT, "ecefZ" ),
                           "set_ecefZ", "get_ecefZ" );
    ADD_SIGNAL( MethodInfo( "ecef_changed" ) );
}

EarthCenteredEarthFixed::EarthCenteredEarthFixed()
{
}

EarthCenteredEarthFixed::~EarthCenteredEarthFixed()
{
}

void EarthCenteredEarthFixed::set_ecefX( const double p_ecefX )
{
    if ( ecefX != p_ecefX )
    {
        ecefX = p_ecefX;
        emit_signal( "ecef_changed" );
    }
}

double EarthCenteredEarthFixed::get_ecefX() const
{
    return ecefX;
}

void EarthCenteredEarthFixed::set_ecefY( const double p_ecefY )
{
    if ( ecefY != p_ecefY )
    {
        ecefY = p_ecefY;
        emit_signal( "ecef_changed" );
    }
}

double EarthCenteredEarthFixed::get_ecefY() const
{
    return ecefY;
}

void EarthCenteredEarthFixed::set_ecefZ( const double p_ecefZ )
{
    if ( ecefZ != p_ecefZ )
    {
        ecefZ = p_ecefZ;
        emit_signal( "ecef_changed" );
    }
}

double EarthCenteredEarthFixed::get_ecefZ() const
{
    return ecefZ;
}

String EarthCenteredEarthFixed::get_name() const
{
    return name;
}