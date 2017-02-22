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


double fn_NetSim_WiMax_FormUL_MAP(double time)
{
	NETSIM_ID DeviceId;
	NETSIM_ID InterfaceId;
	WIMAX_BS_MAC* BS_MAC;
	WIMAX_BS_PHY* BS_PHY;
	NetSim_PACKET* Packet;
	double Transmissiontime=0;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_MAC =(WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_PHY = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);
	
	BS_MAC->struUL_MAP = (UL_MAP* )fnpAllocateMemory(1,sizeof(UL_MAP));
	BS_MAC->struUL_MAP->ManagementMessageType = MMM_UL_MAP;
	BS_MAC->struUL_MAP->AllocationStartTime = BS_PHY->pstruSymbolParameter->UPlinkFrameStartSymbol & 0xFFFF;
	BS_MAC->struUL_MAP->UCDCount = 0;
	
	Packet = fn_NetSim_WiMax_GenerateBroadcastCtrlPacket(DeviceId,InterfaceId,MMM_UL_MAP,time);
	
	BS_MAC->struUL_MAP->PacketSize = fnGetPacketSize(Packet);
	
	BS_MAC->Symbol_Reqd_UL_MAP = 1;
	BS_MAC->Symbol_Available_DL -=BS_MAC->Symbol_Reqd_UL_MAP;

	fn_NetSim_AddPacketToList(&BS_PHY->struDLPacketList[0],Packet);

	BS_MAC->struUL_MAP = Packet->pstruMacData->Packet_MACProtocol;
	Transmissiontime = fn_NetSim_WiMax_CalculateTransmissionTime(Packet,BS_PHY);
	return time+Transmissiontime;
}