/*
 * Api for thorlabs messages.
 */
#include "../ftdi_lib/ftd2xx.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define HEADER_SIZE 6

enum {
    //Generic messages
    IDENTIFY = 0x0223,
    
    SET_CHANENABLESTATE = 0x0210,
    REQ_CHANENABLESTATE = 0x0211,
    GET_CHANENABLESTATE = 0x0212,
    
    HW_DISCONNECT = 0x0002,
    HW_RESPONSE = 0x0080,
    RICHRESPONSE = 0x0081,
    
    HW_START_UPDATEMSGS = 0x0011,
    HW_STOP_UPDATEMSGS = 0x0012,
    HW_REQ_INFO = 0x0005,
    HW_GET_INFO = 0x0006,
    
    RACK_REQ_BAYUSED = 0x0060,
    RACK_GET_BAYUSED = 0x0061,
    HUB_REQ_BAYUSED = 0x0065,
    HUB_GET_BAYUSED = 0x0066,
    
    MOD_SET_DIGOUTPUTS = 0x0213,
    MOD_REQ_DIGOUTPUTS = 0x0214,
    MOD_GET_DIGOUTPUTS = 0x0215,
    
    //Motor controll messages
    HW_YES_FLASH_PROGRAMMING = 0x0017,
    HW_NO_FLASH_PROGRAMMING = 0x0018,
    
    SET_POSCOUNTER = 0x0410,
    REQ_POSCOUNTER = 0x0411,
    GET_POSCOUNTER = 0x0412,
    
    SET_ENCCOUNTER = 0x0409,
    REQ_ENCCOUNTER = 0x040a,
    GET_ENCCOUNTER = 0x040b,
    
    SET_VELPARAMS = 0x0413,
    REQ_VELPARAMS = 0x0414,
    GET_VELPARAMS = 0x0415,
    
    SET_JOGPARAMS = 0x0416,
    REQ_JOGPARAMS = 0x0417,
    GET_JOGPARAMS = 0x0418,
    
    REQ_ADCINPUTS = 0x042b,
    GET_ADCINPUTS = 0x042c,
    
    SET_POWERPARAMS = 0x0426,
    REQ_POWERPARAMS = 0x0427,
    GET_POWERPARAMS = 0x0428,
    
    SET_GENMOVEPARAMS = 0x043a,
    REQ_GENMOVEPARAMS = 0x043b,
    GET_GENMOVEPARAMS = 0x043c,
    
    SET_MOVERELPARAMS = 0x0445,
    REQ_MOVERELPARAMS = 0x0446,
    GET_MOVERELPARAMS = 0x0447,
    
    SET_MOVEABSPARAMS = 0x0450,
    REQ_MOVEABSPARAMS = 0x0451,
    GET_MOVEABSPARAMS = 0x0452,
    
    SET_HOMEPARAMS = 0x0440,
    REQ_HOMEPARAMS = 0x0441,
    GET_HOMEPARAMS = 0x0442,
    
    SET_LIMSWITCHPARAMS = 0x0423,
    REQ_LIMSWITCHPARAMS = 0x0424,
    GET_LIMSWITCHPARAMS = 0x0425,
    
    MOVE_HOME = 0x0443,
    MOVE_HOMED = 0x0444,
    
    MOVE_RELATIVE = 0x0448,
    MOVE_COMPLETED = 0x0464,
    MOVE_ABSOLUTE = 0x0453,
    MOVE_JOG = 0x046a,
    MOVE_VELOCITY = 0x0457,
    MOVE_STOP = 0x0465,
    MOVE_STOPPED = 0x0466,
    
    SET_BOWINDEX = 0x04f4,
    REQ_BOWINDEX = 0x04f5,
    GET_BOWINDEX = 0x04f6,
    
    SET_DCPIDPARAMS = 0x04a0,
    REQ_DCPIDPARAMS = 0x04a1,
    GET_DCPIDPARAMS = 0x04a2,
    
    SET_AVMODES = 0x04b3,
    REQ_AVMODES = 0x04b4,
    GET_AVMODES = 0x04b5,
    
    SET_POTPARAMS = 0x04b0,
    REQ_POTPARAMS = 0x04b1,
    GET_POTPARAMS = 0x04b2,
    
    SET_BUTTONPARAMS = 0x04b6,
    REQ_BUTTONPARAMS = 0x04b7,
    GET_BUTTONPARAMS = 0x04b8,
    
    SET_EEPROMPARAMS = 0x04b9,
    
    SET_PMDPOSITIONLOOPPARAMS = 0x04d7,
    REQ_PMDPOSITIONLOOPPARAMS = 0x04d8,
    GET_PMDPOSITIONLOOPPARAMS = 0x04d9,
    
    SET_PMDMOTOROUTPUTPARAMS = 0x04da,
    REQ_PMDMOTOROUTPUTPARAMS = 0x04db,
    GET_PMDMOTOROUTPUTPARAMS = 0x04dc,
    
    SET_PMDTRACKSETTLEPARAMS = 0x04e0,
    REQ_PMDTRACKSETTLEPARAMS = 0x04e1,
    GET_PMDTRACKSETTLEPARAMS = 0x04e2,
    
    SET_PMDPROFILEMODEPARAMS = 0x04e3,
    REQ_PMDPROFILEMODEPARAMS = 0x04e4,
    GET_PMDPROFILEMODEPARAMS = 0x04e5,
    
    SET_PMDJOYSTICKPARAMS = 0x04e6,
    REQ_PMDJOYSTICKPARAMS = 0x04e7,
    GET_PMDJOYSTICKPARAMS = 0x04e8,
    
    SET_PMDCURRENTLOOPPARAMS = 0x04d4,
    REQ_PMDCURRENTLOOPPARAMS = 0x04d5,
    GET_PMDCURRENTLOOPPARAMS = 0x04d6,
    
    SET_PMDSETTLEDCURRENTLOOPPARAMS = 0x04e9,
    REQ_PMDSETTLEDCURRENTLOOPPARAMS = 0x04ea,
    GET_PMDSETTLEDCURRENTLOOPPARAMS = 0x04eb,
    
    SET_PMDSTAGEAXISPARAMS = 0x04f0,
    REQ_PMDSTAGEAXISPARAMS = 0x04f1,
    GET_PMDSTAGEAXISPARAMS = 0x04f2,
    
    SET_TSTACTUATORTYPE = 0x04fe,
    GET_STATUSUPDATE =  0x0481,
    REQ_STATUSUPDATE = 0x0480,
    
    GET_DCSTATUSUPDATE = 0x0491,
    REQ_DCSTATUSUPDATE = 0x0490,
    ACK_DCSTATUSUPDATE = 0x0492,
    
    REQ_STATUSBITS = 0x0429,
    GET_STATUSBITS = 0x042a,
    
    SUSPEND_ENDOFMOVEMSGS = 0x046b,
    RESUME_ENDOFMOVEMSGS = 0x046c,
    
    SET_TRIGGER = 0x0500,
    REQ_TRIGGER = 0x0501,
    GET_TRIGGER = 0x0502,
            
};

class Message{
public:
    Message(uint8_t* buffer, unsigned int buffer_size){
        bytes = (uint8_t *) malloc(buffer_size);
        bytes = (uint8_t *) memcpy(bytes, buffer, buffer_size);
        length = buffer_size;
    }
    
    Message(size_t in_size){
        bytes = (uint8_t *) malloc(in_size);
    }
    
    ~Message(){
        free(bytes);
    }
 
    uint8_t* data(){ return bytes; }
    
    unsigned int msize(){ return length; }
    
protected:
    unsigned int length;
    uint8_t *bytes;
};


class MessageHeader: Message{
public:
    /**
     * For incoming messages.
     * @param header_bytes
     */
    MessageHeader(uint8_t *header_bytes): Message(header_bytes, HEADER_SIZE){}
    
    
    /**
     * For sending messages.
     * @param type
     * @param param1
     * @param param2
     * @param dest
     * @param source
     */
    MessageHeader(uint16_t type, uint8_t param1, uint8_t param2 ,uint8_t dest, uint8_t source):Message(HEADER_SIZE){
        *((uint16_t *) &bytes[0]) = type;
        bytes[2] = param1;
        bytes[3] = param2;
        bytes[4] = dest;
        bytes[5] = source;
    }
    
    void Getparams(uint8_t *p1, uint8_t *p2){
        *p1 =(uint8_t) bytes[2];
        *p2 =(uint8_t) bytes[3];
    }
    
    void SetParams(uint8_t p1, uint8_t p2){
        bytes[2] = p1;
        bytes[3] = p2;
    }
    
    uint8_t GetSource(){ return bytes[5]; }
    void SetSource(uint8_t source){ bytes[5] = source; } 

    uint8_t GetDest(){ return bytes[4]; }
    void SetDest(uint8_t dest){
       bytes[4] = dest;
    }
    
    uint16_t GetType(){ return *((uint16_t *) &bytes[0]) ;}
    
};

class LongMessage: Message{
public:
    LongMessage(uint8_t *input_bytes, unsigned int buffer_size):Message(input_bytes, buffer_size){};
    
    LongMessage(uint16_t type, uint16_t size, uint8_t dest, uint8_t source):Message(HEADER_SIZE + size){
        *((uint16_t *) &bytes[0]) = type;
        *((uint16_t *) &bytes[2]) = size;
        bytes[4] = dest;
        bytes[5] = source;
    }
    
    void SetDest(uint8_t dest){ bytes[4] = (dest | 0x80); }
    uint8_t GetDest(){ return bytes[4]; }

    uint8_t GetSource(){ return bytes[5]; }
    void SetSource(uint8_t source){ bytes[5] = source; } 
    
    uint16_t GetPacketLength(){ return *((uint16_t *) &bytes[2]);}
    void SetPacketLength(uint16_t size){ *((uint16_t *) &bytes[2]) = size;}
    
};





int SendMessage(Message message, FT_HANDLE &handle){
    FT_STATUS wrStatus;
    unsigned int wrote;
    wrStatus = FT_Write(handle, message.data(), message.msize(), &wrote );
    if (wrStatus == FT_OK && wrote == message.msize()){
        return 0;
    }
    else {
        printf("Sending message failed, error code : %d \n", wrStatus );
        printf("wrote : %d should write: %d \n", wrote, message.msize());
    }
    return -1;
}

