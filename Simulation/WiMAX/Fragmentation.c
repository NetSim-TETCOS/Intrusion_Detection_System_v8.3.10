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

NetSim_PACKET* fn_NetSim_WiMax_Fragment_Packet(double Size_fragment,NETSIM_ID DeviceId,NETSIM_ID InterfaceId)
{
	NetSim_PACKET* Packet;
	FRAGMENT_SUB_HEADER* Fragment_Header;
	NetSim_PACKET* NewPacket = NULL;
	WIMAX_MAC_HEADER* MAC_Header1;
	WIMAX_MAC_HEADER* MAC_Header2;

	Packet = fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_MAC_NW_INTERFACE(DeviceId,InterfaceId)->pstruAccessBuffer,0);
	Packet->pstruNetworkData->dPacketSize -= Size_fragment;
	Packet->pstruAppData->dPayload -= Size_fragment;
	
	NewPacket = fn_NetSim_Packet_CopyPacket(Packet);
	NewPacket->pstruMacData->dPayload = Size_fragment;
	NewPacket->pstruNetworkData->dPacketSize = Size_fragment;
	NewPacket->pstruAppData->dPayload = Size_fragment;
	
	if(NewPacket->pstruMacData->Packet_MACProtocol)
	{
		MAC_Header1 = (WIMAX_MAC_HEADER*)NewPacket->pstruMacData->Packet_MACProtocol;	
	}
	else
	{
		MAC_Header1 = (WIMAX_MAC_HEADER*)fnpAllocateMemory(1,sizeof* MAC_Header1);
	}
	if(Packet->pstruMacData->Packet_MACProtocol)
		MAC_Header2 = (WIMAX_MAC_HEADER*)Packet->pstruMacData->Packet_MACProtocol;	
	else
		MAC_Header2 = (WIMAX_MAC_HEADER*)fnpAllocateMemory(1,sizeof* MAC_Header2);

	MAC_Header1->HT =0;
	MAC_Header1->EC =0;
	MAC_Header1->CID = 0;
	MAC_Header1->HCS =0;
	MAC_Header1->CI =0;
	MAC_Header1->EKS =0;
	MAC_Header1->Type[FRAGMENT_BIT] =1;

	MAC_Header2->HT =0;
	MAC_Header2->EC =0;
	MAC_Header2->CID = 0;
	MAC_Header2->HCS =0;
	MAC_Header2->CI =0;
	MAC_Header2->EKS =0;
	MAC_Header2->Type[FRAGMENT_BIT] =1;

	if(!MAC_Header2->Fragment_Header)
		MAC_Header2->Fragment_Header =(FRAGMENT_SUB_HEADER*)fnpAllocateMemory(1,sizeof* Fragment_Header);

	if(MAC_Header1->Fragment_Header)
	{
		MAC_Header1->Fragment_Header->FC = B2_11;
	}
	else
	{
		MAC_Header1->Fragment_Header = (FRAGMENT_SUB_HEADER*)fnpAllocateMemory(1,sizeof* Fragment_Header);
		MAC_Header1->Fragment_Header->FC = B2_10;
		MAC_Header1->Fragment_Header->FSN++;
	}

	MAC_Header2->Fragment_Header->FC = B2_01;//last fragment
	MAC_Header2->Fragment_Header->FSN = MAC_Header1->Fragment_Header->FSN + 1;

	NewPacket->pstruMacData->Packet_MACProtocol = MAC_Header1;
	NewPacket->pstruMacData->nMACProtocol = MAC_PROTOCOL_IEEE802_16;
	Packet->pstruMacData->Packet_MACProtocol = MAC_Header2;
	Packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_IEEE802_16;

	return NewPacket;
}


int fn_NetSim_Pack_Fragmented_Packet(NETSIM_ID DeviceId,NETSIM_ID InterfaceId)
{
	NetSim_PACKET* Fragment;
	WIMAX_MAC_HEADER* MAC_Header;
	NetSim_PACKET** Fragment_Packet;

	if(DEVICE_TYPE(DeviceId)==BASESTATION)
		Fragment_Packet = ((WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId))->Fragment_Packet;

	else if(DEVICE_TYPE(DeviceId)==SUBSCRIBER)
		Fragment_Packet = ((WIMAX_SS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId))->Fragment_Packet;
	else
	{
		fnNetSimError("error");
		return 0;
	}

	Fragment = pstruEventDetails->pPacket;

	if(Fragment->pstruMacData->Packet_MACProtocol)
	{
		MAC_Header = (WIMAX_MAC_HEADER*)Fragment->pstruMacData->Packet_MACProtocol;
	}
	else 
	{
		return 0;
	}

	if (MAC_Header->Type[FRAGMENT_BIT] == 1)
	{
		switch(MAC_Header->Fragment_Header->FC) 
		{
		case B2_10:
			Fragment_Packet[Fragment->nTransmitterId] = Fragment;
			fn_NetSim_WiMax_FreePacket_F(Fragment);
			Fragment->pstruMacData->Packet_MACProtocol = NULL;
			pstruEventDetails->pPacket = NULL;
			break;
		case B2_11:
			Fragment_Packet[Fragment->nTransmitterId]->pstruNetworkData->dPacketSize += Fragment->pstruNetworkData->dPacketSize;
			fn_NetSim_Packet_FreePacket(Fragment); 
			pstruEventDetails->pPacket = NULL;
			break;
		case B2_01:
			Fragment_Packet[Fragment->nTransmitterId]->pstruNetworkData->dPacketSize += Fragment->pstruNetworkData->dPacketSize;
			pstruEventDetails->pPacket = Fragment_Packet[Fragment->nTransmitterId];
			Fragment_Packet[Fragment->nTransmitterId] = NULL;
			fn_NetSim_Packet_FreePacket(Fragment); 
			break;
		default:
			//no fragment
			break;
		}	
	}
	return 0;
}

