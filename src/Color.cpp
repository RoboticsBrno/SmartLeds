#include "Color.h"
#include <algorithm>
#include <cmath>

namespace {

// Int -> fixed point
int up( int x ) { return x * 255; }

} // namespace

Rgb::Rgb( Hsv y ) {
    // https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
    // greyscale
    if(y.s == 0) {
        r = g = b = y.v;
        return;
    }

    const int region = y.h / 43;
    const int remainder = (y.h - (region * 43)) * 6;

    const int p = (y.v * (255 - y.s)) >> 8;
    const int q = (y.v * (255 - ((y.s * remainder) >> 8))) >> 8;
    const int t = (y.v * (255 - ((y.s * (255 -remainder)) >> 8))) >> 8;

    switch(region) {
        case 0: r = y.v; g = t; b = p; break;
        case 1: r = q; g = y.v; b = p; break;
        case 2: r = p; g = y.v; b = t; break;
        case 3: r = p; g = q; b = y.v; break;
        case 4: r = t; g = p; b = y.v; break;
        case 5: r = y.v; g = p; b = q; break;
        default: __builtin_trap();
    }
}

Rgb& Rgb::operator=( Hsv hsv ) {
    Rgb r{ hsv };
    swap( r );
    return *this;
}

Rgb Rgb::operator+( Rgb in ) const {
    auto copy = *this;
    copy += in;
    return copy;
}

Rgb& Rgb::operator+=( Rgb in ) {
    unsigned int red = r + in.r;
    r = ( red < 255 ) ? red : 255;
    unsigned int green = g + in.g;
    g = ( green < 255 ) ? green : 255;
    unsigned int blue = b + in.b;
    b = ( blue < 255 ) ? blue : 255;
    return *this;
}

uint8_t IRAM_ATTR Rgb::getGrb( int idx ) {
    switch ( idx ) {
        case 0: return g;
        case 1: return r;
        case 2: return b;
    }
    __builtin_unreachable();
}

Hsv::Hsv( Rgb r ) {
    int min = std::min( r.r, std::min( r.g, r.b ) );
    int max = std::max( r.r, std::max( r.g, r.b ) );
    int chroma = max - min;

    v = max;
    if ( chroma == 0 ) {
        h = s = 0;
        return;
    }

    s = up( chroma ) / max;
    int hh;
    if ( max == r.r )
        hh = ( up( int( r.g ) - int( r.b ) ) ) / chroma / 6;
    else if ( max == r.g )
        hh = 255 / 3 + ( up( int( r.b ) - int( r.r ) ) ) / chroma / 6;
    else
        hh = 2 * 255 / 3 + ( up( int( r.r ) - int( r.g ) ) ) / chroma / 6;

    if ( hh < 0 )
        hh += 255;
    h = hh;
}

Hsv& Hsv::operator=( Rgb rgb ) {
    Hsv h{ rgb };
    swap( h );
    return *this;
}
