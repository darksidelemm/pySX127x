/*
    Arduino SPI Bridge - Packet Handlers

    Copyright (C) 2015 Mark Jessop <vk5qi@rfhead.net>
    
*/
#include <SPI.h>
#include <compat/deprecated.h>


void print_version(){
    opcode = OPCODE_VERSION;
    payload_buffer[0] = SOFTWARE_MAJOR;
    payload_buffer[1] = SOFTWARE_MINOR;
    payload_buffer[2] = SOFTWARE_REVISION;

    for(int i = 0; i<IDENTIFIER_LENGTH; i++){
        payload_buffer[3+i] = identifier[i];
    }

    payload_length.payload_len_int = 3 + IDENTIFIER_LENGTH;

    transmit_packet();
}

void set_led(){
    if(payload_buffer[0]==0x01){
        digitalWrite(LED_PIN, HIGH);
    }else{
        digitalWrite(LED_PIN, LOW);
    }

    payload_length.payload_len_int = 1;
    transmit_packet();
}

void spi_transfer(){
    // Transfer out the bytes in the payload buffer, replacing them with bytes read from the SPI Device.
    // It will be up to the host to include enough 0x00 bytes to read in the full ASIC register.
    digitalWrite(SS_PIN,LOW);
    for(unsigned int i = 0; i<payload_buf_ptr; i++){
        uint8_t temp = payload_buffer[i];
        payload_buffer[i] = SPI.transfer(temp);
    }
    digitalWrite(SS_PIN,HIGH);
    payload_length.payload_len_int = payload_buf_ptr;
    opcode = OPCODE_SPI_TXFR;

    transmit_packet();
}

void read_gpio(){

    payload_buffer[0] = digitalRead(DIO0);
    payload_buffer[1] = digitalRead(DIO5);

    payload_length.payload_len_int = 2;
    opcode = OPCODE_READ_GPIO;

    transmit_packet();

}
