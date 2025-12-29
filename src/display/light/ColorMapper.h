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
	uint8_t calibrationIdx;
	int16_t attenuationInverse; // 255/min(calibR,G,B,W1,W2)
	public:
	virtual colorRaw fromRGB(colorRaw rgb);
	virtual void toRGB(colorRaw *localColor);
	void applyCalibration(colorRaw *rgb);
	virtual void setCalibration(uint8_t calIdx);
	static ColorMapper* createMapper(entityConfig *cfg);
	static void getCalibrationPreset(uint8_t calIdx, lightCalibration *cal);
};

class ColorMapperRaw: public ColorMapper{
	public:
	ColorMapperRaw();
	colorRaw fromRGB(colorRaw rgb);
	void toRGB(colorRaw *localColor);
};

class ColorMapperMono: public ColorMapper{
	public:
	ColorMapperMono();
	colorRaw filterMono;
	colorRaw fromRGB(colorRaw rgb);
	void toRGB(colorRaw *localColor);
	void setCalibration(uint8_t calIdx);
};

class ColorMapperCCT: public ColorMapper{
	public:
	ColorMapperCCT();
	colorRaw filterCold, filterWarm;
	colorRaw fromRGB(colorRaw rgb);
	void toRGB(colorRaw *localColor);
	void setCalibration(uint8_t calIdx);
};


#endif /* COLORMAPPER_H_ */