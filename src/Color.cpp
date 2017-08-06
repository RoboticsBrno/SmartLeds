#include "Color.h"
#include <algorithm>
#include <cmath>

Rgb::Rgb( Hsv y ) {
    // Formula taken from: https://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
    int h = 6 * y.h;
    int c = y.v * y.s;
    int x = c * ( 255 - std::abs( h % ( 2 << 8 ) - 255 ) );
    c >>= 8;
    h >>= 8;
    x >>= 16;
    int m = y.v - c;
    switch ( h ) {
        case 6:
        case 0: r = c + m; g = x + m; b = m;     break;
        case 1: r = x + m; g = c + m; b = m;     break;
        case 2: r = m;     g = c + m; b = x + m; break;
        case 3: r = m;     g = x + m; b = c + m; break;
        case 4: r = x + m; g = m;     b = c + m; break;
        case 5: r = c + m; g = m;     b = x + m; break;
    }
}

Rgb& Rgb::operator=( Hsv hsv ) {
    Rgb r{ hsv };
    swap( r );
    return *this;
}

Hsv::Hsv( Rgb r ) {
    // Formula taken from http://www.easyrgb.com/en/math.php
    int min = std::min( r.r, std::min( r.g, r.b ) );
    int max = std::max( r.r, std::max( r.g, r.b ) );
    int delta = max - min;

    v = max;
    if ( delta == 0 ) {
        h = s = 0;
        return;
    }

    s = ( delta << 8 ) / max;
    int deltaR = ( ( ( ( max - r.r ) / 6 ) + ( max / 2 ) ) << 8 ) / max;
    int deltaG = ( ( ( ( max - r.g ) / 6 ) + ( max / 2 ) ) << 8 ) / max;
    int deltaB = ( ( ( ( max - r.b ) / 6 ) + ( max / 2 ) ) << 8 ) / max;

    int hh;
    if ( max == r.r )
        hh = deltaB - deltaG;
    else if ( max == r.g )
        hh = 255 / 3 + deltaR - deltaB;
    else
        hh = 2 * 255 / 3 + deltaG - deltaR;

    if ( hh < 0 )
        hh += 255;
    if ( h > 255 )
        hh -= 255;
    h = hh;
}

Hsv& Hsv::operator=( Rgb rgb ) {
    Hsv h{ rgb };
    swap( h );
    return *this;
}