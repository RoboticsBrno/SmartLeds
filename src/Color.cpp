#include "Color.h"
#include <algorithm>
#include <cmath>

namespace {

// Int -> fixed point
int up( int x ) { return x * 255; }
int up2( int x ) { return x * 255 * 255; }
int up3( int x ) { return x * 255 * 255 * 255; }
// Fixed point -> int
int down( int x ) { return x / 255; };
int down2( int x ) { return x / 255 / 255; }
int down3( int x ) { return x / 255 / 255 / 255; }

} // namespace

Rgb::Rgb( Hsv y ) {
    // // Formula taken from: https://gist.github.com/yoggy/8999625
    int hi = y.h * 6;
    int f  = y.h * 6 - up( down( hi ) );
    int p  = y.v * ( up2( 1 ) - up( y.s ) );
    int q  = y.v * ( up2( 1 ) - y.s * f );
    int t  = y.v * ( up2( 1 ) - y.s * ( up( 1 ) - f ) );

    int rr, gg, bb;
    int vv = up2( y.v );
    switch ( down( hi ) ) {
        case 0: rr = vv, gg = t,  bb = p; break;
        case 1: rr = q,  gg = vv, bb = p; break;
        case 2: rr = p,  gg = vv, bb = t; break;
        case 3: rr = p,  gg = q,  bb = vv; break;
        case 4: rr = t,  gg = p,  bb = vv; break;
        case 5: rr = vv, gg = p,  bb = q; break;
    }
    r = down2( rr );
    g = down2( gg );
    b = down2( bb );
}

Rgb& Rgb::operator=( Hsv hsv ) {
    Rgb r{ hsv };
    swap( r );
    return *this;
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