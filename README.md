# esp32_ANCS
Example of ANCS for the ESP32

original code @S-March


after a lot of trying I manage to compile for ESP32 C3, but it presents an error when sending to the board, but at least it compiles without errors

and what helped me?
oficial documentation Apple ANCS
https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Appendix/Appendix.html#//apple_ref/doc/uid/TP40013460-CH3-SW1
 https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html#//apple_ref/doc/uid/TP40013460-CH1-SW7


@ParcoMolo copy #erro class MySecurity : public BLESecurityCallbacks in:
https://github.com/nkolban/esp32-snippets/issues/793#issuecomment-1189008694

intructions for @chegewara replace path > /libraries/BLE/src in:
https://github.com/espressif/esp-idf/issues/3230#issuecomment-479256634

instructions for @squonk11 #erro ringbuf_type_t  issue by replacing it with RingbufferType_t. in:
https://github.com/nkolban/esp32-snippets/issues/933#issuecomment-569625223

instructions for @vijitSingh97 #erro esp_ble_gap_update_whitelist in:
https://github.com/nkolban/esp32-snippets/issues/472#issuecomment-501418988

I uploaded the modifications made to my fork

https://github.com/Eonassis/esp32_ANCS



as i did:
 
1 - Download the files from this repository

2 - Delete the src folder from the path, in my case it was: C:\Users\userpc\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.7\libraries\BLE

3 - Copy the src folder from the repository to: C:\Users\userpc\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.7\libraries\BLE

3 - Delete the ESP32_BLE_Arduino folder from the path, in my case it was: C:\Users\userpc\Documents\Arduino\libraries


My arduino IDE is: 2.0.4

My esp32 libary is: 2.0.7

