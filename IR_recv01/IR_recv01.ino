/*
 * Un programa que enciende y apaga un led con un microcontrolador y un receptor infrarrojo. 
 */

// select only NEC and the universal decoder for pulse distance protocols
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_DISTANCE_WIDTH // In case NEC is not received correctly. Universal decoder for pulse distance width protocols

//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
#define NO_LED_FEEDBACK_CODE      // saves 318 bytes program memory
//#define NO_LED_RECEIVE_FEEDBACK_CODE  // Saves 44 bytes program memory
//#define NO_LED_SEND_FEEDBACK_CODE     // Saves 36 bytes program memory
//#define FEEDBACK_LED_IS_ACTIVE_LOW // The LED on board (D8) is active LOW (inverted logic)
//#define USE_16_BIT_TIMING_BUFFER  // Use a 16-bit buffer to preserve values above 12750 us
//#define IR_SEND_PIN (your_send_pin) //Connect IR sender led at this pin
#define IR_RECEIVE_PIN 3 //Connect IR receiver phototransistor at this pin
//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000

#define LED 8 //Led for testing
#define LED_INVERTED //If defined, led uses inverted logic. Disable if led is not inverted

#define SDA_OLED 5
#define SCL_OLED 6

#define TRIGGER_CMD 0x40 //I used the command 0x40 because this is the on-off button of muy control

bool led_on = false;

#if defined(SDA_OLED) && defined(SCL_OLED)
#include <U8g2lib.h>

//OLed object, there is no 72x40 constructor in u8g2 hence the 72x40 screen is
// mapped in the middle of the 132x64 pixel buffer of the SSD1306 controller
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_OLED, SDA_OLED);

const int width = 70;
const int height = 40;
const int xOffset = 30; // = (132-w)/2
const int yOffset = 24; // = (64-h)/2
#endif


void receive_ir_data() {
    if (IrReceiver.decode()) {
        //Print protocol in Serial
        Serial.print("Decoded protocol: ");
        Serial.println(getProtocolString(IrReceiver.decodedIRData.protocol));

        //Print raw data in Serial
        Serial.print("Decoded raw data: ");
#if (__INT_WIDTH__ < 32)
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
#else
        PrintULL::print(&Serial, IrReceiver.decodedIRData.decodedRawData, HEX);
        Serial.println("");
#endif
        //Print address
        Serial.print("Decoded address: ");
        Serial.println(IrReceiver.decodedIRData.address, HEX);

        //Print command
        Serial.print("Decoded command: ");
        Serial.println(IrReceiver.decodedIRData.command, HEX);

        //If present, print data in an Oled screen
        #if defined(SDA_OLED) && defined(SCL_OLED)
        u8g2.clearBuffer();
        u8g2.setCursor(xOffset+1, yOffset+u8g2.getMaxCharHeight());
        u8g2.print(String("Prot: ") + getProtocolString(IrReceiver.decodedIRData.protocol));
        u8g2.setCursor(xOffset+1, yOffset+ 2*u8g2.getMaxCharHeight());
        u8g2.printf("Addr: 0x%x", IrReceiver.decodedIRData.address);
        u8g2.setCursor(xOffset+1, yOffset+ 3*u8g2.getMaxCharHeight());
        u8g2.printf("Cmd: 0x%x", IrReceiver.decodedIRData.command);
        u8g2.sendBuffer();
        #endif

        //Turn led if command matches the TRIGGER_CMD
        if(IrReceiver.decodedIRData.command == TRIGGER_CMD) turn_led();
        IrReceiver.resume();
    }
}


void turn_led() {
    if(!led_on) { //Turn on led
        #ifndef LED_INVERTED
        digitalWrite(LED, 1);
        #else
        digitalWrite(LED, 0);
        #endif
        led_on = true;
    } else { //Turn off led
        #ifndef LED_INVERTED
        digitalWrite(LED, 0);
        #else
        digitalWrite(LED, 1);
        #endif
        led_on = false;
    }
}


void setup() {
    Serial.begin(115200);
    
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my microcontroller
    Serial.println(F("Using library version " VERSION_IRREMOTE));
    
    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN);
    
    Serial.print("Ready to receive IR signals of protocols: ");
    printActiveIRProtocols(&Serial);
    Serial.println("at pin " +String(IR_RECEIVE_PIN));
    /*
     * No IR send setup required :-)
     * Default is to use IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin
     * and use feedback LED at default feedback LED pin if not disabled by #define NO_LED_SEND_FEEDBACK_CODE
     */
    
#if FLASHEND >= 0x3FFF  // For 16k flash or more, like ATtiny1604
// For esp32 we use PWM generation by ledcWrite() for each pin.
#  if !defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM) && !defined(ESP32)
    /*
     * Print internal software PWM generation info
     */
    IrSender.enableIROut(38); // Call it with 38 kHz to initialize the values printed below
    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#  endif
    
    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(" us is the (minimum) gap, after which the start of a new IR packet is assumed");

#  if defined(USE_THRESHOLD_DECODER)
    Serial.println("Threshold decoding is active and thus MARK_EXCESS_MICROS is set to 0");
#  else
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(" us are subtracted from all marks and added to all spaces for decoding");
#  endif
#endif
    
    pinMode(LED, OUTPUT);
    #ifdef LED_INVERTED
    digitalWrite(LED, 1);
    #endif
    
    #if defined(SDA_OLED) && defined(SCL_OLED)
    u8g2.begin();
    u8g2.setContrast(255); // set contrast to maximum 
    u8g2.setBusClock(400000); //400kHz I2C 
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_6x12_tr);
    Serial.println("OLed iniciada.");
    #endif
}


void loop() {
    // wait for the receiver state machine to detect the end of a protocol
    delay((RECORD_GAP_MICROS / 1000) + 5);
    receive_ir_data();
    
    delay(100); // Loop delay
}
