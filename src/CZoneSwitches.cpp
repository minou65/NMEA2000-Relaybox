// 
// 
// 

#include "CZoneSwitches.h"
#include <N2kMessages.h>

//********************************************************************
//      PGN65284 CZone Switch Bank Command message. This needs to be
//      sent every 2 seconds for each switch bank. 
// ********************************************************************

void SetCZoneSwitchHeartbeat65284(tN2kMsg& N2kMsg, unsigned char CzSwitchBankSerialNum) {

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

void SetCZoneSwitchChangeAck65283(tN2kMsg& N2kMsg, unsigned char CzSwitchBankSerialNum) {

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

void SetCZoneSwitchState127501(tN2kMsg& N2kMsg, unsigned char DeviceInstance) {
    tN2kBinaryStatus BankStatus;
    N2kResetBinaryStatus(BankStatus);
    BankStatus = (BankStatus & CzMfdDisplaySyncState2) << 8; //
    BankStatus = BankStatus | CzMfdDisplaySyncState1;
    BankStatus = BankStatus | 0xffffffffffff0000ULL;
    SetN2kPGN127501(N2kMsg, DeviceInstance, BankStatus);
}

void SetCZoneSwitchChangeRequest127502(tN2kMsg& N2kMsg, unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus){
    N2kResetBinaryStatus(CzBankStatus);
    N2kSetStatusBinaryOnStatus(CzBankStatus, ItemStatus ? N2kOnOff_On : N2kOnOff_Off, SwitchIndex);
    //send out to other N2k switching devices on network ( pgn 127502 )
    SetN2kSwitchBankCommand(N2kMsg, SwitchBankInstance, CzBankStatus);
}


