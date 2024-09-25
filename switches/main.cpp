/*
    Author:     Paul
*/



//#define MEGA_Dfrobot_Can
#if defined (MEGA_Dfrobot_Can)
#define N2k_CAN_INT_PIN 2 // Dfrobot Can-Bus V2.0
#define N2k_SPI_CS_PIN 10 // Dfrobot Can-Bus V2.0
#endif

// use teensy can drivers

//#define USE_N2K_CAN 8

#include <Arduino.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <ActisenseReader.h>

#ifdef _DEBUG 
#define CzDebug
#endif

// Defined constants

#define CzUpdatePeriod65284 2000
#define CzUpdatePeriod65283 500
#define CzUpdatePeriod127501 10000
#define CZoneMessage 0x9927 // 295 + reserved + industry code=marine
#define CzSwitchBank1SerialNum  0x1d // Serial Number switches 1-4 (00260128)
#define CzSwitchBank2SerialNum  0x1b // Serial Number switches 5-8  (00260126)
#define CzDipSwitch     200 //  CZone Dip switch value, CZone address
#define BinaryDeviceInstance 0x00 // Instance of 127501 switch state message
#define SwitchBankInstance 0x00   //Instance of 127502 change switch state message
#define NumberOfSwitches 8   // change to 4 for bit switch bank

// Function prototypes

void HandleNMEA2000Msg(const tN2kMsg& N2kMsg);
void SendN2k(void);
void SetCZoneSwitchStateBroadcast130817(unsigned char);
void SetCZoneSwitchChangeAck65283(unsigned char); //acknowledgement of switch change or current state to MFD, PGN 65283 
void SetCZoneSwitchHeartbeat65284(unsigned char); // Tell the MDF the switch bank is still online
void SetCZoneSendConfigToMFD65290(unsigned char, uint8_t, uint8_t, uint8_t);
void SetChangeSwitchState(uint8_t, bool);
void SetCZoneSwitchState127501(unsigned char);  // N2K compatability
void SetCZoneSwitchChangeRequest127502(unsigned char, uint8_t, bool); // N2K compatability
void ParseCZoneMFDSwitchChangeRequest65280(const tN2kMsg&); // MFD requests change of a switch PGN 65280
void ParseCZoneConfigRequest65290(const tN2kMsg&);
void ParseMDF65284(const tN2kMsg&);
void ParseMDF65288(const tN2kMsg&);
void ParseN2kPGN127502(const tN2kMsg& N2kMsg);
void SetN2kSwitchBankCommand(tN2kMsg& , unsigned char , tN2kBinaryStatus);

typedef struct
{
    unsigned long PGN;
    void (*Handler)(const tN2kMsg& N2kMsg);
} tNMEA2000Handler;

tNMEA2000Handler NMEA2000Handlers[] =
{
    {65280L,ParseCZoneMFDSwitchChangeRequest65280},
    {65290L,ParseCZoneConfigRequest65290},
    {65284L,ParseMDF65284},
    {65288L,ParseMDF65288},
    {127502L,ParseN2kPGN127502},
    {0,0}
};

//  Global variables

uint8_t CzRelayPinMap[] = { 13,14,15,16,17,18,19,20 }; // arduino pins driving relays i.e CzRelayPinMap[0] returns the pin number of Relay 1
tN2kBinaryStatus CzBankStatus;
uint8_t CzSwitchState1 = 0;
uint8_t CzSwitchState2 = 0;
uint8_t CzMfdDisplaySyncState1 = 0;
uint8_t CzMfdDisplaySyncState2 = 0;
bool    CzConfigAuthenticated = false;

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = { 65284L,65283L,65290L,127501L,127502L,130813L,0 };


void setup()
{
#ifdef CzDebug
    Serial.begin(115200);
    delay(500);
    Serial.println("Starting Up CzSwitches");
#endif
    
    // sets the digital relay driver pins as output
    for (uint8_t i = 0; i < NumberOfSwitches; i++)
        pinMode(CzRelayPinMap[i], OUTPUT);

    // initialize intitial switch state

    N2kResetBinaryStatus(CzBankStatus);

    // setup the N2k parameters

    NMEA2000.SetN2kCANSendFrameBufSize(150);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    //Set Product information
    NMEA2000.SetProductInformation("00260001", 0001, "Switch Bank", "1.000 06/04/21", "My Yacht 8 Bit ");
    // Set device information
    NMEA2000.SetDeviceInformation(260001, 140, 30, 717);
   // NMEA2000.SetForwardStream(&Serial);
    NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 169);
    NMEA2000.ExtendTransmitMessages(TransmitMessages);
    NMEA2000.SetMsgHandler(HandleNMEA2000Msg);
    NMEA2000.Open();
    delay(200);
}

void loop()
{
    NMEA2000.ParseMessages();
    SendN2k();
}

// send periodic updates to maintain sync and as a "heatbeat" to the MFD

void SendN2k(void)
{
    static unsigned long CzUpdate65284 = millis();
    static unsigned long CzUpdated65284 = millis();
    static unsigned long CzUpdate127501 = millis();

    if (CzUpdate65284 + CzUpdatePeriod65284 < millis()) {
        CzUpdate65284 = millis();
        SetCZoneSwitchHeartbeat65284(CzSwitchBank1SerialNum);
        if (NumberOfSwitches == 8) SetCZoneSwitchHeartbeat65284(CzSwitchBank2SerialNum);
        SetCZoneSwitchStateBroadcast130817(CzSwitchBank1SerialNum);
        if (NumberOfSwitches == 8) SetCZoneSwitchStateBroadcast130817(CzSwitchBank2SerialNum);
    }
    if (CzUpdated65284 + CzUpdatePeriod65283 < millis()) {
        CzUpdated65284 = millis();
        if (CzConfigAuthenticated) {
            SetCZoneSwitchChangeAck65283(CzSwitchBank1SerialNum);
            if (NumberOfSwitches == 8) SetCZoneSwitchChangeAck65283(CzSwitchBank2SerialNum);
        }
    }
    if (CzUpdate127501 + CzUpdatePeriod127501 < millis()) {
        CzUpdate127501 = millis();
        SetCZoneSwitchState127501(BinaryDeviceInstance);
    }
}

//NMEA 2000 message handler

void HandleNMEA2000Msg(const tN2kMsg& N2kMsg) {
    int iHandler;
    for (iHandler = 0; NMEA2000Handlers[iHandler].PGN != 0 && !(N2kMsg.PGN == NMEA2000Handlers[iHandler].PGN); iHandler++);
    if (NMEA2000Handlers[iHandler].PGN != 0) {
        NMEA2000Handlers[iHandler].Handler(N2kMsg);
    }
}

//********************************************************************
//      PGN65284 CZone Switch Bank Command message. This needs to be
//      sent every 2 seconds for each switch bank. 
// ********************************************************************

void SetCZoneSwitchHeartbeat65284(unsigned char CzSwitchBankSerialNum) {
    
    tN2kMsg N2kMsg;
    N2kMsg.SetPGN(65284L);
    N2kMsg.Priority = 7;
    N2kMsg.Add2ByteUInt(CZoneMessage);
    if (CzConfigAuthenticated) {

        N2kMsg.AddByte(CzSwitchBankSerialNum);
        N2kMsg.AddByte(0x0f); // ?
        if (CzSwitchBankSerialNum == CzSwitchBank1SerialNum)
            N2kMsg.AddByte(CzSwitchState1);
        else N2kMsg.AddByte(CzSwitchState2);
    }
    else {   // if Switch bank is not authenticated, send following to MFD to prompt a 65290 from MFD

        N2kMsg.AddByte(0xFF);
        N2kMsg.Add2ByteUInt(0x0f0f);
    }
    N2kMsg.Add2ByteUInt(0x0000);
    N2kMsg.AddByte(0x00);
    NMEA2000.SendMsg(N2kMsg);
}


void ParseMDF65284(const tN2kMsg& N2kMsg) {

    // more work maybe required here
    int idx = 0;
    if (N2kMsg.PGN != 65284UL || N2kMsg.Get2ByteUInt(idx) != CZoneMessage) return; // is YD Czone  from MFD msg
    if (CzDipSwitch != N2kMsg.GetByte(idx)) return; // if byte2 == CzDipSwitch then its from the MFD
    CzConfigAuthenticated = true;

}

void ParseMDF65288(const tN2kMsg& N2kMsg) {   
    
    // more work maybe required here
        int idx = 0;
    if (N2kMsg.PGN != 65288UL || N2kMsg.Get2ByteUInt(idx) != CZoneMessage) return; // is YD Czone  from MFD msg
    idx = 3;
    if (CzDipSwitch != N2kMsg.GetByte(idx)) return; // if byte3 == CzDipSwitch then its from the MFD

}


// ******************************************************************************
//      65283 MFD Display sync, sent as a resonse to a switch key press
//      Called as a response to a PGN 65280 "Switch Control" from MFD
//      PGN 65280 commands a toggle of the state of the switch inicated by
//      " SwitchToChange ". Also sent every 0.5 sec for each set of four switches
//
// ******************************************************************************

void SetCZoneSwitchChangeAck65283(unsigned char CzSwitchBankSerialNum) {

    tN2kMsg N2kMsg;
    N2kMsg.SetPGN(65283L);
    N2kMsg.Priority = 7;
    N2kMsg.Destination = 0;
    N2kMsg.Add2ByteUInt(CZoneMessage);
    N2kMsg.AddByte(CzSwitchBankSerialNum);
    if (CzSwitchBankSerialNum == CzSwitchBank1SerialNum) N2kMsg.AddByte(CzMfdDisplaySyncState1); // is it switchbank 1 to 4 or 5 to 8
    else  N2kMsg.AddByte(CzMfdDisplaySyncState2);
    N2kMsg.Add2ByteUInt(0x0000);
    N2kMsg.AddByte(0x00);
    N2kMsg.AddByte(0x10);
    NMEA2000.SendMsg(N2kMsg);
}


// ********************************************************************************************
//      PGN65280 sent from controlling MFD to request a change of state of a single switch
//      The command Byte 2 = Bit to change 0x05 = Sw 1, 0x06 = Sw 2, 0x07 = Sw 3, 0x08 = Sw4
//      byte6 == 0xF1 set switch on, or 0xF2 reset the switch off and initiates the sending of the following:
//      PGN12501, PGN12502 for compatability With NMEA2000 switching from the switching device.
//      After the response has been sent, the MFD sends a PNG65280 with Byte 6 equal 0x40 to indicate success 
//      PGN65283 is then sent from the switching device.
//
// ********************************************************************************************

void ParseCZoneMFDSwitchChangeRequest65280(const tN2kMsg& N2kMsg) {

    int idx = 0;
    if (N2kMsg.PGN != 65280UL || N2kMsg.Get2ByteUInt(idx) != CZoneMessage) return;
    idx = 5;
    if (CzDipSwitch != N2kMsg.GetByte(idx)) return; // if byte5 == CzMfdDipSwitch then its from the MFD
    idx = 6; // if byte6 == 0xF1 set switch on or 0xF2 set the switch off
    uint8_t iState = N2kMsg.GetByte(idx);
    if (iState == 0xf1 || iState == 0xf2) {

        idx = 2;
        iState = N2kMsg.GetByte(idx); // Get which bit to toggle 
        switch (iState) {

        case 0x05:  CzSwitchState1 ^= 0x01; // toggle state of the of switch bit
            CzMfdDisplaySyncState1 ^= 0x01; // toggle state of the of switch bit for MDF display sync
            SetChangeSwitchState(1, CzSwitchState1 & 0x01); // send the change out
            break;

        case 0x06:  CzSwitchState1 ^= 0x02;
            CzMfdDisplaySyncState1 ^= 0x04;
            SetChangeSwitchState(2, CzSwitchState1 & 0x02); // send the change out
            break;

        case 0x07:  CzSwitchState1 ^= 0x04;
            CzMfdDisplaySyncState1 ^= 0x10;
            SetChangeSwitchState(3, CzSwitchState1 & 0x04); // send the change out
            break;

        case 0x08:  CzSwitchState1 ^= 0x08;
            CzMfdDisplaySyncState1 ^= 0x40;
            SetChangeSwitchState(4, CzSwitchState1 & 0x08); // send the change out
            break;
// second 4 switches 
        case 0x09:  CzSwitchState2 ^= 0x01; // state of the four switches
            CzMfdDisplaySyncState2 ^= 0x01; // for MDF display sync
            SetChangeSwitchState(5, CzSwitchState2 & 0x01); // send the change out
            break;
        case 0x0a:  CzSwitchState2 ^= 0x02;
            CzMfdDisplaySyncState2 ^= 0x04;
            SetChangeSwitchState(6, CzSwitchState2 & 0x02); // send the change out
            break;
        case 0x0b:  CzSwitchState2 ^= 0x04;
            CzMfdDisplaySyncState2 ^= 0x10;
            SetChangeSwitchState(7, CzSwitchState2 & 0x04); // send the change out
            break;
        case 0x0c:  CzSwitchState2 ^= 0x08;
            CzMfdDisplaySyncState2 ^= 0x40;
            SetChangeSwitchState(8, CzSwitchState2 & 0x08); // send the change out
        }
    }
    else if (iState == 0x40) {  // 0x04 = end of change 65280 msg 
        idx = 2;
        iState = N2kMsg.GetByte(idx); // Get which bit to toggle 
        if (iState > 0x08) SetCZoneSwitchChangeAck65283(CzSwitchBank2SerialNum); // If switch 5 to 8 send sync reply to network
        else  SetCZoneSwitchChangeAck65283(CzSwitchBank1SerialNum); // else sync 1 to 4 to network
    }
}

void SetCZoneSwitchState127501(unsigned char DeviceInstance) {

    tN2kMsg N2kMsg;
    tN2kBinaryStatus BankStatus;
    N2kResetBinaryStatus(BankStatus);
    BankStatus = (BankStatus & CzMfdDisplaySyncState2) << 8; //
    BankStatus = BankStatus | CzMfdDisplaySyncState1;
    BankStatus = BankStatus | 0xffffffffffff0000ULL;
    SetN2kPGN127501(N2kMsg, DeviceInstance, BankStatus);
    NMEA2000.SendMsg(N2kMsg);
}

void SetCZoneSwitchChangeRequest127502(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus)
{
    tN2kMsg N2kMsg;
    N2kResetBinaryStatus(CzBankStatus);
    N2kSetStatusBinaryOnStatus(CzBankStatus, ItemStatus ? N2kOnOff_On : N2kOnOff_Off, SwitchIndex);
    //send out to other N2k switching devices on network ( pgn 127502 )
    SetN2kSwitchBankCommand(N2kMsg, SwitchBankInstance, CzBankStatus);
    NMEA2000.SendMsg(N2kMsg);
}

//*************************************************************************
//      127502 Switch Bank Control
//      Universal commands to multiple banks of two - state devices
//      Field #     Field Description
//            1     Switch bank instance
//            2     Switch 1
//                    --
//            29    Switch 28
//
//*************************************************************************

void SetN2kPGN127502(tN2kMsg& N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus)
{
    N2kMsg.SetPGN(127502L);
    N2kMsg.Priority = 3;
    BankStatus = (BankStatus << 8) | DeviceBankInstance;
    N2kMsg.AddUInt64(BankStatus);
}

//*****************************************************************************
void SetN2kSwitchBankCommand(tN2kMsg& N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus)
{
    SetN2kPGN127502(N2kMsg, DeviceBankInstance, BankStatus);
}

//*****************************************************************************
// Change the state of the relay requested and broadcast change to other N2K switching devices
//*****************************************************************************

void SetChangeSwitchState(uint8_t SwitchIndex, bool ItemStatus) {

    // Set or reset the relay
    if (ItemStatus)
        digitalWrite(CzRelayPinMap[SwitchIndex - 1], HIGH); // adjust SwitchIndex to CzRelayPinMap value and set or reset
    else
        digitalWrite(CzRelayPinMap[SwitchIndex - 1], LOW);
    //send out change and status to other N2k devices on network
    SetCZoneSwitchState127501(BinaryDeviceInstance);
    SetCZoneSwitchChangeRequest127502(SwitchBankInstance, SwitchIndex, ItemStatus);
}



//************************************************************************************************************
//   The MFD sends a 65290 requesting a configuration message, the switch device must respond with
//  a 65290 to match the request.
//************************************************************************************************************/

void ParseCZoneConfigRequest65290(const tN2kMsg& N2kMsg)
{
    int Index = 0;
    if (N2kMsg.PGN != 65290UL || N2kMsg.Get2ByteUInt(Index) != CZoneMessage) return;
    Index = 7;
    if (CzDipSwitch != N2kMsg.GetByte(Index)) return; // if byte7 == CzMfdDipSwitch then its from the MFD
    Index = 2;
    uint8_t CZoneConfig0 = N2kMsg.GetByte(Index);
    uint8_t CZoneConfig1 = N2kMsg.GetByte(Index);
    uint8_t CZoneConfig2 = N2kMsg.GetByte(Index);
    // send the strings for 2 banks of 4 bit switches
    SetCZoneSendConfigToMFD65290(CzSwitchBank1SerialNum, CZoneConfig0, CZoneConfig1, CZoneConfig2);
    if (NumberOfSwitches == 8) SetCZoneSendConfigToMFD65290(CzSwitchBank2SerialNum, CZoneConfig0, CZoneConfig1, CZoneConfig2);

}

// Respond to a configuration request from the MFD

void SetCZoneSendConfigToMFD65290(unsigned char CzSwitchBankSerial, uint8_t CZoneConfig0, uint8_t CZoneConfig1, uint8_t CZoneConfig2)
{
    tN2kMsg N2kMsg;
    N2kMsg.SetPGN(65290L);
    N2kMsg.Priority = 7;
    N2kMsg.Add2ByteUInt(CZoneMessage);
    N2kMsg.Destination = 255;
    N2kMsg.AddByte(CZoneConfig0);
    N2kMsg.AddByte(CZoneConfig1);
    N2kMsg.AddByte(CZoneConfig2);
    N2kMsg.Add2ByteUInt(0x0000);
    N2kMsg.AddByte(CzSwitchBankSerial);
    CzConfigAuthenticated = true;
#ifdef CzDebug 
    Serial.println("Authenticated");
#endif
    NMEA2000.SendMsg(N2kMsg);
}

//************************************************************************************************************
// Czone proprietary fast package sent every 1.5 seconds for other Czone systems to sync to (speculation maybe)
// Not required for Raymarine MFD operation but included for completeness. Czone load controllers have 6 channels
// the YD controllers have 4, each channel is defined by 24 bits (3 bytes) within the packet, so 18 bytes long 
//************************************************************************************************************

void SetCZoneSwitchStateBroadcast130817(unsigned char CzSwitchBankSerialNum) {

    tN2kMsg N2kMsg;
    N2kMsg.SetPGN(130817L);
    N2kMsg.Priority = 7;
    N2kMsg.Add2ByteUInt(CZoneMessage);
    N2kMsg.Destination = 255;
    N2kMsg.AddByte(0x01); // ?? maybe an "instance" value
    if (CzSwitchBankSerialNum == CzSwitchBank1SerialNum) { // is it switch 1 to 4 or 5 to 8
        N2kMsg.AddByte(CzSwitchBank1SerialNum); 
        for (uint8_t i = 0; i < 4; i++)
        {
            if (CzSwitchState1 & (1 << i)) N2kMsg.AddByte(0x01); // if switch is on set Bit 0 of first byte
            else N2kMsg.AddByte(0x00);
            N2kMsg.Add2ByteUInt(0x0000);
        }
    }
    else { 
        N2kMsg.AddByte(CzSwitchBank2SerialNum);
        for (uint8_t i = 0; i < 4; i++)
        {
            if (CzSwitchState2 & (1 << i)) N2kMsg.AddByte(0x01);
            else N2kMsg.AddByte(0x00);
            N2kMsg.Add2ByteUInt(0x0000);
        }
    }
    N2kMsg.Add3ByteInt(0); // Pad out for the non existant switches
    N2kMsg.Add3ByteInt(0);
    NMEA2000.SendMsg(N2kMsg);
}
//*****************************************************************************
void ParseN2kPGN127502(const tN2kMsg& N2kMsg) {

    tN2kOnOff State;
    unsigned char ChangeIndex;
    int Index = 0;
    uint8_t DeviceBankInstance = N2kMsg.GetByte(Index);
    if (N2kMsg.PGN != 127502L && DeviceBankInstance  != BinaryDeviceInstance) return; //Nothing to see here
    uint16_t BankStatus = N2kMsg.Get2ByteUInt (Index);
    //Serial.println(BankStatus);
    for (ChangeIndex = 0; ChangeIndex < NumberOfSwitches; ChangeIndex++) {

        State = (tN2kOnOff)(BankStatus & 0x03);
        if (State != N2kOnOff_Unavailable) break; // index (0 to 7) found for switch and state
        BankStatus >>= 2;
    }
    //Serial.println(ChangeIndex );
    //Serial.println(State);

    switch (ChangeIndex) {

    case 0x00:  CzSwitchState1 ^= 0x01; // toggle state of the of switch bit
        CzMfdDisplaySyncState1 ^= 0x01; // toggle state of the of switch bit for MDF display sync
        SetChangeSwitchState(1, CzSwitchState1 & 0x01); // send the change out
        break;

    case 0x01:  CzSwitchState1 ^= 0x02;
        CzMfdDisplaySyncState1 ^= 0x04;
        SetChangeSwitchState(2, CzSwitchState1 & 0x02); // send the change out
        break;

    case 0x02:  CzSwitchState1 ^= 0x04;
        CzMfdDisplaySyncState1 ^= 0x10;
        SetChangeSwitchState(3, CzSwitchState1 & 0x04); // send the change out
        break;

    case 0x03:  CzSwitchState1 ^= 0x08;
        CzMfdDisplaySyncState1 ^= 0x40;
        SetChangeSwitchState(4, CzSwitchState1 & 0x08); // send the change out
        break;
        // second 4 switches 
    case 0x04:  CzSwitchState2 ^= 0x01; // state of the four switches
        CzMfdDisplaySyncState2 ^= 0x01; // for MDF display sync
        SetChangeSwitchState(5, CzSwitchState2 & 0x01); // send the change out
        break;
    case 0x05:  CzSwitchState2 ^= 0x02;
        CzMfdDisplaySyncState2 ^= 0x04;
        SetChangeSwitchState(6, CzSwitchState2 & 0x02); // send the change out
        break;
    case 0x06:  CzSwitchState2 ^= 0x04;
        CzMfdDisplaySyncState2 ^= 0x10;
        SetChangeSwitchState(7, CzSwitchState2 & 0x04); // send the change out
        break;
    case 0x07:  CzSwitchState2 ^= 0x08;
        CzMfdDisplaySyncState2 ^= 0x40;
        SetChangeSwitchState(8, CzSwitchState2 & 0x08); // send the change out
    }

    
    if (ChangeIndex > 0x08) SetCZoneSwitchChangeAck65283(CzSwitchBank2SerialNum); // If switch 5 to 8 send sync reply to network
    else  SetCZoneSwitchChangeAck65283(CzSwitchBank1SerialNum); // else sync 1 to 4 to network
    
}
