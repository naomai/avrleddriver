/*
 * ColorMapper.cpp
 *
 * Created: 26.12.2019 17:10:35
 *  Author: bun
 */ 

#include "ColorMapper.h"

void ColorMapper::setCalibration(lightCalibration cal){
	this->calibration = cal;
}

ColorMapper* ColorMapper::createMapper(lightConfig *cfg){
	ColorMapper *m;
	switch(cfg->type){
		case LIGHT_RGBW:
			m = new ColorMapperMono;
			break;
		case LIGHT_CCT:
			m = new ColorMapperCCT;
			break;
		default:
			m = new ColorMapperRaw;
			break;
	}
	m->setCalibration(cfg->calibration);
	return m;
}

colorRaw ColorMapper::fromRGB(colorRaw rgb){
	return rgb;
}

colorRaw ColorMapper::toRGB(colorRaw localColor){
	return localColor;
}

/* ColorMapperRaw */

ColorMapperRaw::ColorMapperRaw(){
	
}

colorRaw ColorMapperRaw::fromRGB(colorRaw rgb){
	rgb.rgb.r = ((uint16_t)rgb.rgb.r * this->calibration.dr)>>8;
	rgb.rgb.g = ((uint16_t)rgb.rgb.g * this->calibration.dg)>>8;
	rgb.rgb.b = ((uint16_t)rgb.rgb.b * this->calibration.db)>>8;
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
	rgb.white.w = compareColors(rgb, this->filterMono);
	return rgb;
}

colorRaw ColorMapperMono::toRGB(colorRaw localColor){
	colorRaw result;
	result.rgb.r = (this->filterMono.rgb.r * (uint16_t)localColor.white.w)>>8;
	result.rgb.g = (this->filterMono.rgb.g * (uint16_t)localColor.white.w)>>8;
	result.rgb.b = (this->filterMono.rgb.b * (uint16_t)localColor.white.w)>>8;
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
	rgb.cct.cold = compareColors(rgb, this->filterCold);
	rgb.cct.warm = compareColors(rgb, this->filterWarm);
	return rgb;
}

colorRaw ColorMapperCCT::toRGB(colorRaw localColor){
	colorRaw result;
	result.rgb.r = (this->filterCold.rgb.r * (uint16_t)localColor.cct.cold)>>9;
	result.rgb.g = (this->filterCold.rgb.g * (uint16_t)localColor.cct.cold)>>9;
	result.rgb.b = (this->filterCold.rgb.b * (uint16_t)localColor.cct.cold)>>9;
	
	result.rgb.r += (this->filterWarm.rgb.r * (uint16_t)localColor.cct.warm)>>9;
	result.rgb.g += (this->filterWarm.rgb.g * (uint16_t)localColor.cct.warm)>>9;
	result.rgb.b += (this->filterWarm.rgb.b * (uint16_t)localColor.cct.warm)>>9;
	return result;
}
