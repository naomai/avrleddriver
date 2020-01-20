/*
 * ColorMapper.h
 *
 * Created: 26.12.2019 17:10:44
 *  Author: bun
 */ 


#ifndef COLORMAPPER_H_
#define COLORMAPPER_H_

#include "../color.h"
#include "../LightConfig.h"


class ColorMapper {
	protected:
	lightCalibration calibration;
	int16_t attenuationInverse; // 255/min(calibR,G,B,W1,W2)
	public:
	virtual colorRaw fromRGB(colorRaw rgb);
	virtual colorRaw toRGB(colorRaw localColor);
	void applyCalibration(colorRaw *rgb);
	void setCalibration(uint8_t calIdx);
	virtual void setCalibration(lightCalibration cal);
	static ColorMapper* createMapper(lightConfig *cfg);
	static lightCalibration getCalibrationPreset(uint8_t calIdx);
};

class ColorMapperRaw: public ColorMapper{
	public:
	ColorMapperRaw();
	colorRaw fromRGB(colorRaw rgb);
	colorRaw toRGB(colorRaw localColor);
};

class ColorMapperMono: public ColorMapper{
	public:
	ColorMapperMono();
	colorRaw filterMono;
	colorRaw fromRGB(colorRaw rgb);
	colorRaw toRGB(colorRaw localColor);
	void setCalibration(lightCalibration cal);
};

class ColorMapperCCT: public ColorMapper{
	public:
	ColorMapperCCT();
	colorRaw filterCold, filterWarm;
	colorRaw fromRGB(colorRaw rgb);
	colorRaw toRGB(colorRaw localColor);
	void setCalibration(lightCalibration cal);
};


#endif /* COLORMAPPER_H_ */