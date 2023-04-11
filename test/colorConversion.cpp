#include <Color.h>
#include <catch.hpp>
#include <cmath>
#include <vector>
#include <iostream>

// Oracle functions, source: https://gist.github.com/yoggy/8999625
//

#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c)  (fmaxf(a, fmaxf(b, c)))

void rgb2hsv(const unsigned char &src_r, const unsigned char &src_g, const unsigned char &src_b, unsigned char &dst_h, unsigned char &dst_s, unsigned char &dst_v)
{
    float r = src_r / 255.0f;
    float g = src_g / 255.0f;
    float b = src_b / 255.0f;

    float h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    float max = max_f(r, g, b);
    float min = min_f(r, g, b);

    v = max;

    if (max == 0.0f) {
        s = 0;
        h = 0;
    }
    else if (max - min == 0.0f) {
        s = 0;
        h = 0;
    }
    else {
        s = (max - min) / max;

        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        }
        else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        }
        else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }

    if (h < 0) h += 360.0f;

    dst_h = (unsigned char)(h / 360.0 * 255);   // dst_h : 0-180
    dst_s = (unsigned char)(s * 255); // dst_s : 0-255
    dst_v = (unsigned char)(v * 255); // dst_v : 0-255
}

void hsv2rgb(const unsigned char &src_h, const unsigned char &src_s, const unsigned char &src_v, unsigned char &dst_r, unsigned char &dst_g, unsigned char &dst_b)
{
    float h = src_h / 255.0 * 360; // 0-360
    float s = src_s / 255.0f; // 0.0-1.0
    float v = src_v / 255.0f; // 0.0-1.0

    float r, g, b; // 0.0-1.0

    int   hi = (int)(h / 60.0f) % 6;
    float f  = (h / 60.0f) - hi;
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - s * f);
    float t  = v * (1.0f - s * (1.0f - f));

    switch(hi) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    dst_r = (unsigned char)(r * 255); // dst_r : 0-255
    dst_g = (unsigned char)(g * 255); // dst_r : 0-255
    dst_b = (unsigned char)(b * 255); // dst_r : 0-255
}

// End of oracle

std::vector< Rgb > rgbSamples() {
    std::vector< Rgb > ret;
    static const int step = 10;
    for ( int r = 0; r <= 255; r += step )
        for ( int g = 0; g <= 255; g += step )
            for ( int b = 0; b <= 255; b += step )
                ret.emplace_back( r, g, b );
    return ret;
}

std::vector< Hsv > hsvSamples() {
    std::vector< Hsv > ret;
    static const int step = 10;
    for ( int h = 0; h <= 255; h += step )
        for ( int s = 0; s <= 255; s += step )
            for ( int v = 0; v <= 255; v += 255 )
                ret.emplace_back( h, s, v );
    return ret;
}

int dist( int x, int y ) {
    return std::abs( x - y );
}


TEST_CASE("RGB <-> HSV", "[rgb<->hsv]") {
    for ( Rgb color : rgbSamples() ) {
        Hsv hsv{ color };
        Rgb rgb{ hsv };
        uint8_t h, s, v;
        rgb2hsv( color.r, color.g, color.b, h, s, v );
        uint8_t r, g, b;
        hsv2rgb( hsv.h, hsv.s, hsv.v, r, g, b );

        CAPTURE( int( color.r ), int( color.g ), int( color.b ) );
        CAPTURE( int( rgb.r ), int( rgb.g ), int( rgb.b ) );
        CAPTURE( int( hsv.h ), int( hsv.s ), int( hsv.v ) );
        CAPTURE( int( h ), int( s ), int( v ) );
        CAPTURE( int( r ), int( g ), int( b ) );

        REQUIRE( dist( rgb.r, r ) <= 1 );
        REQUIRE( dist( rgb.g, g ) <= 1 );
        REQUIRE( dist( rgb.b, b ) <= 1 );

        REQUIRE( dist(hsv.h, h ) <= 1 );
        REQUIRE( dist(hsv.s, s ) <= 1 );
        REQUIRE( dist(hsv.v, v ) <= 1 );
    }
}