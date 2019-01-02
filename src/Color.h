#pragma once

#include <cstdint>

union Hsv;

union Rgb {
    struct __attribute__ ((packed)) {
        uint8_t r, g, b, _extra;
    };
    uint32_t value;

    Rgb( uint8_t r = 0, uint8_t g = 0, uint8_t b = 0 ) : r( r ), g( g ), b( b ), _extra( 0 ) {}
    Rgb( Hsv c );
    Rgb& operator=( Rgb r ) { swap( r ); return *this; }
    Rgb& operator=( Hsv hsv );
    void swap( Rgb& o ) {  value = o.value; }
    void linearize() {
        r = ( static_cast< int >( r ) * static_cast< int >( r ) ) >> 8;
        g = ( static_cast< int >( g ) * static_cast< int >( g ) ) >> 8;
        g = ( static_cast< int >( b ) * static_cast< int >( b ) ) >> 8;
    }
    uint8_t getGrb( int idx ) {
        switch ( idx ) {
            case 0: return g;
            case 1: return r;
            case 2: return b;
        }
        __builtin_unreachable();
    }

    void stretchChannels( uint8_t maxR, uint8_t maxG, uint8_t maxB ) {
        r = stretch( r, maxR );
        g = stretch( g, maxG );
        b = stretch( b, maxB );
    }

private:
    uint8_t stretch( int value, uint8_t max ) {
        return ( value * max ) >> 8;
    }
};

union Hsv {
    struct __attribute__ ((packed)) {
        uint8_t h, s, v, _extra;
    };
    uint32_t value;

    Hsv( uint8_t h, uint8_t s = 0, uint8_t v = 0 ) : h( h ), s( s ), v( v ), _extra( 0 ) {}
    Hsv( Rgb r );
    Hsv& operator=( Hsv h ) { swap( h ); return *this; }
    Hsv& operator=( Rgb rgb );
    void swap( Hsv& o ) { value = o.value; }
};
