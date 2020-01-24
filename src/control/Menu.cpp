/*
 * Menu.cpp
 *
 * Created: 27.12.2019 16:00:39
 *  Author: bun
 */ 

#include <stddef.h>
#include <avr/pgmspace.h>
#include "Menu.h"
#include "../LedDriver.h"
#include "../control/EncoderModule.h"
#include "../config.h"
#include "../types.h"

extern PowerManagement *pwr;

const uint32_t colorPresets[] PROGMEM = {
	0xFFD876, 0xFFEAAA, 0xFFFFEE, 0xFF0000, 0xFF8800, 0xFFBB00, 0xFFDD00, 0xFFFF00,
	0xCCFF00, 0x99FF00, 0x00FF00, 0x00FFAA, 0x00FFD8, 0x00FFFF, 0x00DDFF,
	0x0099FF, 0x0000FF, 0x9900FF, 0xDD00FF, 0xFF00DD, 0xFF0066, (uint32_t)P_SPECIAL_RANDOM<<24
};

const menuType menuFlow[] = {
	MENU_NOTHING,MENU_ID,MENU_PRESET,MENU_V
};

const menuProps menuProperties[] PROGMEM = {
	{MENU_NOTHING, {0,2}, 0, false},
	{MENU_ID, {0,STRIPS_COUNT}, 1, true},
	{MENU_PRESET, {0,count(colorPresets)}, 1, true},
	{MENU_H, {0,256}, 8, false},
	{MENU_S, {0,256}, 8, false},
	{MENU_V, {0,256}, 8, false}
};

extern LedDriver *lights;


Menu::Menu() : Module(){
	this->lightSelectorAnim=NULL;
	this->flowId = 0;
	this->currentMenu = MENU_NOTHING;
	this->currentMenuProps = this->getMenuProperties(this->currentMenu);
	this->currentLightId = 0;
	this->currentLight = lights->getLightById(this->currentLightId);
	this->settingChanged = false;
	current=255;
}

void Menu::select(menuType id){
	menuType prevMenu = currentMenu;
	if(this->isMenuValid(id)){
		currentMenu = id;
		for(uint8_t i=0; i<count(menuFlow); i++){
			if(menuFlow[i]==id) flowId = i;
		}
		currentMenuProps = this->getMenuProperties(currentMenu);
		
		this->raiseEvent(EVENT_MENU_SELECT, currentMenu, prevMenu);
		
		
		// stop LedDriver from resetting userColor
		if(currentMenu == MENU_NOTHING){
			currentLight->special &= ~(P_SPECIAL_DONTRESET);
			if(settingChanged){
				currentLight->setColor(HSV2RGB(currentHSV), LIGHT_COLOR_SET);
				settingChanged = false;
			}
		}else{
			currentLight->special |= (P_SPECIAL_DONTRESET);
		}
		
		currentLight->applySpecialColor();
		
		/* menu initialization */
		if(currentMenu == MENU_ID){
			current = currentLightId;
		}else if(currentMenu == MENU_V){
			current = currentHSV.v;
		}else if(currentMenu == MENU_PRESET){
			current = 0;
		}
		this->setValue(current);
	}
}

void Menu::next(){
	menuType tempMenu;
	do{
		//flowId = repeatingScroller(flowId + 1, count(menuFlow));
		flowId++;
		if(flowId >= count(menuFlow)) flowId=0;
		tempMenu = menuFlow[flowId];
	}while(!this->isMenuValid(tempMenu));
	this->select(tempMenu);
}

bool Menu::isMenuValid(menuType menu){
	if(menu == MENU_ID && STRIPS_COUNT == 1)
		// skip strip selection if there's only 1
		return false;
	if(menu == MENU_V && (currentLight->special & 0x0F)) 
		// skip brightness setting for random color
		return false;
	
	return true;
}

menuType Menu::which(){
	return currentMenu;	
}
menuProps Menu::getMenuProperties(menuType menu){
	menuProps props;
	for(uint8_t i=0; i<count(menuProperties); i++){
		memcpy_P(&props, &menuProperties[i], sizeof(menuProps));
		if(props.menuId == menu){
			return props;
		}
	}
	return props;
}


int16_t Menu::getValue(){
	return current;
}

void Menu::setValue(int16_t val){
	current = Menu::clamp(val, currentMenuProps.range);
	this->raiseEvent(EVENT_MENU_VALUE, currentMenu, (uint8_t)(current & 0xFF));
	
	switch(currentMenu){
		case MENU_ID:
			currentLight->resetTempColor();
			currentLightId = current;
			currentLight = lights->getLightById(currentLightId);
			Menu::setLightBlinking(currentLightId, true);
			break;
		case MENU_PRESET:
		{
			uint32_t currentPreset = pgm_read_dword(&(colorPresets[current]));
			currentHSV = RGB2HSV(HEX2RGB(currentPreset & 0xFFFFFF));
			currentLight->setColor(HSV2RGB(currentHSV), LIGHT_COLOR_DISPLAY);
			currentLight->setSpecialAttribute(currentPreset >> 24);
			settingChanged = true;
			break;
		}
		case MENU_V:
			currentHSV.v = current;
			currentLight->setColor(HSV2RGB(currentHSV), LIGHT_COLOR_DISPLAY);
			settingChanged = true;
			break;
		default:
			currentLight->resetTempColor();
			break;
	}
}

void Menu::setValueDelta(int16_t delta){
	int16_t newVal;
	if(delta==0 || currentMenuProps.deltaStep==0) return;
	
	newVal = this->getValue();
	newVal += delta * currentMenuProps.deltaStep;
	if(currentMenuProps.cyclic){
		newVal = Menu::clampCyclic(newVal, currentMenuProps.range);
	}
	this->setValue(newVal);
}

menuRange Menu::getRange(){
	return currentMenuProps.range;
}

int16_t Menu::clamp(int16_t val, menuRange range){
	return (val>range.max-1) ? range.max-1 : (val<range.min ? range.min : val);
}
int16_t Menu::clampCyclic(int16_t val, menuRange range){
	int16_t result, totalRange;
	result = val - range.min;
	totalRange = (range.max - range.min);
	
	result = result % totalRange;
	if(result < 0){
		result += totalRange;
	}
	result += range.min;
	return result;
}
void Menu::setLightBlinking(uint8_t light, bool state){
	LedLight *s = lights->getLightById(light);
	
	if(lightSelectorAnim){
		LedLight *previous = lights->getLightById(lightSelectorAnim->stripId);
		animCancel(lightSelectorAnim);
		previous->resetTempColor();
		lightSelectorAnim = NULL;
	}
	if(state){
		colorRaw userColor = s->getColor(LIGHT_COLOR_USER);
		colorRaw halfColor = blendColor(colorBlack, userColor, 192);
		lightSelectorAnim = animCreate(light, halfColor, userColor, 2300, ANIM_LOOP_ALTERNATING);
		animStart(lightSelectorAnim);
	}
}

void Menu::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	if(pwr->state != PS_ON) return;
	switch(type){
		case EVENT_INPUT_KEY:
			if(hbyte==KEY_ENTER){
				if(lbyte == KEYSTATE_DOWN){
					// previous menu stuff
					if(currentMenu == MENU_ID){
						this->setLightBlinking(currentLightId, false);
					}
					keyIsDown = true;
				}else if(lbyte == KEYSTATE_UP){
					this->next();
					keyIsDown = false;
				}
			}
			break;
		case EVENT_INPUT_DELTA:
			this->setValueDelta((int16_t)((int8_t)lbyte));
			break;
		case EVENT_FRAME:
			this->frameEvent((hbyte<<8) | lbyte);
			break;
		default:
			return;
	}
}


void Menu::frameEvent(uint16_t frameId){
	switch(currentMenu){
		case MENU_PRESET:
		case MENU_H:
		case MENU_S:
		case MENU_V:
		
			//currentLight->special |= (P_SPECIAL_DONTRESET);

			// settings blinker (to remind that we're in menu)
			if((frameId & 63) == 63){
				currentLight->setColor(colorBlack, LIGHT_COLOR_DISPLAY);
			}else if((frameId & 63) == 0){
				//currentLight->resetTempColor();
				currentLight->setColor(HSV2RGB(currentHSV), LIGHT_COLOR_DISPLAY);
			}
			// rainbow effect for random color
			if((frameId & 7) == 0){
				currentLight->applySpecialColor();
				//currentLight->resetTempColor();
			}
			break;
		default:
			break;
	}
	if(keyIsDown){
		currentLight->setColor(colorBlack,LIGHT_COLOR_DISPLAY);
	}
}
