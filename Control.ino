void homekitLoop() {
  if (!sys_status.module_initialized) return;

  char _cmd[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES
  int _cmdLength = 0;
  bool acted = false;

  for (uint8_t i = 0; i < sys_info.num_modules; i++) {
    int targetedAddr = i + 1;
    int mid = sys_info.modules[i][0];
    int mSwitchState = sys_info.modules[i][1];
    int hkState = Homekit.getServiceValue(i, mid);

    if (Homekit.getServiceTriggered(i, mid)) { //triggered, update module
      hkState = Homekit.getServiceValue(i, mid);
      _cmd[_cmdLength * 2] = targetedAddr;

      if (hkState) {
        Serial.println("[HOMEKIT] Switch turn ON");
        _cmd[_cmdLength * 2 + 1] = DO_TURN_ON;

        //turnSwitchOn(targetedAddr);
      }
      else {
        Serial.println("[HOMEKIT] Switch turn OFF");
        _cmd[_cmdLength * 2 + 1] = DO_TURN_OFF;

        //turnSwitchOff(targetedAddr);
      }

      _cmdLength++;
      acted = true;
      sys_info.modules[i][1] = hkState;
    } else {
      if (hkState != mSwitchState) {
        Serial.println("[HOMEKIT] Switch force change");
        Homekit.setServiceValue(i, mid, mSwitchState); //set homekit state forcibly
      }
    }
  }


  if (acted) {
    int l = _cmdLength * 2;
    char *p = (char*)malloc(l * sizeof(char));

    for (int i = 0; i < l; i++) p[i] = _cmd[i];

    sendI2CCmd(CMD_DO_MODULE, p, l);
    free(p);
  }

}

void turnSwitchOn(int addr) {
  char p[1] = {addr};
  sendDoModule(DO_TURN_ON, p, sizeof(p));
}
void turnSwitchOff(int addr) {
  char p[1] = {addr};
  sendDoModule(DO_TURN_OFF, p, sizeof(p));
}

void checkSysCurrent() {
  if (sys_info.all_current > MAX_CURRENT) { //check if system current is over loaded
    if (smf_info.advancedSMF) { //if customized emergency cutdown is enabled
      /*
         1. reverse the smfImportances to start cutting down powered plug(check the current)
      */

      for (int i = sys_info.num_modules - 1; i >= 0; i--) {
        int addr = searchAddrById(smf_info.importances[i]);

        if (sys_info.modules[addr][1] && sys_info.modules[addr][2] > 10) {
          turnSwitchOff(addr);
        }
      }
    } else {
      Serial.println("System current is overloaded! Cut down the last-plugged.");
      if (sys_info.last_plugged != 0) turnSwitchOff(sys_info.last_plugged); //cut the overloaded itself
    }
  }
}
