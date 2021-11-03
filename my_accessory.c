
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

char accessoryName[] = "Prophetic Maquina Fumaca";
char accessoryManufacturer[] = "PROPHETIC";
char accessorySerialNumber[] = "000003";
char accessoryModel[] = "Fuamca";
char accessoryFirmwareRevision[] = "1.1";
char accessoryPassword[] = "333-33-333";

extern void identify_accessory(homekit_value_t);
extern void identify_switch_1(homekit_value_t);
homekit_characteristic_t cha_switch_on1 = HOMEKIT_CHARACTERISTIC_(ON, true);

//HOMEKIT_SERVICE_LIGHTBULB

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_switch, .services = (homekit_service_t *[]){// HAP section 8.17:
                                                                                                                  // For a bridge accessory, only the primary HAP accessory object must contain this(INFORMATION) service.
                                                                                                                  // Here the bridged accessories must contain an INFORMATION service,
                                                                                                                  // otherwise the HomeKit will reject to pair.
                                                                                                                  HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]){HOMEKIT_CHARACTERISTIC(NAME, accessoryName), HOMEKIT_CHARACTERISTIC(MANUFACTURER, accessoryManufacturer), HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, accessorySerialNumber), HOMEKIT_CHARACTERISTIC(MODEL, accessoryModel), HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, accessoryFirmwareRevision), HOMEKIT_CHARACTERISTIC(IDENTIFY, identify_accessory), NULL}), NULL}),

    HOMEKIT_ACCESSORY(.id = 2, .category = homekit_accessory_category_switch, .services = (homekit_service_t *[]){HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]){HOMEKIT_CHARACTERISTIC(NAME, "Maquina Fumaca"), HOMEKIT_CHARACTERISTIC(IDENTIFY, identify_switch_1), NULL}), HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t *[]){&cha_switch_on1, HOMEKIT_CHARACTERISTIC(NAME, "Maquina Fumuca"), NULL}), NULL}),


    NULL};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = accessoryPassword};
