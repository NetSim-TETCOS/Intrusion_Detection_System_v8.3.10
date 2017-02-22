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

int fn_NetSim_WiMax_Receive_MAC_PDU()
{
	NETSIM_ID DeviceId,InterfaceId;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;
	NetSim_PACKET* Packet;
	WIMAX_SS_INFO* info;
	double PacketSize;
	double PacketSize_available;
	unsigned int SymbolRequired;
	unsigned int ServiceType;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac =(WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);
	info =BS_Mac->ssInfo;

	while(info)
	{
		while(fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(info->nSSId,info->nSSInterfaceId)->pstruAccessInterface->pstruAccessBuffer))
		{
			Packet = fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_MAC_NW_INTERFACE(info->nSSId,info->nSSInterfaceId)->pstruAccessBuffer,0);
			
			//Set the receiver id
			Packet->nReceiverId = DeviceId;
			Packet->nTransmitterId = info->nSSId;
					
			PacketSize = fnGetPacketSize(Packet);
			SymbolRequired = (int)ceil(PacketSize*8.0/BS_Phy->BitsCountInOneSymbol);
					
			if(BS_Mac->Symbol_Available_UL > SymbolRequired)
			{
				ServiceType = fn_NetSim_WiMax_GetBurstNo(Packet->nQOS);
				Packet = fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_MAC_NW_INTERFACE(info->nSSId,info->nSSInterfaceId)->pstruAccessBuffer,1);
				Packet->pstruMacData->dPayload = Packet->pstruNetworkData->dPacketSize;
				
				fn_NetSim_Write_Phy_Out_SS(BS_Mac,Packet,BS_Phy->pstruSymbolParameter,SymbolRequired);
				BS_Mac->Symbol_Available_UL -= SymbolRequired;
				fn_NetSim_WiMax_Update_UL_Map_IE(SymbolRequired);
			}
			else
			{
				if(BS_Mac->Symbol_Available_UL > 1)
				{
					unsigned int symbolreqd_frag=0;
					PacketSize_available = ((BS_Mac->Symbol_Available_UL-1)*BS_Phy->BitsCountInOneSymbol/8)-MAC_HEADER_SIZE;
					Packet = fn_NetSim_WiMax_Fragment_Packet(PacketSize_available,info->nSSId,info->nSSInterfaceId);
			
					symbolreqd_frag = (int)ceil(PacketSize_available*8.0/BS_Phy->BitsCountInOneSymbol);
					fn_NetSim_Write_Phy_Out_SS(BS_Mac,Packet,BS_Phy->pstruSymbolParameter,symbolreqd_frag);
								
					BS_Mac->Symbol_Available_UL -= symbolreqd_frag ;
					fn_NetSim_WiMax_Update_UL_Map_IE(symbolreqd_frag);
				}
				return 0;
			}
		}
		info=(WIMAX_SS_INFO*)LIST_NEXT(info);
	}
	
	return 0;
}

int fn_NetSim_Write_Phy_Out_SS(WIMAX_BS_MAC* BS_Mac, NetSim_PACKET* Packet, SYMBOL_PARAMETER* SymbolParameter,unsigned int SymbolRequired)
{
	double StartTime, EndTime;
	NetSim_EVENTDETAILS pevent;
		
	StartTime = (SymbolParameter->UPlinkFrameStartSymbol+BS_Mac->Symbol_Count_UL-BS_Mac->Symbol_Available_UL)* SymbolParameter->OFDMSymbolTime + BS_Mac->FrameStartTime; 
	EndTime = StartTime+ (SymbolRequired)*SymbolParameter->OFDMSymbolTime;

	Packet->pstruMacData->dOverhead = MAC_HEADER_SIZE;
	Packet->pstruMacData->dPacketSize = Packet->pstruMacData->dOverhead \
										+ Packet->pstruMacData->dPayload;
	Packet->pstruMacData->dArrivalTime = StartTime;
	Packet->pstruMacData->dEndTime = EndTime;
	Packet->pstruPhyData->dArrivalTime = StartTime;
	Packet->pstruPhyData->dEndTime = EndTime;
	Packet->pstruPhyData->dPacketSize = Packet->pstruMacData->dPacketSize; 
	Packet->pstruPhyData->dStartTime = StartTime;
	Packet->pstruPhyData->nPacketErrorFlag = PacketStatus_NoError;
	Packet->pstruPhyData->nPhyMedium = PHY_MEDIUM_WIRELESS;
	
	memcpy(&pevent,pstruEventDetails,sizeof* pstruEventDetails);
	pevent.nDeviceId = Packet->nTransmitterId;
	pevent.nDeviceType = SUBSCRIBER;
	pevent.pPacket = Packet;
	pevent.dEventTime = StartTime;
	pevent.dPacketSize = Packet->pstruMacData->dPacketSize;//MAC Header
	pevent.nEventType = PHYSICAL_OUT_EVENT;
	pevent.nSubEventType = TRANSMIT_MAC_PDU;
	fnpAddEvent(&pevent);
	return 1; //Allocation successful
}

int fn_NetSim_WiMax_Update_UL_Map_IE(unsigned int symbolreqd)
{
	NETSIM_ID DeviceId,InterfaceId;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;
	UL_Map_IE* struUL_MAP_IE, *struUL_MAP_IE2=NULL;
	double NewPacketSize;
	unsigned int SymbolRequired;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac = (WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);

	struUL_MAP_IE = (UL_Map_IE* )(fnpAllocateMemory(1,sizeof *BS_Mac->struUL_MAP->pstruULMAPIE));
	struUL_MAP_IE->CID = 0;
	struUL_MAP_IE->Starttime = BS_Phy->pstruSymbolParameter->UPlinkFrameStartSymbol;
	struUL_MAP_IE->duration = symbolreqd;
	struUL_MAP_IE->UIUC =0;
	struUL_MAP_IE->SubchannelIndex = 0;
	struUL_MAP_IE->MidambleRepetitionInterval=0;
	
	if(!BS_Mac->struUL_MAP->pstruULMAPIE)
	{
		BS_Mac->struUL_MAP->pstruULMAPIE = struUL_MAP_IE;
	}
	else
	{
		struUL_MAP_IE2 = BS_Mac->struUL_MAP->pstruULMAPIE;
		while(struUL_MAP_IE2->pstruNext)
			struUL_MAP_IE2 =struUL_MAP_IE2->pstruNext;
		struUL_MAP_IE2->pstruNext = struUL_MAP_IE;
	}
	BS_Mac->struUL_MAP->IE_Count++;
	
	NewPacketSize = BS_Mac->struUL_MAP->PacketSize + 48/8;
	
	SymbolRequired = (int)ceil(NewPacketSize*8.0/struPhyParameters[0].BitsCountInOneSymbol);

	if(SymbolRequired > BS_Mac->Symbol_Reqd_UL_MAP)
	{
		BS_Mac->Symbol_Available_UL--;
		BS_Mac->Symbol_Reqd_UL_MAP = SymbolRequired;
	}
	BS_Mac->struUL_MAP->PacketSize = NewPacketSize;
	return 0;
}