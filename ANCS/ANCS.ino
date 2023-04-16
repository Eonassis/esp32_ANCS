#define CORE_DEBUG_LEVEL 3
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEClient.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <esp_bt_main.h>
#include <string>
#include <Task.h>
#include <sys/time.h>
#include <time.h>
#include "sdkconfig.h"

 

static char LOG_TAG[] = "SampleServer";

static BLEUUID ancsServiceUUID("7905F431-B5CE-4E99-A40F-4B1E122D00D0");
static BLEUUID notificationSourceCharacteristicUUID("9FBF120D-6301-42D9-8C58-25E699A21DBD");
static BLEUUID controlPointCharacteristicUUID("69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9");
static BLEUUID dataSourceCharacteristicUUID("22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB");

class MySecurity2 : public BLESecurityCallbacks {

    uint32_t onPassKeyRequest(){
        ESP_LOGI(LOG_TAG, "PassKeyRequest");
        return 123456;
    }

    void onPassKeyNotify(uint32_t pass_key){
        ESP_LOGI(LOG_TAG, "On passkey Notify number:%d", pass_key);
    }

    bool onSecurityRequest(){
        ESP_LOGI(LOG_TAG, "On Security Request");
        return true;
    }
    
    bool onConfirmPIN(unsigned int){
        ESP_LOGI(LOG_TAG, "On Confrimed Pin Request");
        return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
        ESP_LOGI(LOG_TAG, "Starting BLE work!");
        if(cmpl.success){
            uint16_t length;
            esp_ble_gap_get_whitelist_size(&length);
            ESP_LOGD(LOG_TAG, "size: %d", length);
        }
    }
};


class MySecurity : public BLESecurityCallbacks {

uint32_t onPassKeyRequest(){
ESP_LOGI(LOG_TAG, "PassKeyRequest");
return 123456;
}

void onPassKeyNotify(uint32_t pass_key){
ESP_LOGI(LOG_TAG, "On passkey Notify number:%d", pass_key);
USBSerial.println("onPassKeyNotify - the key passed : ");
USBSerial.println(pass_key);
}

bool onSecurityRequest(){
ESP_LOGI(LOG_TAG, "On Security Request");
USBSerial.println("onSecurityRequest");
return true;
}

void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
ESP_LOGI(LOG_TAG, "Starting BLE work!");
if(cmpl.success){
uint16_t length;
USBSerial.println("Auth Complete");
esp_ble_gap_get_whitelist_size(&length);
ESP_LOGD(LOG_TAG, "size: %d", length);
}
}
bool onConfirmPIN(uint32_t pass_key){
ESP_LOGI(LOG_TAG, "The passkey YES/NO number:%d", pass_key);
USBSerial.println("onConfirmPIN - pass_key:");
USBSerial.println(pass_key);
vTaskDelay(500);
return false;
}
};

static void dataSourceNotifyCallback(
  BLERemoteCharacteristic* pDataSourceCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    USBSerial.print("Notify callback for characteristic ");
    USBSerial.print(pDataSourceCharacteristic->getUUID().toString().c_str());
    USBSerial.print(" of data length ");
    USBSerial.println(length);
}

static void NotificationSourceNotifyCallback(
  BLERemoteCharacteristic* pNotificationSourceCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
    if(pData[0]==0)
    {
        USBSerial.println("New notification!");
        switch(pData[2])
        {
            case 0:
                USBSerial.println("Category: Other");
            break;
            case 1:
                USBSerial.println("Category: Incoming call");
            break;
            case 2:
                USBSerial.println("Category: Missed call");
            break;
            case 3:
                USBSerial.println("Category: Voicemail");
            break;
            case 4:
                USBSerial.println("Category: Social");
            break;
            case 5:
                USBSerial.println("Category: Schedule");
            break;
            case 6:
                USBSerial.println("Category: Email");
            break;
            case 7:
                USBSerial.println("Category: News");
            break;
            case 8:
                USBSerial.println("Category: Health");
            break;
            case 9:
                USBSerial.println("Category: Business");
            break;
            case 10:
                USBSerial.println("Category: Location");
            break;
            case 11:
                USBSerial.println("Category: Entertainment");
            break;
            default:
            break;
        }
    }
}

/**
 * Become a BLE client to a remote BLE server.  We are passed in the address of the BLE server
 * as the input parameter when the task is created.
 */
class MyClient: public Task {
    void run(void* data) {

        BLEAddress* pAddress = (BLEAddress*)data;
        BLEClient*  pClient  = BLEDevice::createClient();
        BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
        BLEDevice::setSecurityCallbacks(new MySecurity());

        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
        pSecurity->setCapability(ESP_IO_CAP_IO);
        pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
        // Connect to the remove BLE Server.
        pClient->connect(*pAddress);

        /** BEGIN ANCS SERVICE **/
        // Obtain a reference to the service we are after in the remote BLE server.
        BLERemoteService* pAncsService = pClient->getService(ancsServiceUUID);
        if (pAncsService == nullptr) {
            ESP_LOGD(LOG_TAG, "Failed to find our service UUID: %s", ancsServiceUUID.toString().c_str());
            return;
        }
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        BLERemoteCharacteristic* pNotificationSourceCharacteristic = pAncsService->getCharacteristic(notificationSourceCharacteristicUUID);
        if (pNotificationSourceCharacteristic == nullptr) {
            ESP_LOGD(LOG_TAG, "Failed to find our characteristic UUID: %s", notificationSourceCharacteristicUUID.toString().c_str());
            return;
        }        
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        BLERemoteCharacteristic* pControlPointCharacteristic = pAncsService->getCharacteristic(controlPointCharacteristicUUID);
        if (pControlPointCharacteristic == nullptr) {
            ESP_LOGD(LOG_TAG, "Failed to find our characteristic UUID: %s", controlPointCharacteristicUUID.toString().c_str());
            return;
        }        
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        BLERemoteCharacteristic* pDataSourceCharacteristic = pAncsService->getCharacteristic(dataSourceCharacteristicUUID);
        if (pDataSourceCharacteristic == nullptr) {
            ESP_LOGD(LOG_TAG, "Failed to find our characteristic UUID: %s", dataSourceCharacteristicUUID.toString().c_str());
            return;
        }        
        const uint8_t v[]={0x1,0x0};
        pDataSourceCharacteristic->registerForNotify(dataSourceNotifyCallback);
        pDataSourceCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)v,2,true);
        pNotificationSourceCharacteristic->registerForNotify(NotificationSourceNotifyCallback);
        pNotificationSourceCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)v,2,true);
        /** END ANCS SERVICE **/
    } // run
}; // MyClient

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
        USBSerial.println("********************");
        USBSerial.println("**Device connected**");
        USBSerial.println(BLEAddress(param->connect.remote_bda).toString().c_str());
        USBSerial.println("********************");
        MyClient* pMyClient = new MyClient();
        pMyClient->setStackSize(18000);
        pMyClient->start(new BLEAddress(param->connect.remote_bda));
    };

    void onDisconnect(BLEServer* pServer) {
        USBSerial.println("************************");
        USBSerial.println("**Device  disconnected**");
        USBSerial.println("************************");
    }
};

class MainBLEServer: public Task {
   //USBSerial.println("Start Task MainBLEServer");
    void run(void *data) {
        USBSerial.println("Start MainBLEServer");
        ESP_LOGD(LOG_TAG, "Starting BLE work!");
        esp_log_buffer_char(LOG_TAG, LOG_TAG, sizeof(LOG_TAG));
        esp_log_buffer_hex(LOG_TAG, LOG_TAG, sizeof(LOG_TAG));

        // Initialize device
        BLEDevice::init("ANCS");
        BLEServer* pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
        BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
        BLEDevice::setSecurityCallbacks(new MySecurity());

        // Advertising parameters:
        // Soliciting ANCS
        BLEAdvertising *pAdvertising = pServer->getAdvertising();
        BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
        oAdvertisementData.setFlags(0x01);
        oAdvertisementData.setServiceSolicitation(BLEUUID("7905F431-B5CE-4E99-A40F-4B1E122D00D0"));
        pAdvertising->setAdvertisementData(oAdvertisementData);        

        // Set security
        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
        pSecurity->setCapability(ESP_IO_CAP_OUT);
        pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

        //Start advertising
        pAdvertising->start();
        
        ESP_LOGD(LOG_TAG, "Advertising started!");
        delay(portMAX_DELAY);
    }
};

void SampleSecureServer(void)
{
    
    USBSerial.println("Start SampleSecureServer");
    MainBLEServer* pMainBleServer = new MainBLEServer();
    pMainBleServer->setStackSize(20000);
    pMainBleServer->start();

    USBSerial.println("End SampleSecureServer");
}

void setup()
{
    USBSerial.begin(115200);
    USBSerial.println("printlnb");
    USBSerial.setDebugOutput(true);
    USBSerial.println("Hello Setup");
    ESP_LOGE(LOG_TAG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>LOGE");


    SampleSecureServer();
}
void loop()
{   
}
