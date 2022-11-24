# ME310G1 / Charlie official communication library for Arduino

Code library for Arduino projects using ME310G1 devices (for example, on [Charlie Board](https://github.com/telit/arduino-charlie) )



## Contents

This Library will provide a set of APIs wrapping the AT commands to simplify the interaction with the ME310G1 device.

### Classes

The library provides the following classes:

 - **ME310** :  _main class providing all the functionalities, the only needed for users' code_
 - **Parser** : _internal helper classes to simplify the AT command responses parsing_
 - **PathParsing** : _internal helper class to parse the file paths on the device_
 - **ATCommandDataParsing** : _internal class used to call Parser_


### Examples

The following examples are available:

 - **[AGNSS_example](examples/AGNSS_example/AGNSS_example.ino)** : _Simple example that shows how to configure the AGNSS functionality_
 - **[Bosch_sensor_example](examples/Bosch_sensor_example/Bosch_sensor_example.ino)** : _Shows how to retrieve info from the onboard Bosch BMA400 accelerometer_
 - **[CheckModule](examples/CheckModule/CheckModule.ino)** : _Turns the modem on and checks if it responsive_
 - **[CLIP_example](examples/CLIP_example/CLIP_example.ino)** : _Shows how to use Calling Line Identifier protocol_
 - **[FTP_example](examples/FTP_example/FTP_example.ino)** : _Connects to an FTP server and performs basic operations_
 - **[GNSS_example](examples/GNSS_example/GNSS_example.ino)** : _Simple GNSS example, it enables the GNSS receiver and provides raw location data_
 - **[Hello_World_example](examples/Hello_World_example/Hello_World_example.ino)** : _Basic Hello World example_
 - **[LWM2M_example](examples/LWM2M_example/)** : _uses LwM2M protocol and the accelerometer to send data to the OneEdge portal_
   - **[LWM2M_example_2G](examples/LWM2M_example/LWM2M_example_2G/LWM2M_example_2G.ino)** : _example with 2G network_
   - **[LWM2M_example_4G](examples/LWM2M_example/LWM2M_example_4G/LWM2M_example_4G.ino)** : _example with 4G network_
 - **[LWM2M_first_example](examples/LWM2M_first_example/)** : _uses LwM2M to create a simple connection to OneEdge portal and pushes accelerometer data showing basic resources operations_
 - **[LWM2M_Get_Object_example](examples/LWM2M_Get_Object_example/)** : _uses LwM2M GET OBJ functionality to access a whole object with single calls_
 - **[M2M_example](examples/M2M_example/M2M_example.ino)** : _Communicates with the modem using the M2M commands to manage the filesystem_
 - **[ME310_AT_Test](examples/ME310_AT_Test/ME310_AT_Test.ino)** : _Communicates with the modem and provides info about it (ICCID, IMEI etc.)_
 - **[MQTT_example](examples/MQTT_example/MQTT_example.ino)** : _Communicate with a MQTT broker_
 - **[Ping_example](examples/Ping_example/Ping_example.ino)** : _Simple example that enables the connectivity and pings a server_
 - **[Socket_example](examples/Socket_example/Socket_example.ino)** : _Enables connectivity and uses a TCP socket example communicating with a demo server_
 - **[Sleep_mode_example](examples/Sleep_mode_example/Sleep_mode_example.ino)** : _Shows how to configure sleep modes on the board_
 - **[TransparentBridge](examples/TransparentBridge/TransparentBridge.ino)** : _Enable the modem and creates a bridge between Arduino serial and modem AT interface, allowing user to send AT commands manually_
 - **[GenericCommand_example](examples/GenericCommand_example/GenericCommand_example.ino)** : _Shows how to send AT commands to the modem manually and manage the response (useful for complex AT commands or chains of commands)_


## Support

If you need support, please open a ticket to our technical support by sending an email to:

 - ts-americas@telit.com if you are in the Americas region
 - ts-emea@telit.com if you are in EMEA region
 - ts-apac@telit.com if you are in APAC

 providing the following information:

 - module type
 - answer to the commands (you can use the TrasparentBridge example to communicate with the modem)
   - AT#SWPKGV
   - AT+CPIN?
   - AT+CCID
   - AT+CGSN
   - AT+CGDCONT?

and add [Charlie][AppZone] in the e-mail object, and in the e-mail body refer to the opened issue on github.
