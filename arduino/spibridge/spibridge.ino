/*
    Arduino SPI Bridge

    Allows SPI transfers via serial.
    Also allows control over various GPIO.

	Copyright (C) 2015 Mark Jessop <vk5qi@rfhead.net>
    
*/

#include <util/crc16.h>
#include <SPI.h>
#include <Wire.h>

// Version Information
#define SOFTWARE_MAJOR      1
#define SOFTWARE_MINOR      0
#define SOFTWARE_REVISION   0

#define IDENTIFIER_LENGTH   10
char identifier[] = "SPIBridge ";


// Packet Info
#define SYNC_CHAR_1         0xAB
#define SYNC_CHAR_2         0xCD

#define OPCODE_VERSION      0x00
#define OPCODE_SPI_TXFR     0x01
#define OPCODE_LED          0x02
#define OPCODE_READ_GPIO    0x03

// States
#define NO_INPUT            0
#define READ_SYNC           1
#define READ_OPCODE         2
#define READ_LENGTH         3
#define READ_LENGTH_2       4
#define READ_PAYLOAD        5
#define READ_CRC            6

// Hardware Pins
#define LED_PIN             13 // Blinkenlights

// Seeduino IO pins
#define SS_PIN              10
#define DIO0                A0
#define DIO5                A2
#define RST                 A1

// Seeduino Mega (3.3V)
/*
#define SS_PIN  53
#define DIO0  A0
#define DIO5  A2
#define RST   A1
*/

// Input Buffers
uint8_t state = NO_INPUT;
uint8_t opcode = 255;
//uint8_t payload_length = 0;
#define PAYLOAD_BUFFER_LENGTH   1024
uint8_t payload_buffer [PAYLOAD_BUFFER_LENGTH];
unsigned int payload_buf_ptr = 0;
uint8_t checksum_valid = 0;
uint8_t large_packet = 0;

union crc16_buf{
    uint8_t checksum_buffer[2];
    unsigned int checksum_int;
}   crc16_buffer;

union payload_len_buf{
    uint8_t payload_len_buffer[2];
    unsigned int payload_len_int;
}   payload_length;

void setup(){
    // Set up LED Pin IO
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Set up DIO0 & 5 IO
    pinMode(DIO0,INPUT);
    pinMode(DIO5,INPUT);

    // Set up SPI IO
    pinMode(SS_PIN, OUTPUT);
    digitalWrite(SS_PIN, HIGH);

    // 'Set up' the serial. This actually does nothing for USB CDC - it's on all the time.
    Serial.begin(57600);

    // Start up the SPI bus
    SPI.begin();

    delay(300);
    digitalWrite(LED_PIN, HIGH); // Set the LED on until serial comms starts.
    while(!Serial) {
        ;// Wait for the user to open the serial port.
    }
    digitalWrite(LED_PIN, LOW);
}


void loop(){
    // Pass all serial data onto the state machine.
    if(Serial.available()>0) parse_packet();
}

// Packet parsing state machine
void parse_packet(){
    uint8_t data = Serial.read();

    switch(state){
        case NO_INPUT:
            if(data == SYNC_CHAR_1) state = READ_SYNC;
            break;

        case READ_SYNC:
            if(data == SYNC_CHAR_2){
                state = READ_OPCODE;
            }else{
                clear_state();
            }
            break;

        case READ_OPCODE:
            opcode = data;
            state = READ_LENGTH;
            break;

        case READ_LENGTH:
            payload_length.payload_len_buffer[1] = data;
            state = READ_LENGTH_2;
            break;

        case READ_LENGTH_2:
            payload_length.payload_len_buffer[0] = data;

            // If the payload length is >1024, we can assume that the host is about to flood us with
            // more data than we can handle.
            // Setting the payload length to 1024 will cause the CRC to fail, and put us back
            // into the 'NO INPUT' state. 
            if(payload_length.payload_len_int > 1024){
                payload_length.payload_len_int = 1024;
            }

            state = READ_PAYLOAD;
            break;

        case READ_PAYLOAD:
            if(payload_buf_ptr < payload_length.payload_len_int){
                payload_buffer[payload_buf_ptr++] = data;
            }else{
                crc16_buffer.checksum_buffer[1] = data;
                state = READ_CRC;
            }
            break;

        case READ_CRC:
            crc16_buffer.checksum_buffer[0] = data;
            check_crc16();
            if(checksum_valid){ 
                process_packet();
            }
            clear_state();
            break;

        default:
            break;
    }
    
}

void process_packet(){
    switch(opcode){
        case OPCODE_VERSION:
            // Return version info
            print_version();
            break;

        case OPCODE_LED:
            // Set LED based on value
            set_led();
            break;

        case OPCODE_READ_GPIO:
            // Read DIO0 and DIO5 state.
            read_gpio();
            break;

        case OPCODE_SPI_TXFR:
            spi_transfer();
            break;

        default:
            break;
    }
}

void clear_state(){
    state = NO_INPUT;
    opcode = 255;
    payload_length.payload_len_int = 0;
    payload_buf_ptr = 0;
    checksum_valid = 0;
}

// CRC16 checking function, for received packets.
uint8_t check_crc16(){
    unsigned int i;
    unsigned int crc;
    crc = 0xFFFF; // Standard CCITT seed for CRC16.

    crc = _crc_xmodem_update(crc,opcode);
    crc = _crc_xmodem_update(crc,(uint8_t)((payload_buf_ptr>>8)&0xFF));
    crc = _crc_xmodem_update(crc,(uint8_t)(payload_buf_ptr&0xFF));
   // Serial.println(payload_length.payload_len_buffer[0],HEX);
 //   Serial.println(payload_buf_ptr,DEC);
    for (i = 0; i < payload_buf_ptr; i++) {
        crc = _crc_xmodem_update(crc,payload_buffer[i]);
    }
    //Serial.print("Calculated CRC: ");
    //Serial.println(crc, HEX);
    //Serial.print("RX CRC:");
    //Serial.println(crc16_buffer.checksum_int, HEX);
    if(crc == crc16_buffer.checksum_int){
        checksum_valid = 1;
    }else{
        checksum_valid = 0;
    }
    return checksum_valid;
}

// CRC generation function, for packets to be transmitted.
void calculate_crc16(){
    unsigned int i;
    unsigned int crc;
    crc = 0xFFFF; // Standard CCITT seed for CRC16.
    // Calculate the sum
    crc = _crc_xmodem_update(crc,opcode);
    crc = _crc_xmodem_update(crc,payload_length.payload_len_buffer[1]);
    crc = _crc_xmodem_update(crc,payload_length.payload_len_buffer[0]);
    for (i = 0; i < payload_length.payload_len_int; i++) {
        crc = _crc_xmodem_update(crc,payload_buffer[i]);
    }
    crc16_buffer.checksum_int = crc;
}

void transmit_packet(){
    calculate_crc16();
    Serial.write(SYNC_CHAR_1);
    Serial.write(SYNC_CHAR_2);
    Serial.write(opcode);
    Serial.write(payload_length.payload_len_buffer[1]);
    Serial.write(payload_length.payload_len_buffer[0]);
    if(payload_length.payload_len_int>0){
        Serial.write(payload_buffer, payload_length.payload_len_int);
    }
    Serial.write(crc16_buffer.checksum_buffer[1]);
    Serial.write(crc16_buffer.checksum_buffer[0]);

    // Does this even do anything useful? Probably depends on CDC serial implementation on host.
    Serial.flush();
}

void short_blink(){
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(100);
}
