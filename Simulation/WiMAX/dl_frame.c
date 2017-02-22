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

int fn_NetSim_WiMax_Send_MAC_PDU()
{
	NETSIM_ID DeviceId,InterfaceId;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;
	NetSim_PACKET* Packet;
	double PacketSize;
	double PacketSize_available;
	unsigned int SymbolRequired;
	unsigned int ServiceType;
		
	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac =(WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);
	
	if(BS_Mac->Symbol_Available_DL > 0)
	{
		while(fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(DeviceId,InterfaceId)->pstruAccessInterface->pstruAccessBuffer))
		{
			Packet = fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_MAC_NW_INTERFACE(DeviceId,InterfaceId)->pstruAccessBuffer,0);
			
			//Set the receiver id
			Packet->nReceiverId = Packet->nDestinationId;
			Packet->nTransmitterId = DeviceId;
		
			PacketSize = Packet->pstruNetworkData->dPacketSize;
			SymbolRequired = (int)ceil(PacketSize*8.0/BS_Phy->BitsCountInOneSymbol);
		
			if(BS_Mac->Symbol_Available_DL > SymbolRequired)
			{
				ServiceType = fn_NetSim_WiMax_GetBurstNo(Packet->nQOS);
				Packet = fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_MAC_NW_INTERFACE(DeviceId,InterfaceId)->pstruAccessBuffer,1);
				Packet->pstruMacData->dPayload = Packet->pstruNetworkData->dPacketSize;

				fn_NetSim_AddPacketToList(&BS_Phy->struDLPacketList[0],Packet);
				
				BS_Mac->Symbol_Available_DL -= SymbolRequired;
				fn_NetSim_WiMax_Update_DL_Map_IE();
			}
			else
			{
				if(BS_Mac->Symbol_Available_DL > 1)
				{
					PacketSize_available = ((BS_Mac->Symbol_Available_DL-1)*BS_Phy->BitsCountInOneSymbol/8)-MAC_HEADER_SIZE;
					Packet = fn_NetSim_WiMax_Fragment_Packet(PacketSize_available,DeviceId,InterfaceId);
				
					fn_NetSim_AddPacketToList(&BS_Phy->struDLPacketList[0],Packet);
				}
				BS_Mac->Symbol_Available_DL = 1;
				fn_NetSim_WiMax_Update_DL_Map_IE();
				return 0;
			}
		}
	}
	
	return 0;
}

int fn_NetSim_WiMax_Update_DL_Map_IE()
{
	NETSIM_ID DeviceId,InterfaceId;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;
	DL_Map_IE* struDL_MAP_IE, *struDL_MAP_IE2;
	double NewPacketSize;
	unsigned int SymbolRequired;

	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;

	BS_Mac =(WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);
	BS_Phy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(DeviceId,InterfaceId);

	struDL_MAP_IE = (DL_Map_IE* )(fnpAllocateMemory(1,sizeof *struDL_MAP_IE));
	struDL_MAP_IE->CID = 0; //Not implemented.
	struDL_MAP_IE->StartTime = BS_Mac->Symbol_Count_DL - BS_Mac->Symbol_Available_DL;
	struDL_MAP_IE->DIUC = 11;
	struDL_MAP_IE->PreamblePresent = 0;

	if(!BS_Mac->struDL_MAP->pstruDLMAPIE)
	{
		BS_Mac->struDL_MAP->pstruDLMAPIE = struDL_MAP_IE;
	}
	else
	{
		struDL_MAP_IE2 = BS_Mac->struDL_MAP->pstruDLMAPIE;
		while(struDL_MAP_IE2->pstruNext)
			struDL_MAP_IE2 = struDL_MAP_IE2->pstruNext;
		struDL_MAP_IE2->pstruNext = struDL_MAP_IE;
	}
	BS_Mac->struDL_MAP->IE_Count++;
	
	NewPacketSize = BS_Mac->struDL_MAP->PacketSize + 32/8;
	
	SymbolRequired = (int)ceil(NewPacketSize*8.0/struPhyParameters[0].BitsCountInOneSymbol);

	if(SymbolRequired > BS_Mac->Symbol_Reqd_DL_MAP)
	{
		BS_Mac->Symbol_Available_DL--;
		BS_Mac->Symbol_Reqd_DL_MAP = SymbolRequired;
	}
	BS_Mac->struDL_MAP->PacketSize = NewPacketSize;
	return 0;
}