/*
 * Programa para emular un control infrarrojo en un microcontrolador con comandos introducidos por Serial.
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
#define IR_SEND_PIN 3 //Connect IR sender led at this pin
//#define IR_RECEIVE_PIN (your_recv_pin) //Connect IR receiver phototransistor at this pin
//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000


uint16_t sAddress = 0; //In my control address = 0
uint8_t sCommand = 0x40;
uint8_t sRepeats = 1;

/*
 * Send NEC IR protocol
 */
void send_ir_data() {
    Serial.println("Sending data:");
    Serial.printf("Address: 0x%x, Cmd: 0x%x, Repeats: %d", sAddress, sCommand, sRepeats);
    Serial.println("");
    Serial.flush(); // To avoid disturbing the software PWM generation by serial output interrupts

    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }
    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendNEC(sAddress, sCommand, sRepeats); //Change sendNEC() by sendProtocol() for using another protocol
}


void setup() {
    Serial.begin(115200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my microcontroller
    Serial.println(F("Using library version " VERSION_IRREMOTE));
    
    /*
     * No IR send setup required :-)
     * Default is to use IR_SEND_PIN and use feedback LED at default feedback LED pin
     * if not disabled by #define NO_LED_SEND_FEEDBACK_CODE
     */
    Serial.println("Send IR signals at pin " +String(IR_SEND_PIN));

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
    
    Serial.println("Write in Serial a hex command:");
}


void loop() {
    if(Serial.available()) {
        sscanf(Serial.readStringUntil('\n').c_str(), "%hhX", &sCommand); //Read string as hex format
        send_ir_data();
        //IrReceiver.restartAfterSend(); // Is a NOP if sending does not require a timer.
        
        // wait for the receiver state machine to detect the end of a protocol
        //delay((RECORD_GAP_MICROS / 1000) + 5);
    }
}
