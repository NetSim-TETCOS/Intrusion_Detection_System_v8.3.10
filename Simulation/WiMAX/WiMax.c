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

char* fn_NetSim_WiMax_WritePacketTrace_F();

_declspec(dllexport) int fn_NetSim_WiMax_Init(struct stru_NetSim_Network *NETWORK_Formal,\
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,\
	char *pszWritePath_Formal,int nVersion_Type,void **fnPointer)
{
	fn_NetSim_WiMax_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,pszWritePath_Formal,nVersion_Type,fnPointer);
	return 1;
}

_declspec(dllexport) int fn_NetSim_WiMax_Run()
{
	switch(pstruEventDetails->nEventType)
	{
	case MAC_OUT_EVENT:
		//No processing
		break;

	case MAC_IN_EVENT:
		if(pstruEventDetails->pPacket->nControlDataType/100 != MAC_PROTOCOL_IEEE802_16)
		{
			fn_NetSim_Pack_Fragmented_Packet(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
			if(pstruEventDetails->pPacket)
			{
				pstruEventDetails->pPacket->pstruMacData->dPacketSize =0;
				pstruEventDetails->pPacket->pstruMacData->dOverhead =0;
				pstruEventDetails->pPacket->pstruMacData->dPayload =0;
				pstruEventDetails->pPacket->pstruPhyData->dPacketSize =0;
				pstruEventDetails->pPacket->pstruPhyData->dOverhead =0;
				pstruEventDetails->pPacket->pstruPhyData->dPayload =0;
			}
		}
		if(pstruEventDetails->pPacket)
		{
			switch(pstruEventDetails->nDeviceType)
			{
				case BASESTATION:
					fn_NetSim_WiMax_BS_Mac_In();
					break;
				default:
					fn_NetSim_WiMax_SS_Mac_In();
					break;
			}
		}
		break;

	case PHYSICAL_OUT_EVENT:

		switch(pstruEventDetails->nDeviceType)
		{
		case BASESTATION:
			fn_NetSim_WiMax_BS_Physical_Out();
			break;
		default:	 //Subscriber, Node
			fn_NetSim_WiMax_SS_Physical_Out();
			break;
		}
		break;

	case PHYSICAL_IN_EVENT:

		fn_NetSim_WiMax_Physical_In();
		break;

	case TIMER_EVENT:

		switch(pstruEventDetails->nSubEventType)
		{
		case TRANSMIT_FCH:
			{
				double fch_time,dl_map_time,ul_map_time;
				WIMAX_BS_MAC* BS_Mac;
				WIMAX_BS_PHY* BS_Phy;
				BS_Mac =(WIMAX_BS_MAC*)DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
				BS_Phy =(WIMAX_BS_PHY*)DEVICE_PHYVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
				BS_Mac->Symbol_Available_DL = BS_Mac->Symbol_Count_DL;
				BS_Mac->Symbol_Available_UL = BS_Mac->Symbol_Count_UL;
				fch_time = fn_NetSim_WiMax_FormFCH(pstruEventDetails->dEventTime);
				dl_map_time = fn_NetSim_WiMax_FormDL_MAP(fch_time);
				ul_map_time = fn_NetSim_WiMax_FormUL_MAP(dl_map_time);
				fn_NetSim_WiMax_Send_MAC_PDU();
				fn_NetSim_WiMax_Receive_MAC_PDU();						
				fn_NetSim_WiMax_Write_Phy_Out_BS(fch_time,dl_map_time,ul_map_time);
				//Schedule next FCH event
				pstruEventDetails->dEventTime = BS_Mac->FrameStartTime + BS_Phy->FrameDuration*MILLISECOND;
				pstruEventDetails->dPacketSize = 0;
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nSubEventType = TRANSMIT_FCH;
				pstruEventDetails->pPacket = NULL;
				fnpAddEvent(pstruEventDetails);
			}
			break;
		}
		break;

	default:
		printf("WiMax--- Unknown sub event type %d of event type %d for WiMax Network.\n",pstruEventDetails->nSubEventType,pstruEventDetails->nEventType);
		break;
	}
	return 1;
}

_declspec(dllexport) char* fn_NetSim_WiMax_Trace(int nSubEvent)
{
	return fn_NetSim_WiMax_Trace_F(nSubEvent);
}
_declspec(dllexport) int fn_NetSim_WiMax_FreePacket(NetSim_PACKET* packet)
{
	return fn_NetSim_WiMax_FreePacket_F(packet);
}
_declspec(dllexport) int fn_NetSim_WiMax_CopyPacket(const NetSim_PACKET* srcPacket,const NetSim_PACKET* destPacket)
{
	return fn_NetSim_WiMax_CopyPacket_F(destPacket,srcPacket);
}
_declspec(dllexport) int fn_NetSim_WiMax_Metrics(char* file)
{
	return fn_NetSim_WiMax_Metrics_F(file);
}
_declspec(dllexport) int fn_NetSim_WiMax_Configure(void** var)
{
	return fn_NetSim_WiMax_Configure_F(var);
}
_declspec(dllexport) char* fn_NetSim_WiMax_ConfigPacketTrace()
{
	return fn_NetSim_WiMax_ConfigPacketTrace_F();
}
_declspec(dllexport) int fn_NetSim_WiMax_Finish()
{
	return fn_NetSim_WiMax_Finish_F();
}
_declspec(dllexport) char* fn_NetSim_WiMax_WritePacketTrace()
{
	return fn_NetSim_WiMax_WritePacketTrace_F();
}

int fn_NetSim_WiMax_BS_Physical_Out()
{
	NetSim_PACKET* struPacket;
	NETSIM_ID DeviceId = pstruEventDetails->nDeviceId;
	NETSIM_ID InterfaceId = pstruEventDetails->nInterfaceId;

	struPacket = pstruEventDetails->pPacket;

	if(struPacket && struPacket->nDestinationId == 0)
	{
		//Broadcast the packet over link
		fn_NetSim_WiMax_BroadCastPacket(struPacket,DeviceId,InterfaceId);
		fn_NetSim_Packet_FreePacket(struPacket);
	}
	else
	{
		fn_NetSim_WiMax_TransmitP2PPacket(struPacket,DeviceId,InterfaceId);
	}
	return 1;
}

int fn_NetSim_WiMax_SS_Physical_Out()
{
	double TransmissionTime;
	WIMAX_SS_MAC* SS_Mac;
	WIMAX_SS_PHY* SS_Phy;
	NetSim_PACKET* Packet;
	NETSIM_ID BS_Id,BS_Interface;
	WIMAX_BS_MAC* BS_Mac;
	WIMAX_BS_PHY* BS_Phy;

	SS_Mac = (WIMAX_SS_MAC* )DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
	SS_Phy = (WIMAX_SS_PHY* )DEVICE_PHYVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
	BS_Id = SS_Mac->BSId;
	BS_Interface = SS_Mac->BSInterface;
	BS_Mac = (WIMAX_BS_MAC* )DEVICE_MACVAR(BS_Id,BS_Interface);
	BS_Phy = (WIMAX_BS_PHY* )DEVICE_PHYVAR(BS_Id,BS_Interface);

	Packet = pstruEventDetails->pPacket;
	//Update the phy layer
	Packet->pstruPhyData->dArrivalTime = pstruEventDetails->dEventTime;
	Packet->pstruPhyData->dPayload = Packet->pstruMacData->dPacketSize;
	Packet->pstruPhyData->dPacketSize = Packet->pstruPhyData->dPayload;
	Packet->pstruPhyData->dStartTime = pstruEventDetails->dEventTime;
	Packet->pstruPhyData->nPhyMedium = PHY_MEDIUM_WIRELESS;
	TransmissionTime = fn_NetSim_WiMax_CalculateTransmissionTime(Packet,BS_Phy);
	Packet->pstruPhyData->dEndTime = pstruEventDetails->dEventTime + TransmissionTime;

	if(Packet->nDestinationId == 0)//Broadcast packet from upper layer transmit to BS
	{
		fn_NetSim_WiMax_TransmitPacket(Packet,pstruEventDetails->nDeviceId,SS_Mac->BSId,SS_Mac->BSInterface);
	}
	else
		fn_NetSim_WiMax_TransmitP2PPacket(Packet,pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);

	return 1;
}

int fn_NetSim_WiMax_Physical_In()
{
	//Call function for metrics
	fn_NetSim_Metrics_Add(pstruEventDetails->pPacket);
	//Call Trace function to write packet trace
	fn_NetSim_WritePacketTrace(pstruEventDetails->pPacket);
	//Add the MAC IN event
	pstruEventDetails->nEventType = MAC_IN_EVENT;
	fnpAddEvent(pstruEventDetails);
	return 1;
}

int fn_NetSim_WiMax_BS_Mac_In()
{
	NETSIM_ID DeviceId,InterfaceId;
	NetSim_PACKET* Packet;
	WIMAX_BS_MAC* BS_Mac;

	//Get the packet
	Packet = pstruEventDetails->pPacket;
	DeviceId = pstruEventDetails->nDeviceId;
	InterfaceId = pstruEventDetails->nInterfaceId;
	BS_Mac = (WIMAX_BS_MAC*)DEVICE_MACVAR(DeviceId,InterfaceId);

	if(Packet->nDestinationId) 
	{
		WIMAX_SS_INFO* info = BS_Mac->ssInfo;
		while(info)
		{
			if(Packet->nDestinationId == info->nSSId)
			{
				fn_NetSim_Packet_AddPacketToList(DEVICE_MAC_NW_INTERFACE(DeviceId,InterfaceId)->pstruAccessBuffer,Packet,0);
				return 1;
			}
			info = (WIMAX_SS_INFO* )LIST_NEXT(info);
		}
	}
	else
	{
		fn_NetSim_Packet_AddPacketToList(DEVICE_MAC_NW_INTERFACE(DeviceId,InterfaceId)->pstruAccessBuffer,Packet,0);
	}
	if(DEVICE(DeviceId)->nNumOfInterface == 2)
	{
		if(!fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(DeviceId,2)->pstruAccessInterface->pstruAccessBuffer))
		{
			//write mac out event
			pstruEventDetails->nEventType=MAC_OUT_EVENT;
			pstruEventDetails->dPacketSize=fnGetPacketSize(Packet);
			pstruEventDetails->nInterfaceId=2;
			pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetMacProtocol(DeviceId,2);
			pstruEventDetails->nSubEventType=0;
			pstruEventDetails->pPacket=NULL;
			fnpAddEvent(pstruEventDetails);
		}
		fn_NetSim_Packet_AddPacketToList(DEVICE_INTERFACE(DeviceId,2)->pstruAccessInterface->pstruAccessBuffer,fn_NetSim_Packet_CopyPacket(Packet),0);
	
	}
	return 1;
}

int fn_NetSim_WiMax_SS_Mac_In()
{
	NetSim_PACKET* Packet = pstruEventDetails->pPacket;

	if(Packet->nControlDataType/100 != MAC_PROTOCOL_IEEE802_16)
	{
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->dPacketSize = Packet->pstruMacData->dPayload;
		pstruEventDetails->nEventType = NETWORK_IN_EVENT;
		fnpAddEvent(pstruEventDetails);
	}
	else
	{
		switch(Packet->nControlDataType%100)
		{
		case MMM_FCH:
			//Drop the packet
			fn_NetSim_Packet_FreePacket(Packet);
			return 1;
			break;
		case MMM_DL_MAP:
			//Drop the packet
			fn_NetSim_Packet_FreePacket(Packet);
			return 1;
			break;
		case MMM_UL_MAP:
			fn_NetSim_Packet_FreePacket(Packet);
			return 1;
			break;
		default:
			printf("WiMax--- Unknown control packet arrive to SS Mac\n");
			break;
		}
	}
	return 1;	
}

int fn_NetSim_WiMax_Mobility(NETSIM_ID nDeviceId)
{
	if(DEVICE_MACLAYER(nDeviceId,1)->nMacProtocolId == MAC_PROTOCOL_IEEE802_16)
	{
		WIMAX_SS_MAC* ssMac=(WIMAX_SS_MAC*)DEVICE_MACVAR(nDeviceId,1);
		WIMAX_SS_PHY* ssPhy=(WIMAX_SS_PHY*)DEVICE_PHYVAR(nDeviceId,1);
		NETSIM_ID bsId=ssMac->BSId;
		NETSIM_ID bsInterface=ssMac->BSInterface;
		WIMAX_BS_PHY* bsPhy=(WIMAX_BS_PHY*)DEVICE_PHYVAR(bsId,bsInterface);
		//Call function to change received power and modulation, coding rate etc.
		fn_NetSim_WiMax_CalculateAndSetReceivedPower(bsId,1,nDeviceId,1);
		fn_NetSim_WiMax_Calculate_SNR(bsId,1,nDeviceId,1);
		fn_NetSim_WiMax_Mod(bsId,1,nDeviceId,1);
		fprintf(stdout,"\nDevice_Id = %d, \tTime = %f, \tRx_power = %f, \tSNR = %f, ",nDeviceId,pstruEventDetails->dEventTime,ssPhy->TotalReceivedPower,ssPhy->SNR);
		switch(bsPhy->Modulation)
		{
		case 3: fprintf(stdout,"\t64-QAM ");
			if(bsPhy->CodingRate == 5)
			{
				fprintf(stdout,"\tCoding 2/3\n" );
			}
			else
				fprintf(stdout,"\tCoding 3/4\n" );
			break;
		case 2:fprintf(stdout,"\t16-QAM");
			if(bsPhy->CodingRate == 3)
			{
				fprintf(stdout,"\tCoding 1/2\n" );
			}
			else
				fprintf(stdout,"\tCoding 3/4\n" );
			break;
		case 1:fprintf(stdout,"\tQPSK");
			if(bsPhy->CodingRate == 1)
			{
				fprintf(stdout,"\tCoding 1/2\n" );
			}
			else
				fprintf(stdout,"\tCoding 3/4\n" );
			break;
		default:
			fprintf(stdout,"\tBPSK Coding 1/2\n");
			break;
		}
		fflush(stdout);
	}
	else
	{
		//Do nothing other mac protocol
	}
	return 1;
}