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

int getsymbolreqd(WIMAX_BS_PHY* BS_Phy,WIMAX_BS_MAC* BS_Mac,NetSim_PACKET* packet)
{
	double size; 
	int reqd =0;
	
	if(packet->nControlDataType/100 == MAC_PROTOCOL_IEEE802_16)
	{
		switch(packet->nControlDataType%100) 
		{
		case MMM_UL_MAP:
			reqd = BS_Mac->Symbol_Reqd_UL_MAP;
			break;
		case MMM_DL_MAP:
			reqd = BS_Mac->Symbol_Reqd_DL_MAP;
			break;
		case MMM_FCH:
			reqd = 1;
			break;
		}
	}
	else
	{
		size = packet->pstruNetworkData->dPacketSize;
		reqd= (int)ceil(size*8.0/BS_Phy->BitsCountInOneSymbol);
	}
	return reqd;
}

int fn_NetSim_WiMax_Write_Phy_Out_BS(double fch_time,double dl_map_time,double ul_map_time)
{
	NETSIM_ID DeviceId,InterfaceId;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;
	NetSim_PACKET* Packet;
	unsigned int SymbolRequired; 

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac =(WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy =(WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);

	Packet = fn_NetSim_WiMax_GetPacketFromList(BS_Phy,0);
	while(Packet)
	{
		pstruEventDetails->pPacket = Packet;
		pstruEventDetails->dPacketSize = fnGetPacketSize(pstruEventDetails->pPacket);
		if(Packet->nControlDataType/100 == MAC_PROTOCOL_IEEE802_16)
		{
			switch(Packet->nControlDataType%100)
			{
			case MMM_FCH:
				SymbolRequired = getsymbolreqd(BS_Phy,BS_Mac,Packet);
				pstruEventDetails->dEventTime = fch_time;
				pstruEventDetails->nPacketId = 0;
				pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
				pstruEventDetails->nSubEventType = TRANSMIT_FCH;
				fnpAddEvent(pstruEventDetails);
				break;

			case MMM_DL_MAP:
				SymbolRequired = getsymbolreqd(BS_Phy,BS_Mac,Packet);
				pstruEventDetails->dEventTime = dl_map_time; 
				pstruEventDetails->nPacketId = 0;
				pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
				pstruEventDetails->nSubEventType = TRANSMIT_DL_MAP;
				fnpAddEvent(pstruEventDetails);
				fn_NetSim_Update_FCH(BS_Mac->struFCH,pstruEventDetails->dPacketSize,MMM_DL_MAP);
				break;

			case MMM_UL_MAP:
				SymbolRequired = getsymbolreqd(BS_Phy,BS_Mac,Packet);
				pstruEventDetails->dEventTime = ul_map_time; 
				pstruEventDetails->nPacketId = 0;
				pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
				pstruEventDetails->nSubEventType = TRANSMIT_UL_MAP;
				fnpAddEvent(pstruEventDetails);
				fn_NetSim_Update_FCH(BS_Mac->struFCH,pstruEventDetails->dPacketSize,MMM_UL_MAP);
				break;
			}
		}
		else
		{
			SymbolRequired = getsymbolreqd(BS_Phy,BS_Mac,Packet);
			pstruEventDetails->dEventTime = ul_map_time; 
			pstruEventDetails->pPacket->pstruMacData->dEndTime = ul_map_time + SymbolRequired*BS_Phy->pstruSymbolParameter->OFDMSymbolTime;
			pstruEventDetails->pPacket->pstruPhyData->dArrivalTime = ul_map_time;
			pstruEventDetails->pPacket->pstruPhyData->dStartTime = ul_map_time;
			pstruEventDetails->pPacket->pstruMacData->dStartTime = ul_map_time;
			pstruEventDetails->pPacket->pstruPhyData->dEndTime = ul_map_time+SymbolRequired*BS_Phy->pstruSymbolParameter->OFDMSymbolTime;
			pstruEventDetails->pPacket->pstruPhyData->dPacketSize = Packet->pstruNetworkData->dPacketSize;
			pstruEventDetails->nPacketId = Packet->nPacketId;
			pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
			pstruEventDetails->nSubEventType = TRANSMIT_MAC_PDU;
			fnpAddEvent(pstruEventDetails);
			if(Packet->pstruMacData->Packet_MACProtocol)
			{
				ul_map_time = pstruEventDetails->pPacket->pstruMacData->dEndTime;
			}
		}
		Packet = fn_NetSim_WiMax_GetPacketFromList(BS_Phy,0);
	}
	return 1;
}

NetSim_PACKET* fn_NetSim_WiMax_GetPacketFromList(WIMAX_BS_PHY* BS_Phy,int index)
{
	NetSim_PACKET* packet = BS_Phy->struDLPacketList[index];
	if(packet)
	{
		BS_Phy->struDLPacketList[index] = packet->pstruNextPacket;
		packet->pstruNextPacket = NULL;
	}
	return packet;
}
