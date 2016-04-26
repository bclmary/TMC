#ifndef API
#define API

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "messages.hpp"

#define MAX_RESPONSE_SIZE 128
#define SOURCE 0x01

#define FT_ERROR -6
#define DEVICE_ERROR -7
#define FATAL_ERROR -8

#define MOVED_HOME_STATUS 3
#define MOVE_COMPLETED_STATUS 2
#define MOVE_STOPPED_STATUS 4

#define OTHER_MESSAGE 5
#define EMPTY 1

#define INVALID_DEST -10
#define INVALID_SOURCE -11
#define INVALID_CHANNEL -12

#define INVALID_PARAM_1 -15
#define INVALID_PARAM_2 -16
#define INVALID_PARAM_3 -17
#define INVALID_PARAM_4 -18
#define INVALID_PARAM_5 -19

#define READ_REST(x)  unsigned int bytes_red; ftStatus = FT_Read(opened_device.handle, &buff[2], x, &bytes_red); \
        if (ftStatus != FT_OK) {                                    \
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );    \
        return FT_ERROR;                                            \
        }  

#define EMPTY_IN_QUEUE ret = EmptyIncomingQueue();    \
        if (ret != 0 ) return ret;

#define CHECK_ADDR_PARAMS(dest, chanID) int ret;        \
        ret = CheckParams(dest,chanID);                  \
         if (ret != 0) return ret;

#define GET_MESS(req_mess_class, buff_size, get_mess_code, get_mess_class  ) \
        CHECK_ADDR_PARAMS(dest, channel)                        \
        EMPTY_IN_QUEUE                                          \
        req_mess_class mes(dest, SOURCE, channel);              \
        SendMessage(mes);                                       \
        uint8_t *buff = (uint8_t *) malloc(buff_size);          \
        ret = GetResponseMess(get_mess_code, buff_size, buff);  \
        get_mess_class mess(buff);                              \
        *message = mess;                                        \
        free(buff);                                             \
        if ( ret != 0) return ret;                              \
        EMPTY_IN_QUEUE                                          

static uint8_t DefaultDest(){
    return 0x50;
}

static uint8_t DefaultChanel8(){
    return 0x01;
}

static uint16_t DefaultChanel16(){
    return 0x01;
}

static uint8_t DeafultRate(){
    return 1;
}

static uint8_t DefaultStopMode(){
    return 0x02;
}


inline int CheckParams( uint8_t dest, int chanID){
    if (chanID > opened_device.channels && chanID != -1) return INVALID_CHANNEL;
    if (dest == 0x11 || dest == 0x50) return 0;
    switch (dest){
        case 0x21: {
            if (opened_device.bays >= 1 && opened_device.bay_used[0]) return 0;
            else return INVALID_DEST;
        }
        case 0x22:{
            if (opened_device.bays >= 2 && opened_device.bay_used[1]) return 0;
            else return INVALID_DEST;
        }
        case 0x23: {
            if (opened_device.bays == 3 && opened_device.bay_used[2]) return 0;
            else return INVALID_DEST;
        }
        default : return INVALID_DEST;
    };
    return 0;
};

inline int SendMessage(Message &message){
    FT_STATUS wrStatus;
    unsigned int wrote;
    wrStatus = FT_Write(opened_device.handle, message.data(), message.msize(), &wrote );
    if (wrStatus == FT_OK && wrote == message.msize()){
        return 0;
    }
    else {
        fprintf(stderr,"Sending message failed, error code : %d \n", wrStatus );
        fprintf(stderr,"wrote : %d should write: %d \n", wrote, message.msize());
    }
    return FT_ERROR;
}

inline int CheckIncomingQueue(uint16_t *ret_msgID){
    FT_STATUS ftStatus;
    unsigned int bytes;
    ftStatus = FT_GetQueueStatus(opened_device.handle, &bytes);
    if (ftStatus != FT_OK ) {
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );
        return FT_ERROR;
    }
    if (bytes == 0 ) return EMPTY;
    uint8_t *buff = (uint8_t *) malloc(MAX_RESPONSE_SIZE);
    unsigned int red;
    ftStatus = FT_Read(opened_device.handle, buff, 2, &red);          
    if (ftStatus != FT_OK) {
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );
        free(buff);
        return FT_ERROR;
    }
    uint16_t msgID = le16toh(*((uint16_t*) &buff[0])); 
    switch ( msgID ){
        case HW_DISCONNECT: {
            READ_REST(4)
            HwDisconnect response(buff);
            printf("Device with serial %s disconnecting\n", opened_device.SN);
            free(buff);
            return FATAL_ERROR;
        }
        case HW_RESPONSE:{
            READ_REST(4)
            HwResponse response(buff);
            fprintf(stderr,"Device with serial %s encountered error\n", opened_device.SN);
            free(buff);
            return DEVICE_ERROR;
        }
        case RICHRESPONSE:{
            READ_REST(72)
            HwResponseInfo response(buff);      
            fprintf(stderr, "Device with serial %s encountered error\n", opened_device.SN);
            fprintf(stderr, "Detailed description of error \n ");
            uint16_t error_cause = response.GetMsgID();
            if (error_cause != 0) printf("\tMessage causing error: %d\n ", error_cause);
            fprintf(stderr, "\tThorlabs error code: %d \n", response.GetCode());
            fprintf(stderr, "\tDescription: %s\n", response.GetDescription());
            free(buff);
            return DEVICE_ERROR;
        }
        case MOVE_HOMED:{
            READ_REST(4)
            MovedHome response(buff);
            opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d moved to home position\n", response.GetMotorID() + 1);
            free(buff);
            return MOVED_HOME_STATUS;
        }
        case MOVE_COMPLETED:{
            READ_REST(18) // 14 bytes for status updates
            MoveCompleted response(buff);
            opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d completed move\n", response.GetMotorID() + 1);
            free(buff);
            return MOVE_COMPLETED_STATUS;
        }
        case MOVE_STOPPED:{
            READ_REST(18) // 14 bytes for status updates
            MoveStopped response(buff);
            opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d stopped \n", response.GetMotorID() +1 );
            free(buff);
            return MOVE_STOPPED_STATUS;
        }
        case GET_STATUSUPDATE:{
            READ_REST(18)
            GetStatusUpdate response(buff);
            opened_device.motor[response.GetMotorID()].status_enc_count = response.GetEncCount();
            opened_device.motor[response.GetMotorID()].status_position = response.GetPosition();
            opened_device.motor[response.GetMotorID()].status_status_bits = response.GetStatusBits();   
            free(buff);
            return 0;
        }
        case GET_DCSTATUSUPDATE:{
            READ_REST(18)
            GetMotChanStatusUpdate response(buff);
            opened_device.motor[response.GetMotorID()].status_velocity = response.GetVelocity();
            opened_device.motor[response.GetMotorID()].status_position = response.GetPosition();
            opened_device.motor[response.GetMotorID()].status_status_bits = response.GetStatusBits();
            free(buff);
            return 0;
        }
        default: {
            *ret_msgID = msgID;
            free(buff);
            return OTHER_MESSAGE;
        } 
    };
}

inline int EmptyIncomingQueue(){
    while(true){
        int ret = CheckIncomingQueue(NULL);
        if (ret == EMPTY) return 0;
        if (ret == MOVED_HOME_STATUS || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
            case OTHER_MESSAGE: {
                fprintf(stderr, "Unknown message received, protocol violated\n");
                return FATAL_ERROR;
            }
        }
    }
}

static int GetResponseMess(uint16_t expected_msg, int size, uint8_t *mess ){
    int ret;
    uint16_t msgID;
    while(true){
        ret = CheckIncomingQueue(&msgID);
        if (ret == OTHER_MESSAGE){
            if (msgID == expected_msg) {
                *((int16_t *) &mess[0]) =  htole16(msgID);
                unsigned int red;
                FT_STATUS read_status = FT_Read(opened_device.handle, &mess[2], size-2, &red);
                if ( read_status != FT_OK ) {
                    fprintf(stderr, "FT_Error occured, error code :%d\n", read_status );
                    return FT_ERROR;
                }
                return 0;
            }
            else return FATAL_ERROR;
        } 
        if (ret == MOVED_HOME_STATUS || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
        }
    }
    return 0;
}

namespace device_calls{
// ------------------------- Generic device calls ------------------------------

inline int Identify(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    IdentifyMs mes(dest, 0x01);
    SendMessage(mes); 
    EMPTY_IN_QUEUE
    return 0;
}

inline int EnableChannel(uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8()){   
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 1, dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

inline int DisableChannel(uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8()){  
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 2, dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

inline int ChannelState(GetChannelState *info, uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8()){ 
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    ReqChannelState mes(chanel, dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( GET_CHANENABLESTATE, HEADER_SIZE , buff);
    GetChannelState mess(buff);
    *info = mess;
    free(buff);
    if ( ret != 0) return ret;  
    EMPTY_IN_QUEUE
    return 0;
}

inline int DisconnectHW(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    HwDisconnect mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

inline int StartUpdateMess(uint8_t rate = DeafultRate(), uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    StartUpdateMessages mes(dest, SOURCE);
    if (mes.SetUpdaterate(rate) == IGNORED_PARAM) printf("This parameter is ignored in connected device. Using default.\n");
    SendMessage(mes);
    opened_device.status_updates = true;
    EMPTY_IN_QUEUE
    return 0;
}

inline int StopUpdateMess(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    StopUpdateMessages mes(dest, SOURCE);
    SendMessage(mes);
    opened_device.status_updates = false;
    EMPTY_IN_QUEUE
    return 0;
}

inline int GetHwInfo(HwInfo *message, uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ReqHwInfo mes(dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(90);
    ret = GetResponseMess( HW_GET_INFO, 90, buff);
    HwInfo info(buff);
    *message = info; 
    free(buff);
    if ( ret != 0) return ret;
    EMPTY_IN_QUEUE
    return 0;
}

inline int GetBayUsed(GetRackBayUsed *message, uint8_t bayID, uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ReqRackBayUsed mes(dest, SOURCE);
    mes.SetBayIdent(bayID);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( RACK_GET_BAYUSED, HEADER_SIZE , buff);
    GetRackBayUsed bayused(buff);
    *message = bayused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

inline int GetHubUsed(GetHubBayUsed *message, uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1);
    EMPTY_IN_QUEUE
    ReqHubBayUsed mes(dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( HUB_GET_BAYUSED, HEADER_SIZE , buff);
    GetHubBayUsed hubused(buff);
    *message = hubused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

//-------------------------- Motor control calls ------------------------------

inline int FlashProgYes(uint8_t dest = DefaultDest() ){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    YesFlashProg mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
};

inline int FlashProgNo(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    NoFlashProg mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
};

inline int SetPositionCounter(int32_t pos, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetPosCounter mes(dest, SOURCE, channel);
    mes.SetPosition(pos);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

inline int GetPositionCounter(GetPosCounter *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqPosCounter,12,GET_POSCOUNTER,GetPosCounter)      
    return 0;
};

inline int SetEncoderCounter(int32_t count, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetEncCount mes(dest, SOURCE, channel);
    mes.SetEncoderCount(count);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

inline int GetEncoderCounter(GetEncCount *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqEncCount,12,GET_ENCCOUNTER,GetEncCount)      
    return 0;
};

inline int SetVelocityP(int32_t acc, int32_t maxVel, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetVelocityParams mes(dest, SOURCE, channel);
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMaxVel(maxVel) == INVALID_PARAM) return INVALID_PARAM_2;
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;        
}

inline int GetVelocityP(GetVelocityParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqVelocityParams,20,GET_VELPARAMS,GetVelocityParams)       
    return 0;
};

inline int SetJogP(uint16_t mode, int32_t stepSize, int32_t vel, int32_t acc, uint16_t stopMode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetJogParams mes(dest, SOURCE, channel);
    if (mes.SetJogMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    mes.SetStepSize(stepSize);
    if (mes.SetMaxVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_4;
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_5;
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0; 
};

inline int GetJogP(GetJogParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqJogParams,28,GET_JOGPARAMS,GetJogParams)
    return 0;
};

inline int SetPowerUsed(uint16_t rest_power, uint16_t move_power, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetPowerParams mes(dest, SOURCE, channel);
    if ( mes.SetRestFactor(rest_power) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMoveFactor(move_power) == INVALID_PARAM )return INVALID_PARAM_2;        
    SendMessage(mes);
    EMPTY_IN_QUEUE        
    return 0;
};

inline int GetPowerUsed(GetPowerParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqPowerParams,12,GET_POWERPARAMS,GetPowerParams)
    return 0;
};

inline int SetBacklashDist(uint32_t dist, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetGeneralMoveParams mes(dest, SOURCE, channel);
    mes.SetBacklashDist(dist);    
    SendMessage(mes);
    EMPTY_IN_QUEUE        
    return 0;
};

inline int GetBacklashDist(GetGeneralMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqGeneralMoveParams,12,GET_GENMOVEPARAMS,GetGeneralMoveParams)                        
    return 0;
};

inline int SetRelativeMoveP(uint32_t dist, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetRelativeMoveParams mes(dest, SOURCE, channel);
    mes.SetRelativeDist(dist);    
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;        
};

inline int GetRelativeMoveP(GetRelativeMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqRelativeMoveParams,12,GET_MOVERELPARAMS,GetRelativeMoveParams) 
    return 0;
};

inline int SetAbsoluteMoveP(uint32_t pos, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetAbsoluteMoveParams mes(dest, SOURCE, channel);
    mes.SetAbsolutePos(pos);    
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetAbsoluteMoveP(GetAbsoluteMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqAbsoluteMoveParams,12,GET_MOVEABSPARAMS,GetAbsoluteMoveParams) 
    return 0;
};

inline int SetHomingVel(uint32_t vel, int8_t dest = DefaultDest(),  uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetHomeParams mes(dest, SOURCE, channel);
    if (mes.SetHomingVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetHomingVel(GetHomeParams *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqHomeParams,20,GET_HOMEPARAMS,GetHomeParams) 
    return 0;
};

inline int SetLimitSwitchConfig(uint16_t CwHwLim, uint16_t CCwHwLim, uint16_t CwSwLim, uint16_t CCwSwLim, uint16_t mode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetLimitSwitchParams mes(dest, SOURCE, channel);
    if (mes.SetClockwiseHardLimit(CwHwLim) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetCounterlockwiseHardLimit(CCwHwLim) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetClockwiseSoftLimit(CwSwLim) == IGNORED_PARAM) printf("Software limit ignored in this device");
    if (mes.SetCounterlockwiseSoftLimit(CCwSwLim)== IGNORED_PARAM) printf("Software limit ignored in this device");
    ret = mes.SetLimitMode(mode);
    if (ret == INVALID_PARAM) return INVALID_PARAM_5;
    if (ret == IGNORED_PARAM) printf("Limit mode ignored in this device");
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetLimitSwitchConfig(GetLimitSwitchParams *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqLimitSwitchParams,22,GET_LIMSWITCHPARAMS,GetLimitSwitchParams) 
    return 0;
};

inline int MoveToHome(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveHome mes(dest, SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].homing=true;  
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartSetRelativeMove(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative1 mes(dest,SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartRelativeMove(int32_t dist, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative2 mes(dest,SOURCE,channel);
    mes.SetRelativeDistance(dist);
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartSetAbsoluteMove(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveAbsolute1 mes(dest,SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartAbsoluteMove(int32_t pos, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE       
    MoveAbsolute2 mes(dest,SOURCE,channel);  
    if (mes.SetAbsoluteDistance(pos) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartJogMove(uint8_t direction, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE       
    JogMove mes(dest,SOURCE,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StartSetVelocityMove( uint8_t direction, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE        
    MovewVelocity mes(dest,SOURCE,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int StopMovement(uint8_t stopMode = DefaultStopMode(), uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    StopMove mes(dest,SOURCE,channel);  
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].stopping=true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int SetAccelerationProfile(uint16_t index, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetBowIndex mes(dest, SOURCE, channel);
    if (mes.SetBowindex(index) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetAccelerationProfile(GetBowIndex *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqBowIndex,10,GET_BOWINDEX,GetBowIndex) 
    return 0;
};

inline int SetLedP(uint16_t mode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetLedMode mes(dest, SOURCE, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};


inline int GetLedP(GetLedMode *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqLedMode,10,GET_AVMODES,GetLedMode) 
    return 0;
};

inline int SetButtons(uint16_t mode, int32_t pos1, int32_t pos2, uint16_t timeout, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetButtonParams mes(dest, SOURCE, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetPosition1(pos1) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetPosition2(pos2) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetTimeout(timeout) == IGNORED_PARAM ) printf("Timeout ignored in this device");
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetButtonsInfo(GetButtonParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqButtonParams,22,GET_BUTTONPARAMS,GetButtonParams) 
    return 0;
};

// only requests for data, automatically stored in device info
inline int ReqStatus(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)                
    EMPTY_IN_QUEUE                                          
    ReqStatusUpdate mes(dest, SOURCE, channel);              
    SendMessage(mes);                                       
    EMPTY_IN_QUEUE  
    return 0;
};

// only requests for data, automatically s
inline int ReqDcStatus(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)                
    EMPTY_IN_QUEUE                                          
    ReqMotChanStatusUpdate mes(dest, SOURCE, channel);              
    SendMessage(mes);                                       
    EMPTY_IN_QUEUE  
    return 0;
};

inline int SendServerAlive(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ServerAlive mes(dest,SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetStatBits(GetStatusBits *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqStatusBits,12,GET_STATUSBITS,GetStatusBits) 
    return 0;
};

inline int DisableEomMessages(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    DisableEndMoveMessages mes(dest,SOURCE);
    SendMessage(mes);
    opened_device.end_of_move_messages = false;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int EnableEomMessages(uint8_t dest = DefaultDest()){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    EnableEndMoveMessages mes(dest,SOURCE);
    SendMessage(mes);
    opened_device.end_of_move_messages = true;
    EMPTY_IN_QUEUE 
    return 0;
};

inline int CreateTrigger(uint8_t mode, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetTrigger mes(dest,SOURCE, channel);
    if (mes.SetMode(mode) == IGNORED_PARAM) printf("trigger ignored in this device");;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

inline int GetMotorTrigger(GetTrigger *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqTrigger,HEADER_SIZE,GET_TRIGGER,GetTrigger) 
    return 0;
};


} // namespace device_calls

inline int OpenDevice(unsigned int index){
    if (index >= devices_connected) return INVALID_PARAM_1;
    device_calls::StopUpdateMess();
    FT_Close(opened_device.handle);
    opened_device = connected_device[index];
    FT_HANDLE handle;
    FT_STATUS ft_status;
    ft_status = FT_OpenEx( opened_device.SN, FT_OPEN_BY_SERIAL_NUMBER, &handle);
    if (ft_status != FT_OK ) { fprintf(stderr, "Error opening device: %d\n", ft_status); return FT_ERROR; }
    opened_device.handle = handle;
    opened_device_index = index;
    device_calls::StartUpdateMess();
    return 0;
};

#endif 