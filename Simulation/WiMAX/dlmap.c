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


double fn_NetSim_WiMax_FormDL_MAP(double time)
{
	NetSim_PACKET* Packet;
	NETSIM_ID DeviceId;
	NETSIM_ID InterfaceId;
	WIMAX_BS_PHY* BS_Phy;
	WIMAX_BS_MAC* BS_MAC;
	double TransmissionTime=0;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_MAC = (WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);
	
	//Allocate memory for DL-MAP
	BS_MAC->struDL_MAP = (DL_MAP* )fnpAllocateMemory(1,sizeof(DL_MAP));
	BS_MAC->struDL_MAP->ManagementMessageType = MMM_DL_MAP;
	BS_MAC->struDL_MAP->DCDCount = 0;
	BS_MAC->struDL_MAP->BS_ID = DeviceId;
	BS_MAC->struDL_MAP->IE_Count = 0;
		
	Packet = fn_NetSim_WiMax_GenerateBroadcastCtrlPacket(DeviceId,InterfaceId,MMM_DL_MAP,time);
	
	BS_MAC->struDL_MAP->PacketSize = fnGetPacketSize(Packet);
	BS_MAC->Symbol_Reqd_DL_MAP = 1;
	BS_MAC->Symbol_Available_DL -=BS_MAC->Symbol_Reqd_DL_MAP;

	fn_NetSim_AddPacketToList(&BS_Phy->struDLPacketList[0],Packet);

	TransmissionTime = fn_NetSim_WiMax_CalculateTransmissionTime(Packet,BS_Phy);
	BS_MAC->struDL_MAP = Packet->pstruMacData->Packet_MACProtocol;

	return time+TransmissionTime;
}