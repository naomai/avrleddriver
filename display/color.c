/*
 * color.c
 *
 *  Created on: 10 paŸ 2019
 *      Author: bun
 */


#include "color.h"
#include "../random.h"

const colorRaw colorBlack = {.cct={0,0,0,0,0}};
const colorRaw colorWhite = {.cct={255,255,255,255,255}};
	
// based on https://stackoverflow.com/a/14733008

colorRaw HEX2RGB(uint32_t color){
	colorRaw result;
	/*result.special = (color >> 24) & 0xFF;
	result.rgb.r = (color >> 16) & 0xFF;
	result.rgb.g = (color >> 8) & 0xFF;
	result.rgb.b = color & 0xFF;*/
	result.rgbLong = color;
	return result;
}

colorRaw HSV2RGB(colorHSV hsv)
{
	colorRaw raw;
    unsigned char region, remainder, p=0, q, t=0;
	raw.cct.cold = 0;
	raw.cct.warm = 0;

    if (hsv.s == 0)
    {
		raw.rgb.r = hsv.v;
        raw.rgb.g = hsv.v;
        raw.rgb.b = hsv.v;
		//raw.rgb.special = 0;
		//raw.white.w = p;
        return raw;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - (unsigned char)((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - (unsigned char)((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            raw.rgb.r = hsv.v; raw.rgb.g = t; raw.rgb.b = p;
            break;
        case 1:
            raw.rgb.r = q; raw.rgb.g = hsv.v; raw.rgb.b = p;
            break;
        case 2:
            raw.rgb.r = p; raw.rgb.g = hsv.v; raw.rgb.b = t;
            break;
        case 3:
            raw.rgb.r = p; raw.rgb.g = q; raw.rgb.b = hsv.v;
            break;
        case 4:
            raw.rgb.r = t; raw.rgb.g = p; raw.rgb.b = hsv.v;
            break;
        default:
            raw.rgb.r = hsv.v; raw.rgb.g = p; raw.rgb.b = q;
            break;
    }
	//raw.white.w = p;
    return raw;
}

colorHSV RGB2HSV(colorRaw color)
{
	colorHSV hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = color.rgb.r < color.rgb.g ? (color.rgb.r < color.rgb.b ? color.rgb.r : color.rgb.b) : (color.rgb.g < color.rgb.b ? color.rgb.g : color.rgb.b);
    rgbMax = color.rgb.r > color.rgb.g ? (color.rgb.r > color.rgb.b ? color.rgb.r : color.rgb.b) : (color.rgb.g > color.rgb.b ? color.rgb.g : color.rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * (uint16_t)(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == color.rgb.r)
        hsv.h = 0 + 43 * (color.rgb.g - color.rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == color.rgb.g)
        hsv.h = 85 + 43 * (color.rgb.b - color.rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (color.rgb.r - color.rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

colorRaw blendColor(colorRaw c1, colorRaw c2, uint8_t alpha){
	colorRaw newColor;
	newColor.rgb.r  = (((uint16_t)c1.rgb.r  * (256-alpha)) + ((uint16_t)c2.rgb.r  * alpha))>>8;
	newColor.rgb.g  = (((uint16_t)c1.rgb.g  * (256-alpha)) + ((uint16_t)c2.rgb.g  * alpha))>>8;
	newColor.rgb.b  = (((uint16_t)c1.rgb.b  * (256-alpha)) + ((uint16_t)c2.rgb.b  * alpha))>>8;
	newColor.white.w = (((uint16_t)c1.white.w * (256-alpha)) + ((uint16_t)c2.white.w * alpha))>>8;
	newColor.cct.warm = (((uint16_t)c1.cct.warm * (256-alpha)) + ((uint16_t)c2.cct.warm * alpha))>>8;
	//newColor.rgb.special = c2.rgb.special;
	return newColor;
}

colorRaw multiplyColor(colorRaw source, colorRaw filter){
	colorRaw newColor;
	newColor.rgb.r  = ((uint16_t)source.rgb.r * filter.rgb.r)>>8;
	newColor.rgb.g  = ((uint16_t)source.rgb.g * filter.rgb.g)>>8;
	newColor.rgb.b  = ((uint16_t)source.rgb.b * filter.rgb.b)>>8;
	newColor.white.w = ((uint16_t)source.white.w * filter.white.w )>>8;
	newColor.cct.warm = ((uint16_t)source.cct.warm * filter.cct.warm )>>8;
	return newColor;
}



uint8_t compareColors(colorRaw source, colorRaw filter){
	uint8_t smallestRatio = 255, ratio;
	if(source.rgb.r < filter.rgb.r && (ratio = ((uint16_t)source.rgb.r<<8)/filter.rgb.r) < smallestRatio)
	smallestRatio = ratio;
	if(source.rgb.g < filter.rgb.g && (ratio = ((uint16_t)source.rgb.g<<8)/filter.rgb.g) < smallestRatio)
	smallestRatio = ratio;
	if(source.rgb.b < filter.rgb.b && (ratio = ((uint16_t)source.rgb.b<<8)/filter.rgb.b) < smallestRatio)
	smallestRatio = ratio;
	if(source.white.w < filter.white.w && (ratio = ((uint16_t)source.white.w<<8)/filter.white.w) < smallestRatio)
	smallestRatio = ratio;
	if(source.cct.warm < filter.cct.warm && (ratio = ((uint16_t)source.cct.warm<<8)/filter.cct.warm) < smallestRatio)
	smallestRatio = ratio;
	return smallestRatio;
}


colorHSV randomColor(){
	uint32_t rnd = random32();
	colorHSV c;
	c.h = rnd & 0xFF;
	c.s = (rnd>>8) & 0xFF;
	c.v = (rnd>>16) & 0xFF;
	
	c.s |= 0xB0; // increased sat
	c.v |= 0xA0; // no black
	//_logf("randomColor: H%d S%d V%d\n", c.h, c.s, c.v);
	return c;
}

uint8_t getLogBrightnessForValue(uint8_t value){
	//return pgm_read_byte(&linearBrightnessLut[value]);
	return value;
}