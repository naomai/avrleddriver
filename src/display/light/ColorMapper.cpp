/*
 * ColorMapper.cpp
 *
 * Created: 26.12.2019 17:10:35
 *  Author: bun
 */ 

#include "ColorMapper.h"

#pragma GCC optimize("O0")
static uint8_t clampUByte(int16_t val);
static int16_t calculateIntensity(uint8_t val, uint8_t calibrationFactor, int16_t attenuationInv);
static colorRaw colorNormalize(colorRaw16 color);

extern const lightCalibration PROGMEM calibrationTable[];

void ColorMapper::setCalibration(uint8_t calIdx){
	this->setCalibration(ColorMapper::getCalibrationPreset(calIdx));
}

void ColorMapper::setCalibration(lightCalibration cal){
	this->calibration = cal;
	int16_t minPower = 255;
	if(cal.dr && cal.dr < minPower) minPower = cal.dr;
	if(cal.dg && cal.dg < minPower) minPower = cal.dg;
	if(cal.db && cal.db < minPower) minPower = cal.db;
	if(cal.dc1 && cal.dc1 < minPower) minPower = cal.dc1;
	if(cal.dc2 && cal.dc2 < minPower) minPower = cal.dc2;
	attenuationInverse = 65535 / minPower;
}

void ColorMapper::applyCalibration(colorRaw *rgb){
	colorRaw16 calcColor;
	calcColor.r = calculateIntensity(rgb->rgb.r, this->calibration.dr, attenuationInverse);
	calcColor.g = calculateIntensity(rgb->rgb.g, this->calibration.dg, attenuationInverse);
	calcColor.b = calculateIntensity(rgb->rgb.b, this->calibration.db, attenuationInverse);
	calcColor.cold = calculateIntensity(rgb->cct.cold, this->calibration.dc1, attenuationInverse);
	calcColor.warm = calculateIntensity(rgb->cct.warm, this->calibration.dc2, attenuationInverse);
	*rgb = colorNormalize(calcColor);

}

ColorMapper* ColorMapper::createMapper(lightConfig *cfg){
	ColorMapper *m;
	if((cfg->type & LIGHT_CCT) == LIGHT_CCT){
		m = new ColorMapperCCT;
	}else if((cfg->type & LIGHT_WHITE) == LIGHT_WHITE){
		m = new ColorMapperMono;
	}else{
		m = new ColorMapperRaw;
	}
	m->setCalibration(cfg->calibrationIdx);
	return m;
}

colorRaw ColorMapper::fromRGB(colorRaw rgb){
	return rgb;
}

colorRaw ColorMapper::toRGB(colorRaw localColor){
	return localColor;
}

lightCalibration ColorMapper::getCalibrationPreset(uint8_t calIdx){
	lightCalibration cal;
	memcpy_P(&cal, &calibrationTable[calIdx], sizeof(lightCalibration));
	return cal;
}

/* ColorMapperRaw */

ColorMapperRaw::ColorMapperRaw(){
	
}

colorRaw ColorMapperRaw::fromRGB(colorRaw rgb){
	this->applyCalibration(&rgb);
	return rgb;
}

colorRaw ColorMapperRaw::toRGB(colorRaw localColor){
	return localColor;
}

/* ColorMapperMono */

ColorMapperMono::ColorMapperMono(){
	
}

void ColorMapperMono::setCalibration(lightCalibration cal){
	ColorMapper::setCalibration(cal);
	this->filterMono.rgb = cal.add1;
}

colorRaw ColorMapperMono::fromRGB(colorRaw rgb){
	uint8_t colorContent = compareColors(rgb, this->filterMono);
	rgb.white.w = colorContent;
	//colorContent = (colorContent * (uint16_t)this->calibration.dc1)>>8;
	rgb.rgb.r = clampUByte(rgb.rgb.r - ((this->filterMono.rgb.r * (uint16_t)colorContent)>>8));
	rgb.rgb.g = clampUByte(rgb.rgb.g - ((this->filterMono.rgb.g * (uint16_t)colorContent)>>8));
	rgb.rgb.b = clampUByte(rgb.rgb.b - ((this->filterMono.rgb.b * (uint16_t)colorContent)>>8));

	this->applyCalibration(&rgb);
	return rgb;
}

colorRaw ColorMapperMono::toRGB(colorRaw localColor){
	colorRaw16 calcColor;
	colorRaw result;
	calcColor.r = (uint16_t)localColor.rgb.r + ((this->filterMono.rgb.r * (uint16_t)localColor.white.w)>>8);
	calcColor.g = (uint16_t)localColor.rgb.g + ((this->filterMono.rgb.g * (uint16_t)localColor.white.w)>>8);
	calcColor.b = (uint16_t)localColor.rgb.b + ((this->filterMono.rgb.b * (uint16_t)localColor.white.w)>>8);
	result = colorNormalize(calcColor);
	
	return result;
}

/* ColorMapperCCT */ 

ColorMapperCCT::ColorMapperCCT(){
	
}

void ColorMapperCCT::setCalibration(lightCalibration cal){
	ColorMapper::setCalibration(cal);
	this->filterCold.rgb = cal.add1;
	this->filterWarm.rgb = cal.add2;
}

colorRaw ColorMapperCCT::fromRGB(colorRaw rgb){
	uint8_t colorContent;
	colorContent = compareColors(rgb, this->filterWarm);
	rgb.cct.warm = colorContent;
	//colorContent = (colorContent * (uint16_t)this->calibration.dc2)>>8;
	rgb.rgb.r = clampUByte(rgb.rgb.r - ((this->filterWarm.rgb.r * (uint16_t)colorContent)>>8));
	rgb.rgb.g = clampUByte(rgb.rgb.g - ((this->filterWarm.rgb.g * (uint16_t)colorContent)>>8));
	rgb.rgb.b = clampUByte(rgb.rgb.b - ((this->filterWarm.rgb.b * (uint16_t)colorContent)>>8));
	
	colorContent = compareColors(rgb, this->filterCold);
	rgb.cct.cold = colorContent;
	//colorContent = (colorContent * (uint16_t)this->calibration.dc1)>>8;
	rgb.rgb.r = clampUByte(rgb.rgb.r - ((this->filterCold.rgb.r * (uint16_t)colorContent)>>8));
	rgb.rgb.g = clampUByte(rgb.rgb.g - ((this->filterCold.rgb.g * (uint16_t)colorContent)>>8));
	rgb.rgb.b = clampUByte(rgb.rgb.b - ((this->filterCold.rgb.b * (uint16_t)colorContent)>>8));
	this->applyCalibration(&rgb);
	return rgb;
}

colorRaw ColorMapperCCT::toRGB(colorRaw localColor){
	colorRaw result=localColor;
	
	result.rgb.r = clampUByte(result.rgb.r + ((this->filterCold.rgb.r * (uint16_t)localColor.cct.cold)>>8));
	result.rgb.g = clampUByte(result.rgb.r + ((this->filterCold.rgb.g * (uint16_t)localColor.cct.cold)>>8));
	result.rgb.b = clampUByte(result.rgb.r + ((this->filterCold.rgb.b * (uint16_t)localColor.cct.cold)>>8));
	
	result.rgb.r = clampUByte(result.rgb.r + ((this->filterWarm.rgb.r * (uint16_t)localColor.cct.warm)>>8));
	result.rgb.g = clampUByte(result.rgb.r + ((this->filterWarm.rgb.g * (uint16_t)localColor.cct.warm)>>8));
	result.rgb.b = clampUByte(result.rgb.r + ((this->filterWarm.rgb.b * (uint16_t)localColor.cct.warm)>>8));

	return result;
}

uint8_t clampUByte(int16_t val){
	return (val>255) ? 255 : (val<0 ? 0 : val);
}

int16_t calculateIntensity(uint8_t val, uint8_t calibrationFactor, int16_t attenuationInv){
	int16_t out;
	if(calibrationFactor==0) {
		return 0;
	}
	out = (val * (uint32_t)attenuationInv) >> 8;
	if(calibrationFactor!=255){
		out = (out * (uint32_t)calibrationFactor) >> 8;
	}
	return out;
}

colorRaw colorNormalize(colorRaw16 color){
	uint8_t attenuate;
	colorRaw newColor;
	int16_t max = color.r;
	if(color.g>max) max = color.g;
	if(color.b>max) max = color.b;
	if(color.cold>max) max = color.cold;
	if(color.warm>max) max = color.warm;
	if(max > 255){
		attenuate = 65535 / max;
		newColor.rgb.r = (color.r * attenuate)>>8;
		newColor.rgb.g = (color.g * attenuate)>>8;
		newColor.rgb.b = (color.b * attenuate)>>8;
		newColor.cct.cold = (color.cold * attenuate)>>8;
		newColor.cct.warm = (color.warm * attenuate)>>8;
	}else{
		newColor.rgb.r = color.r;
		newColor.rgb.g = color.g;
		newColor.rgb.b = color.b;
		newColor.cct.cold = color.cold;
		newColor.cct.warm = color.warm;
	}
	return newColor;
}

