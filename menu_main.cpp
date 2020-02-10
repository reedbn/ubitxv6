#include "menu_main.h"

#include <avr/pgmspace.h>

#include "button.h"
#include "menu_utils.h"
#include "morse.h"
#include "settings.h"
#include "ubitx.h"//THRESHOLD_USB_LSB
#include "utils.h"

MenuReturn_e runMainMenu(const ButtonPress_e tuner_button,
                         const ButtonPress_e touch_button,
                         const Point touch_point,
                         const int16_t knob);

Menu_t mainMenu = {
  runMainMenu,
  nullptr
};

Menu_t* const rootMenu = &mainMenu;

bool mainMenuSelecting = false;//Tracks if we're selecting buttons with knob, or adjusting frequency
uint8_t mainMenuSelectedItemRaw = 0;

const Button mainMenuButtons [] PROGMEM = {};
static constexpr uint8_t MAIN_MENU_NUM_BUTTONS = sizeof(mainMenuButtons) / sizeof(mainMenuButtons[0]);

void drawMainMenu(void)
{
  //TODO
}

void mainMenuTune(int16_t knob)
{
  static uint32_t current_freq = 0;

  if((0 == knob) && (GetActiveVfoFreq() == current_freq)){
    //Nothing to do - we're already set!
    return;
  }

  current_freq = GetActiveVfoFreq();
  uint32_t new_freq = current_freq + (50 * knob);
  
  //Transition from below to above the traditional threshold for USB
  if(current_freq < THRESHOLD_USB_LSB && new_freq >= THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
  }
  
  //Transition from above to below the traditional threshold for USB
  if(current_freq >= THRESHOLD_USB_LSB && new_freq < THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
  }

  setFrequency(new_freq);
  current_freq = new_freq;
}

MenuReturn_e runMainMenu(const ButtonPress_e tuner_button,
                         const ButtonPress_e touch_button,
                         const Point touch_point,
                         const int16_t knob)
{
  if(runSubmenu(&mainMenu,
                drawMainMenu,
                tuner_button,
                touch_button,
                touch_point,
                knob)){
    //Submenu processed the input, so return now
    mainMenuSelectedItemRaw = 0;
    mainMenuSelecting = false;
    return MenuReturn_e::StillActive;//main menu always returns StillActive
  }//end submenu

  //Submenu didn't run, so handle the inputs ourselves

  //Check tuner_button
  if(ButtonPress_e::NotPressed != tuner_button){
    switch(tuner_button){
      default://Fallthrough intended
      case ButtonPress_e::NotPressed:
      {
        //Nothing to do
        break;
      }
      case ButtonPress_e::ShortPress:
      {
        if(mainMenuSelecting){
          uint8_t menu_index = mainMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
          //TODO: activate button
        }
        else{
          movePuck(nullptr,&mainMenuButtons[0]);
        }
        mainMenuSelecting = !mainMenuSelecting;

        //Don't handle touch or knob on this run
        return MenuReturn_e::StillActive;//main menu always returns StillActive
        break;
      }
      case ButtonPress_e::LongPress:
      {
        if(!globalSettings.morseMenuOn){
            globalSettings.morseMenuOn = true;//set before playing
            morseLetter(2);
        }
        else{
            morseLetter(4);
            globalSettings.morseMenuOn = false;//unset after playing
        }
        SaveSettingsToEeprom();
        //Don't handle touch or knob on this run
        return MenuReturn_e::StillActive;//main menu always returns StillActive
        break;
      }
    }//switch
  }//tuner_button

  else if(ButtonPress_e::NotPressed != touch_button){
    //We treat long and short presses the same, so no need to have a switch
    Button button;
    if(findPressedButton(mainMenuButtons,MAIN_MENU_NUM_BUTTONS,&button,touch_point)){
      //TODO: activate button
    }
    else{
      //Touch detected, but not on our buttons, so ignore
    }
  }//touch_button

  else{//Neither button input type found, so handle the knob
    if(mainMenuSelecting){
      const uint8_t prev_select = mainMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
      mainMenuSelectedItemRaw += LIMIT(mainMenuSelectedItemRaw+knob,0,MAIN_MENU_NUM_BUTTONS*MENU_KNOB_COUNTS_PER_ITEM);
      const uint8_t new_select = mainMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
      if(prev_select != new_select){
        movePuck(&mainMenuButtons[prev_select],&mainMenuButtons[new_select]);//TODO
        morseLetter(mainMenuButtons[new_select].morse);
        int8_t morse_status = 0;
        mainMenuButtons[new_select].morse_status(morse_status);
        if(morse_status < 0){
          morseBool(false);
        }
        else if(morse_status > 0){
          morseBool(true);
        }
      }
    }
    else{
      mainMenuTune(knob);
    }
  }

  //

  return MenuReturn_e::StillActive;//main menu always returns StillActive
}