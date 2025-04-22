#ifndef CESIUM_ORIGIN_AUTHORITY_H
#define CESIUM_ORIGIN_AUTHORITY_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace CesiumForGodot
{
    /*
    *<summary>
        Sets the origin of the coordinate system to a particular <see cref="longitude"/>,
        <see cref="latitude"/>, and <see cref="height"/>.
        </summary>
        <remarks>
        Calling this method is more efficient than setting the properties individually.
        </remarks>
        <param name="longitude">The longitude in degrees, in the range -180 to 180.</param>
        <param name="latitude">The latitude in degrees, in the range -90 to 90.</param>
        <param name="height">
        The height in meters above the ellipsoid. Do not confuse this with a geoid height
        or height above mean sea level, which can be tens of meters higher or lower
        depending on where in the world the object is located.
        </param>
    */
    class LongitudeLatitudeHeight : public Resource
    {
        GDCLASS( LongitudeLatitudeHeight, Resource )

    private:
        double longitude = 0.0;
        double latitude = 0.0;
        double height = 0.0;
        String name = "LongitudeLatitudeHeight";

    protected:
        static void _bind_methods();

    public:
        LongitudeLatitudeHeight();
        ~LongitudeLatitudeHeight();

        void set_longitude( const double p_longitude );
        double get_longitude() const;

        void set_latitude( const double p_latitude );
        double get_latitude() const;

        void set_height( const double p_height );
        double get_height() const;

        String get_name() const;
    };

    /*
     * The Earth-Centered, Earth-Fixed coordinate of the origin of the coordinate system
     */
    class EarthCenteredEarthFixed : public Resource
    {
        GDCLASS( EarthCenteredEarthFixed, Resource )

    private:
        double ecefX = 6378137.0;
        double ecefY = 0.0;
        double ecefZ = 0.0;
        String name = "EarthCenteredEarthFixed";

    protected:
        static void _bind_methods();

    public:
        EarthCenteredEarthFixed();
        ~EarthCenteredEarthFixed();

        void set_ecefX( const double p_ecefX );
        double get_ecefX() const;

        void set_ecefY( const double p_ecefY );
        double get_ecefY() const;

        void set_ecefZ( const double p_ecefZ );
        double get_ecefZ() const;

        String get_name() const;
    };

} // namespace CesiumForGodot

#endif