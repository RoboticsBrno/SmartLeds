#include "Color.h"
#include <algorithm>
#include <cmath>

/* RGB <-> HSV conversion adapted from
https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
*/

Rgb::Rgb( Hsv y ) {
    float saturation = y.s / 255.0;
    int color = y.v * saturation;
    float hPrime = fmod((float)y.h / 60.0, 6);
    float x = color * (1 - fabs(fmod(hPrime, 2) - 1));
    float m = y.v - color;
    
    if (0 <= hPrime && hPrime < 1) {
        r = color;
        g = static_cast<uint8_t>(x + 0.5);
        b = 0;
    }
    else if (1 <= hPrime && hPrime < 2) {
        r = static_cast<uint8_t>(x + 0.5);
        g = color;
        b = 0;
    }
    else if(2 <= hPrime && hPrime < 3) {
        r = 0;
        g = color;
        b = static_cast<uint8_t>(x + 0.5);
    }
    else if(3 <= hPrime && hPrime < 4) {
        r = 0;
        g = static_cast<uint8_t>(x + 0.5);
        b = color;
    }
    else if(4 <= hPrime && hPrime < 5) {
        r = static_cast<uint8_t>(x + 0.5);
        g = 0;
        b = color;
    }
    else if(5 <= hPrime && hPrime < 6) {
        r = color;
        g = 0;
        b = static_cast<uint8_t>(x + 0.5);
    }
    else {
        r = 0;
        g = 0;
        b = 0;
    }
    
    r += m;
    g += m;
    b += m;
}

Rgb& Rgb::operator=( Hsv hsv ) {
    Rgb r{ hsv };
    swap( r );
    return *this;
}

Hsv::Hsv( Rgb r ) {
    float colorMax = std::max(std::max(r.r, r.g), r.b);
    float colorMin = std::min(std::min(r.r, r.g), r.b);
    float delta = colorMax - colorMin;
    float saturation = 0.0;
    float hue = 0.0;
    float value = 0.0;
    
    if(delta > 0) {
        if(colorMax == r.r) {
        hue = 60 * (fmod(((r.g - r.b) / delta), 6));
        } else if(colorMax == r.g) {
        hue = 60 * (((r.b - r.r) / delta) + 2);
        } else if(colorMax == r.b) {
        hue = 60 * (((r.r - r.g) / delta) + 4);
        }
        
        if(colorMax > 0) {
        saturation = delta / colorMax;
        } else {
        saturation = 0;
        }
        
        value = colorMax;
    } else {
        hue = 0;
        saturation = 0;
        value = colorMax;
    }
    
    if(hue < 0) {
        hue = 360 + hue;
    }
    saturation *= 100;
    h = static_cast<uint16_t>(hue + 0.5);
    s = static_cast<uint8_t>(saturation + 0.5);
    v = static_cast<uint8_t>(value + 0.5);

}

Hsv& Hsv::operator=( Rgb rgb ) {
    Hsv h{ rgb };
    swap( h );
    return *this;
}