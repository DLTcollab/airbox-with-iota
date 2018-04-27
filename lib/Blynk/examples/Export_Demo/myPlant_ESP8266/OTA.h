/**************************************************************
 * This is a DEMO. You can use it only for development and testing.
 *
 * If you would like to add these features to your product,
 * please contact Blynk for Business:
 *
 *                  http://www.blynk.io/
 *
 **************************************************************/

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

String overTheAirURL;

BLYNK_WRITE(InternalPinOTA) {
  // URL should be something like "http://ota-server:port/firmware.bin?auth=12345"
  overTheAirURL = param.asString();
  overTheAirURL += String("?auth=") + configStore.cloudToken;

  // Disconnect, not to interfere with OTA process
  Blynk.disconnect();

  // Start OTA
  BlynkState::set(MODE_OTA_UPGRADE);
  delay(500);
}

void enterOTA() {
  BlynkState::set(MODE_OTA_UPGRADE);

  DEBUG_PRINT(String("Firmware update URL: ") + overTheAirURL);
  switch (ESPhttpUpdate.update(overTheAirURL, BOARD_FIRMWARE_VERSION)) {
  case HTTP_UPDATE_FAILED:
    DEBUG_PRINT(String("Firmware update failed (error ") + ESPhttpUpdate.getLastError() + "): " + ESPhttpUpdate.getLastErrorString());
    BlynkState::set(MODE_ERROR);
    break;
  case HTTP_UPDATE_NO_UPDATES:
    DEBUG_PRINT("No firmware updates available.");
    BlynkState::set(MODE_CONNECTING_CLOUD);
    break;
  case HTTP_UPDATE_OK:
    DEBUG_PRINT("Firmware updated.");
    restartMCU();
    break;
  }
}

