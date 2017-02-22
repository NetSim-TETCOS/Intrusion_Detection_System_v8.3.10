/************************************************************************************
 * Copyright (C) 2014                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Shashi Kant Suman                                                     *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#include "main.h"
#include "LTE.h"
#include "RLC.h"
unsigned int nRLCSN=0;

int fn_NetSim_LTE_RLC_UE_Out(NETSIM_ID ueId,NETSIM_ID ueInterface,LTE_ASSOCIATEUE_INFO* info)
{
	LTE_UE_MAC* ueMac=DEVICE_MACVAR(ueId,ueInterface);
	//Form GBR sdu
	fn_NetSim_LTE_RLC_FormRLCSDU(ueMac->GBRQueue,ueId,ueMac->nENBId,ueMac->HARQBuffer);
	//Form nonGBR sdu
	fn_NetSim_LTE_RLC_FormRLCSDU(ueMac->nonGBRQueue,ueId,ueMac->nENBId,ueMac->HARQBuffer);
	return 1;
}
int fn_NetSim_LTE_RLC_eNB_In()
{
	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
	NetSim_PACKET* rlcSDU=pstruEventDetails->pPacket;
	RLC_PDU* rlcPdu=rlcSDU->pstruMacData->Packet_MACProtocol;
	NetSim_PACKET* packet[2];
	RLC_HEADER* rlcHdr=rlcPdu->header;
	packet[0]=rlcPdu->pdcpPDU[0];
	packet[1]=rlcPdu->pdcpPDU[1];

	//Generate ack
	fn_NetSim_LTE_SendAck(rlcSDU,pstruEventDetails);

	if(isBitSet(rlcHdr->LSF,2) && packet[0])
	{
		LTE_QUEUE* queue;
		NETSIM_ID nDestinationId=packet[0]->nDestinationId;
		LTE_ASSOCIATEUE_INFO* info= fn_NetSim_LTE_FindInfo(enbMac,nDestinationId);
		//update metrics
		enbMac->metrics.nPacketReceived++;
		enbMac->metrics.dBytesReceived += (double)((int)fnGetPacketSize(packet[0]));
		packet[0]->pstruMacData->dPacketSize=packet[0]->pstruMacData->dOverhead+packet[0]->pstruMacData->dPayload;
		if(info)
		{
			if(info->rrcState==RRC_IDLE)
				fn_NetSim_Init_PagingRequest(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,info);
			if(packet[0]->nQOS==QOS_UGS)
				queue=info->GBRQueue;
			else
				queue=info->nonGBRQueue;
			queue->nSize+=(unsigned int)fnGetPacketSize(packet[0]);
			if(queue->head)
			{
				queue->tail->pstruNextPacket=packet[0];
				queue->tail=packet[0];
			}
			else
			{
				queue->head=packet[0];
				queue->tail=packet[0];
			}
		}
		else
		{
			//transmit to MME
			fn_NetSim_LTE_PDCP_hdrUncompression(packet[0]);
			fn_NetSim_LTE_PDCP_RemoveHdr(packet[0]);
			//Add packet to second interface
			if(!fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,2)->pstruAccessInterface->pstruAccessBuffer))
			{
				//write mac out event
				pstruEventDetails->nEventType=MAC_OUT_EVENT;
				pstruEventDetails->dPacketSize=fnGetPacketSize(packet[0]);
				pstruEventDetails->nInterfaceId=2;
				pstruEventDetails->nPacketId=packet[0]->nPacketId;
				pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetMacProtocol(pstruEventDetails->nDeviceId,2);
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=NULL;
				fnpAddEvent(pstruEventDetails);
			}
			packet[0]->nTransmitterId=pstruEventDetails->nDeviceId;
			fn_NetSim_Packet_AddPacketToList(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,2)->pstruAccessInterface->pstruAccessBuffer,packet[0],0);
		}
	}
	else if(packet[0])
	{
		enbMac->metrics.dBytesReceived += (double)((int)fnGetPacketSize(packet[0]));
		fn_NetSim_Packet_FreePacket(packet[0]);
	}
	if(isBitSet(rlcHdr->LSF,1) && packet[1])
	{
		LTE_QUEUE* queue;
		NETSIM_ID nDestinationId=packet[1]->nDestinationId;
		LTE_ASSOCIATEUE_INFO* info= fn_NetSim_LTE_FindInfo(enbMac,nDestinationId);
		//update metrics
		enbMac->metrics.nPacketReceived++;
		enbMac->metrics.dBytesReceived += (double)((int)fnGetPacketSize(packet[1]));
		packet[1]->pstruMacData->dPacketSize=packet[1]->pstruMacData->dOverhead+packet[1]->pstruMacData->dPayload;
		if(info)
		{
			if(info->rrcState==RRC_IDLE)
				fn_NetSim_Init_PagingRequest(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,info);
			if(packet[1]->nQOS==QOS_UGS)
				queue=info->GBRQueue;
			else
				queue=info->nonGBRQueue;
			queue->nSize+=(unsigned int)fnGetPacketSize(packet[1]);
			if(queue->head)
			{
				queue->tail->pstruNextPacket=packet[1];
				queue->tail=packet[1];
			}
			else
			{
				queue->head=packet[1];
				queue->tail=packet[1];
			}
		}
		else
		{
			//transmit to MME
			fn_NetSim_LTE_PDCP_hdrUncompression(packet[1]);
			fn_NetSim_LTE_PDCP_RemoveHdr(packet[1]);
			//Add packet to second interface
			if(!fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,2)->pstruAccessInterface->pstruAccessBuffer))
			{
				//write mac out event
				pstruEventDetails->nEventType=MAC_OUT_EVENT;
				pstruEventDetails->dPacketSize=fnGetPacketSize(packet[1]);
				pstruEventDetails->nInterfaceId=2;
				pstruEventDetails->nPacketId=packet[1]->nPacketId;
				pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetMacProtocol(pstruEventDetails->nDeviceId,2);
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=NULL;
				fnpAddEvent(pstruEventDetails);
			}
			packet[1]->nTransmitterId=pstruEventDetails->nDeviceId;
			fn_NetSim_Packet_AddPacketToList(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,2)->pstruAccessInterface->pstruAccessBuffer,packet[1],0);
		}
	}
	else if(packet[1])
	{
		enbMac->metrics.dBytesReceived += (double)((int)fnGetPacketSize(packet[1]));
		fn_NetSim_Packet_FreePacket(packet[1]);
	}
	fn_NetSim_Packet_FreePacket(rlcSDU);
	return 1;
}

int fn_NetSim_LTE_RLC_eNB_Out(NETSIM_ID enbId,NETSIM_ID enbInterface,LTE_ASSOCIATEUE_INFO* info)
{
	//Form GBR sdu
	fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId,info->HARQBuffer);
	//form non GBR sdu
	fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId,info->HARQBuffer);
	return 1;
}
int fn_NetSim_LTE_RLC_UE_In()
{
	NETSIM_ID ueId=pstruEventDetails->nDeviceId;
	NETSIM_ID ueInterface=pstruEventDetails->nInterfaceId;
	LTE_UE_MAC* ueMac=DEVICE_MACVAR(ueId,ueInterface);
	NetSim_PACKET* rlcSDU=pstruEventDetails->pPacket;
	RLC_PDU* rlcPdu=rlcSDU->pstruMacData->Packet_MACProtocol;
	NetSim_PACKET* packet[2];
	RLC_HEADER* rlcHdr=rlcPdu->header;

	packet[0]=rlcPdu->pdcpPDU[0];
	packet[1]=rlcPdu->pdcpPDU[1];
	
	//Generate ack
	fn_NetSim_LTE_SendAck(rlcSDU,pstruEventDetails);

	if(isBitSet(rlcHdr->LSF,2) && packet[0])
	{
		//update metrics
		ueMac->metrics.nPacketReceived++;
		ueMac->metrics.dBytesReceived += (int)fnGetPacketSize(packet[0]);
		//Call PDCP
		fn_NetSim_LTE_PDCP_hdrUncompression(packet[0]);
		//add network in event
		pstruEventDetails->dPacketSize=fnGetPacketSize(packet[0]);
		if(packet[0]->pstruAppData)
		{
			pstruEventDetails->nApplicationId=packet[0]->pstruAppData->nApplicationId;
			pstruEventDetails->nSegmentId=packet[0]->pstruAppData->nSegmentId;
		}
		else
		{
			pstruEventDetails->nApplicationId=0;
			pstruEventDetails->nSegmentId=0;
		}
		pstruEventDetails->nEventType=NETWORK_IN_EVENT;
		pstruEventDetails->nPacketId=packet[0]->nPacketId;
		pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetNWProtocol(ueId);
		pstruEventDetails->nSubEventType=0;
		pstruEventDetails->pPacket=packet[0];
		fnpAddEvent(pstruEventDetails);
	}
	else if(packet[0])
	{
		ueMac->metrics.dBytesReceived += (int)fnGetPacketSize(packet[0]);
	}
	if(isBitSet(rlcHdr->LSF,1) && packet[1])
	{
		//update metrics
		ueMac->metrics.nPacketReceived++;
		ueMac->metrics.dBytesReceived += (int)fnGetPacketSize(packet[1]);
		//Call PDCP
		fn_NetSim_LTE_PDCP_hdrUncompression(packet[1]);
		//add network in event
		pstruEventDetails->dPacketSize=fnGetPacketSize(packet[1]);
		if(packet[1]->pstruAppData)
		{
			pstruEventDetails->nApplicationId=packet[1]->pstruAppData->nApplicationId;
			pstruEventDetails->nSegmentId=packet[1]->pstruAppData->nSegmentId;
		}
		else
		{
			pstruEventDetails->nApplicationId=0;
			pstruEventDetails->nSegmentId=0;
		}
		pstruEventDetails->nEventType=NETWORK_IN_EVENT;
		pstruEventDetails->nPacketId=packet[1]->nPacketId;
		pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetNWProtocol(ueId);
		pstruEventDetails->nSubEventType=0;
		pstruEventDetails->pPacket=packet[1];
		fnpAddEvent(pstruEventDetails);
	}
	else if(packet[1])
	{
		ueMac->metrics.dBytesReceived += (int)fnGetPacketSize(packet[1]);
	}
	fn_NetSim_Packet_FreePacket(rlcSDU);
	return 1;
}
int fn_NetSim_LTE_RLC_FormRLCSDU(LTE_QUEUE* queue,NETSIM_ID nSourceId,NETSIM_ID nDestinationId,NetSim_PACKET** HARQBuffer)
{
	//LTE metrics
	LTE_METRICS* metrics;
	if(DEVICE_TYPE(nSourceId)==UE)
		metrics=&((LTE_UE_MAC*)DEVICE_MACVAR(nSourceId,1))->metrics;
	else
		metrics=&((LTE_ENB_MAC*)DEVICE_MACVAR(nSourceId,1))->metrics;

	if(queue->nSize && queue->nRBLength)
	{
		unsigned int nBitSize=queue->nBitcount/8;
		while(nBitSize >= RLC_HEADER_SIZE && queue->nSize)
		{
			NetSim_PACKET* packet=fn_NetSim_Packet_CreatePacket(MAC_LAYER);
			RLC_PDU* rlc=calloc(1,sizeof* rlc);
			unsigned int size[2]={0,0};
			NetSim_PACKET* pdu[2]={NULL,NULL};
			nBitSize-=RLC_HEADER_SIZE;//rlc hdr
			packet->dEventTime=pstruEventDetails->dEventTime;
			packet->nDestinationId=nDestinationId;
			packet->nPacketType=PacketType_Control;
			packet->nControlDataType=LTEPacket_RLC_SDU;
			packet->nReceiverId=nDestinationId;
			packet->nSourceId=nSourceId;
			packet->nTransmitterId=nSourceId;
			packet->pstruMacData->dArrivalTime=pstruEventDetails->dEventTime;
			packet->pstruMacData->dEndTime=pstruEventDetails->dEventTime;
			packet->pstruMacData->dStartTime=pstruEventDetails->dEventTime;
			packet->pstruMacData->nMACProtocol=MAC_PROTOCOL_LTE;
			packet->pstruMacData->dOverhead=RLC_HEADER_SIZE;
			rlc->header=calloc(1,sizeof* rlc->header);
			rlc->header->SN=nRLCSN++;

			packet->nPacketId=rlc->header->SN;
		
			
			packet->pstruMacData->Packet_MACProtocol=rlc;
			pdu[0]=queue->head;
			size[0]=(unsigned int)fnGetPacketSize(pdu[0]);
			if(size[0]<=nBitSize)
			{
				nBitSize-=size[0];
				queue->head=queue->head->pstruNextPacket;
				if(queue->head==NULL)
					queue->tail=NULL;
				rlc->header->LSF=setBit(rlc->header->LSF,2);
				//update metrics
				metrics->nPacketTransmitted++;
				metrics->dBytesTransmitted+=size[0];
			}
			else
			{
				size[0]=nBitSize;
				nBitSize=0;
				queue->head->pstruMacData->dPacketSize-=size[0];
				rlc->header->FI=setBit(rlc->header->FI,2);
				pdu[0]=fn_NetSim_Packet_CopyPacket(pdu[0]);
				pdu[0]->pstruMacData->dPacketSize=size[0];
				//update metrics
				metrics->dBytesTransmitted+=size[0];
				
			}
			pdu[0]->pstruNextPacket=NULL;
			queue->nSize-=size[0];
			packet->pstruMacData->dPayload += (double)size[0];
			rlc->header->LI1=size[0];
			rlc->pdcpPDU[0]=pdu[0];

			if(nBitSize && queue->head)
			{
				pdu[1]=queue->head;
				size[1]=(unsigned int)fnGetPacketSize(pdu[1]);
				if(size[1]<=nBitSize)
				{
					nBitSize-=size[1];
					queue->head=queue->head->pstruNextPacket;
					if(queue->head==NULL)
						queue->tail=NULL;
					rlc->header->LSF=setBit(rlc->header->LSF,1);
					//update metrics
					metrics->nPacketTransmitted++;
					metrics->dBytesTransmitted+=size[1];
					
				}
				else
				{
					size[1]=nBitSize;
					nBitSize=0;
					queue->head->pstruMacData->dPacketSize-=size[1];
					rlc->header->FI=setBit(rlc->header->FI,1);
					pdu[1]=fn_NetSim_Packet_CopyPacket(pdu[1]);
					pdu[1]->pstruMacData->dPacketSize=size[1];
					metrics->dBytesTransmitted+=size[1];
				}
				pdu[1]->pstruNextPacket=NULL;
				queue->nSize-=size[1];
				packet->pstruMacData->dPayload += (double)size[1];
				rlc->header->LI2=size[1];
				rlc->pdcpPDU[1]=pdu[1];
			}
			packet->pstruMacData->dPacketSize=packet->pstruMacData->dOverhead+packet->pstruMacData->dPayload;
			//Add physical out event
			pstruEventDetails->dPacketSize=fnGetPacketSize(packet);
			pstruEventDetails->nApplicationId=0;
			pstruEventDetails->nDeviceId=nSourceId;
			pstruEventDetails->nDeviceType=DEVICE_TYPE(nSourceId);
			pstruEventDetails->nEventType=PHYSICAL_OUT_EVENT;
			pstruEventDetails->nInterfaceId=1;
			pstruEventDetails->nPacketId=packet->nPacketId;
			pstruEventDetails->nProtocolId=MAC_PROTOCOL_LTE;
			pstruEventDetails->nSegmentId=0;
			pstruEventDetails->nSubEventType=0;
			pstruEventDetails->pPacket=packet;
			fnpAddEvent(pstruEventDetails);
			
			//Add to HARQ buffer
			fn_NetSim_LTE_AddToHARQBuffer(HARQBuffer,packet,pstruEventDetails->dEventTime);
		}
	}
	return 1;
}
