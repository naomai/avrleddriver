/*
 * Menu.h
 *
 * Created: 27.12.2019 16:00:28
 *  Author: bun
 */ 


#ifndef MENU_H_
#define MENU_H_

#include <stdbool.h>
#include "../display/color.h"
#include "../display/animation.h"
#include "../display/light/LedLight.h"
#include "../modular/Module.h"

typedef enum {
	MENU_H,
	MENU_S,
	MENU_V,
	MENU_ID,
	MENU_PRESET,
	MENU_NOTHING,
	//MENU_FADEIN //now handled by LedDriver
} menuType;

// all the ranges are meant as: [min; max)
typedef struct{
	int16_t min;
	int16_t max;
} menuRange;

typedef struct{
	menuType menuId;
	menuRange range;
	int16_t deltaStep;
	bool cyclic;
} menuProps;

#define EVENT_MENU_SELECT 0x40
#define EVENT_MENU_VALUE 0x41

class Menu : public Module{
	protected:
	animation *lightSelectorAnim;
	uint8_t flowId;
	menuType currentMenu;
	menuProps currentMenuProps;
	LedLight * currentLight;
	uint8_t currentLightId;
	int16_t current;
	colorHSV currentHSV;
	bool keyIsDown;
	bool settingChanged;
	
	void setLightBlinking(uint8_t light, bool state);
	
	public:
	Menu();
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void frameEvent(uint16_t frameId);
	
	void select(menuType id);
	void next();
	menuType which();
	bool isMenuValid(menuType menu);
	menuProps getMenuProperties(menuType menu);

	
	int16_t getValue();
	void setValue(int16_t val);
	void setValueDelta(int16_t delta);
	
	menuRange getRange();


	static int16_t clamp(int16_t val, menuRange range);
	static int16_t clampCyclic(int16_t val, menuRange range);
	 
};




#endif /* MENU_H_ */
