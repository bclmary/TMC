#ifndef CMD_HPP
#define	CMD_HPP
#include "api_calls.hpp"
#include "device.hpp"
#include <map>
#include <vector>
#include <string.h>
#include <iostream>
#include <string>


typedef int (*helper)(std::vector<string>);
typedef std::map<std::string,helper> call_map;

int HelpC(std::vector<string> args){
    if (args.size() > 0) printf("No arguments needed\n");
    printf(" help        prints this help message, for command info use -h\n");
    printf(" open        switch between controlled devices \n");
    printf(" devinfo     prints connected device information \n");
    printf(" flash       flashes front panel LED \n");
    printf(" chan        channel state \n");
    printf(" poscount    device's position counter \n");
    printf(" enccount    device's encoder counter \n");
    printf(" velp        acceleration and maximum velocity \n");
    printf(" jogp        jog move parameters \n");
    printf(" powerp      power used while motor moves or rests \n");
    printf(" bdist       backlash distance value \n");
    printf(" relmovep    relative move parameters \n");
    printf(" absmovep    absolute move parameters \n");
    printf(" limitsw     limit switch parameters \n");
    printf(" homingvel   homing velocity \n");
    printf(" home        move to home position \n");
    printf(" relmove     start relative move \n");
    printf(" absmove     start absolute move \n");
    printf(" jogmove     start jog move \n");
    printf(" velmove     start move with set velocity \n");
    printf(" stop        stop movement \n");
    printf(" ledp        front LED parameters \n");
    printf(" buttp       device's buttons parameters \n");
    printf(" status      get status \n");
    printf(" statusdc    get status for dc servo controller \n");
    printf(" eom         trigger parameters \n");
    return 0;
}

int OpenDeviceC(std::vector<string> args){
    if (args.size() == 1 ) printf("No arguments\n");
    else{
        if (args.at(1) == "-h"){ 
            printf("Choose which connected device to control\n");
            printf("-n=NUMBER       device number in list created by program, see devinfo\n");
            printf("-s=SN           serial number of device\n");
            return 0;
        }
        if ( args.at(1).substr(0,2).compare("-n=") ) {
            unsigned int num = std::stoi(args.at(1).substr(3,args.at(1).size()));
            if (OpenDevice(num) != 0 ) printf("Incorrect device number\n");
            return 0;
        }
        if ( args.at(1).substr(0,3).compare("-s=") ) {
            for ( unsigned int i = 0; i< devices_connected; i++ ){
                std::string to_comp (connected_device[i].SN);
                if ( to_comp.compare(args.at(1).substr(3,args.at(1).size())) ) OpenDevice(i);
            }
            printf("Device with specified serial number not present\n");
            return 0;
        }
    }
    printf("Unrecognized parameter %s, see -h for help\n", args.at(1).c_str());
    return 0;
}

int DeviceInfoC(std::vector<string> args){
    // saved info, ask for bay used, ask for hw info, get hub used
    //not implemented
    return 0;
}

int IdentC(std::vector<string> args){
    //not implemented
    return 0;
}

int ChannelAbleC(std::vector<string> args){
    // enable, disable, info 
    //not implemented
    return 0;
}

int PosCounterC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int EncCountC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int VelParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int JogParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int PowerParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int BacklashDistC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int RelMoveParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int AbsMoveParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int HomingVelC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int LimitSwitchC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int HomeC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartRelMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartAbsMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartJogMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartVelMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StopC(std::vector<string> args){
    //not implemented
    return 0;
}

int AccParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int LedParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int ButtonsParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int StatusC(std::vector<string> args){
    //GetStatBits, GetStatus
    //not implemented
    return 0;
}

int DCStatusC(std::vector<string> args){
    //not implemented
    return 0;
}

int EOMC(std::vector<string> args){
    //enable, disable
    //not implemented
    return 0;
}

int TriggerParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

call_map calls = {
    std::make_pair("help", &HelpC),
    std::make_pair("flash", &IdentC),
    std::make_pair("chan", &ChannelAbleC),
    std::make_pair("open", &OpenDeviceC),
    std::make_pair("devinfo", &DeviceInfoC),
    std::make_pair("poscount", &PosCounterC),
    std::make_pair("enccount", &EncCountC),
    std::make_pair("velp", &VelParamC),
    std::make_pair("jogp", &JogParamC),
    std::make_pair("powerp", &PowerParamC),
    std::make_pair("bdist", &BacklashDistC),
    std::make_pair("relmovep", &RelMoveParamC),
    std::make_pair("absmovep", &AbsMoveParamC),
    std::make_pair("homingvel", &HomingVelC),
    std::make_pair("limitsw", &LimitSwitchC),
    std::make_pair("home", &HomeC),
    std::make_pair("relmove", &StartRelMoveC),
    std::make_pair("absmove", &StartAbsMoveC),
    std::make_pair("jogmove", &StartJogMoveC),
    std::make_pair("velmove", &StartVelMoveC),
    std::make_pair("stop", &StopC),
    std::make_pair("accp", &AccParamC),
    std::make_pair("ledp", &LedParamC),
    std::make_pair("buttp", &ButtonsParamC),
    std::make_pair("status", &StatusC),
    std::make_pair("statusdc", &DCStatusC),
    std::make_pair("eom", &EOMC),
    std::make_pair("triggp", &TriggerParamC)
};


int run_cmd(){
   
    while(true){
        std::string line = "";
        std::getline(std::cin, line);
        if (line == "" ) continue;
        
        std::vector<std::string> args;
        const char delimiter = ' ';
        char* token = strtok(strdup(line.c_str()), &delimiter);
        while(token != NULL){
            args.push_back(std::string(token));
            token = strtok(NULL, &delimiter);
        }
        
        if ( args.at(0).compare("exit") == 0 ) break;
        if ( calls.count(args.at(0))== 0 ) {
            printf("Unrecognized command %s\n", args.at(0).c_str() );
            continue;
        }
        
        int ret_val = calls.at(args.at(0))(args);
        if ( ret_val != 0 ) return ret_val;
    }
    
    return 0;
}

#endif	/* CMD_HPP */

