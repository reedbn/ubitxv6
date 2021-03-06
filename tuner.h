#pragma once

#include "settings.h"

void saveVFOs();
void setFrequency(const unsigned long freq, const bool transmit = false);
void startTx(TuningMode_e tx_mode);
void stopTx();
void ritEnable(unsigned long f);
void ritDisable();
void checkCAT();
void cwKeyer(void);
void switchVFO(Vfo_e vfoSelect);

bool autoSelectSidebandChanged(const uint32_t old_frequency); //if the current frequency defaults to a different sideband mode, updates to that sideband mode and returns true. Else, returns false
