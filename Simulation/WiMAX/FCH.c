/************************************************************************************
 * Copyright (C) 2015                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author: Surabhi Bothra	                                                        *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/

#include "main.h"
#include "WiMax.h"

double fn_NetSim_WiMax_FormFCH(double time)
{
	NetSim_PACKET* Packet;
	NETSIM_ID DeviceId;
	NETSIM_ID InterfaceId;
	SYMBOL_PARAMETER* struSymbol;
	unsigned int SymbolRequiredforFCH;
	
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac = (WIMAX_BS_MAC* )DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY* )DEVICE_PHYVAR(DeviceId,InterfaceId);
	struSymbol = BS_Phy->pstruSymbolParameter;
	
	//Fill the first symbol by FCH frame
	//Generate the FCH frame
	Packet = fn_NetSim_WiMax_GenerateBroadcastCtrlPacket(DeviceId,InterfaceId,MMM_FCH,time);
	Packet->dEventTime = time;
	Packet->pstruMacData->dArrivalTime = time;
	pstruEventDetails->pPacket = Packet;	
	if(BS_Phy->FrameNumber)
	{
		BS_Mac->FrameStartTime = pstruEventDetails->dEventTime;
	}
	//Increment the frame number
	BS_Phy->FrameNumber++;
	SymbolRequiredforFCH = 1;
	BS_Mac->Symbol_Available_DL -= SymbolRequiredforFCH;
	//Add FCH to BS PHY Packelist
	fn_NetSim_AddPacketToList(&BS_Phy->struDLPacketList[0],Packet);
	//Add FCH to BS MAC
	BS_Mac->struFCH = Packet;
	return (time + BS_Phy->pstruSymbolParameter->OFDMSymbolTime); //1 symbol;
}

int fn_NetSim_Update_FCH(NetSim_PACKET* FCH_pkt,double Size,MANAGEMENT_MESSAGE Message)
{
	WIMAX_BS_MAC* BS_MAC;
	WIMAX_BS_PHY* BS_Phy;
	NETSIM_ID Device,Interface;
	void* Mac_Header;
	int No_Of_Symbol_required;

	Device = pstruEventDetails->nDeviceId;
	Interface = pstruEventDetails->nInterfaceId;

	BS_MAC = (WIMAX_BS_MAC* )DEVICE_MACVAR(Device,Interface);
	BS_Phy = (WIMAX_BS_PHY* )DEVICE_PHYVAR(Device,Interface);
	Mac_Header = FCH_pkt->pstruMacData->Packet_MACProtocol;
	No_Of_Symbol_required = (int)ceil(Size*8.0/struPhyParameters[0].BitsCountInOneSymbol);
	
		switch(Message)
		{
		case MMM_DL_MAP:
			((FCH_Header*)Mac_Header)->pstruDL_FP_IE[0]->length=BS_MAC->Symbol_Reqd_DL_MAP;//No of Symbols required for DL-Map
			break;
		case MMM_UL_MAP:
			((FCH_Header*)Mac_Header)->pstruDL_FP_IE[1]->length=BS_MAC->Symbol_Reqd_UL_MAP;
			break;
		default:
			break;
		}
	return 0;
}