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
#include <stdio.h>
#pragma comment(lib,"List.lib")

int fn_NetSim_LTE_InitQueue(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)
{
	switch(NETWORK->ppstruDeviceList[nDeviceId-1]->nDeviceType)
	{
	case UE:
		{
			LTE_UE_MAC* UEMac=(LTE_UE_MAC*)DEVICE_MACVAR(nDeviceId,nInterfaceId);
			if(!UEMac)
			{
				UEMac=(LTE_UE_MAC*)calloc(1,sizeof* UEMac);
				DEVICE_MACVAR(nDeviceId,nInterfaceId)=UEMac;
			}
			UEMac->GBRQueue=(LTE_QUEUE*)calloc(1,sizeof* UEMac->GBRQueue);
			UEMac->nonGBRQueue=(LTE_QUEUE*)calloc(1,sizeof* UEMac->nonGBRQueue);
			
		}
		break;
	default:
		fnNetSimError("Unknown device type for LTE-InitQueue");
		break;
	}
	return 1;
}
int fn_NetSim_LTE_AddInQueue()
{
	while(fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)->pstruAccessInterface->pstruAccessBuffer))
	{
		NetSim_PACKET* packet=fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)->pstruAccessInterface->pstruAccessBuffer,1);
		LTE_QUEUE* queue;
		LTE_UE_MAC* UEMac=(LTE_UE_MAC*)DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
		
		//RRC protocol
		if(UEMac->rrcState==RRC_IDLE)
			fn_NetSim_LTE_InitRRCConnection();

		//Call pdcp protocol
		fn_NetSim_LTE_PDCP_init(packet);

		if(packet->nQOS==QOS_UGS)
			queue=UEMac->GBRQueue;
		else
			queue=UEMac->nonGBRQueue;
		queue->nSize+=(unsigned int)fnGetPacketSize(packet);
		if(queue->head)
		{
			queue->tail->pstruNextPacket=packet;
			queue->tail=packet;
		}
		else
		{
			queue->head=packet;
			queue->tail=packet;
		}
	}
	return 1;
}
int fn_NetSim_LTE_eNB_AddInQueue()
{
	while(fn_NetSim_GetBufferStatus(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)->pstruAccessInterface->pstruAccessBuffer))
	{
		NetSim_PACKET* packet=fn_NetSim_Packet_GetPacketFromBuffer(DEVICE_INTERFACE(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)->pstruAccessInterface->pstruAccessBuffer,1);
		LTE_QUEUE* queue;
		LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
		LTE_ASSOCIATEUE_INFO* info=fn_NetSim_LTE_FindInfo(enbMac,packet->nDestinationId);
		packet->pstruPhyData->dPayload=0;
		packet->pstruPhyData->dOverhead=0;
		packet->pstruPhyData->dPacketSize=0;
		if(packet->nDestinationId==0)//Broadcast packet
			return fn_NetSim_Packet_FreePacket(packet);
		if(!info)
		{
			fnNetSimError("unknown destined packet reaches to mac out of LTE-eNB");
			fn_NetSim_Packet_FreePacket(packet);
			return 0;
		}
		//RRC protocol
		if(info->rrcState==RRC_IDLE)
			fn_NetSim_Init_PagingRequest(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,info);

		//Call pdcp protocol
		fn_NetSim_LTE_PDCP_init(packet);

		if(packet->nQOS==QOS_UGS)
			queue=info->GBRQueue;
		else
			queue=info->nonGBRQueue;
		queue->nSize+=(unsigned int)fnGetPacketSize(packet);
		if(queue->head)
		{
			queue->tail->pstruNextPacket=packet;
			queue->tail=packet;
		}
		else
		{
			queue->head=packet;
			queue->tail=packet;
		}
	}
	return 1;
}
int fn_NetSim_LTE_FormNextFrame()
{
	NETSIM_ID enbId=pstruEventDetails->nDeviceId;
	NETSIM_ID enbInterface=pstruEventDetails->nInterfaceId;
	LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(enbId,enbInterface);
	LTE_ENB_PHY* enbPhy=(LTE_ENB_PHY*)DEVICE_PHYVAR(enbId,enbInterface);
	//Add next event
	pstruEventDetails->dEventTime+=enbPhy->dSubFrameDuration*MILLISECOND;
	fnpAddEvent(pstruEventDetails);
	pstruEventDetails->dEventTime-=enbPhy->dSubFrameDuration*MILLISECOND;
	enbMac->nAllocatedRBG=0;
	
	
	fn_NetSim_LTE_FormUPlinkFrame(enbId,enbInterface);
	fn_NetSim_LTE_FormDownlinkFrame(enbId,enbInterface);
	return 1;
}
int fn_NetSim_LTE_FormUPlinkFrame(NETSIM_ID enbId,NETSIM_ID enbInterface)
{
	LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(enbId,enbInterface);
	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
	//Form uplink queue
	//Form GBR queue
	enbMac->nAllocatedRBG=0;
	while(info)
	{
		fn_NetSim_LTE_HARQ_RetransmitHARQBuffer_UL(info,enbMac,enbId);
		info=(LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}
	info=enbMac->associatedUEInfo;
	while(info)
	{
		NETSIM_ID ueId=info->nUEId;
		NETSIM_ID ueInterface=info->nUEInterface;
		LTE_UE_MAC* ueMac=(LTE_UE_MAC*)DEVICE_MACVAR(ueId,ueInterface);
		if(ueMac->rrcState==RRC_CONNECTED && ueMac->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=ueMac->GBRQueue->nSize;
			
			unsigned int nTBSIndex_UL=info->ULInfo.TBSIndex;
			unsigned int nRBGcount,flag=0;
			size=size*8;
			//calculate the RB count required
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.UL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->ULInfo.modulation== Modulation_64QAM)) 
					switch (info->ULInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_UL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_UL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_UL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_UL][index])
							{
								flag=1;
								break;
							}
				if(flag==1)
					break;
				index++;
				
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			ueMac->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
			ueMac->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.UL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->ULInfo.modulation== Modulation_64QAM)) 
			switch (info->ULInfo.Rank)
					{
						case 2: ueMac->GBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_UL][ueMac->GBRQueue->nRBLength-1];
								break;
						case 3: ueMac->GBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_UL][ueMac->GBRQueue->nRBLength-1];
								break;
						case 4: ueMac->GBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_UL][ueMac->GBRQueue->nRBLength-1];
								break;
					}
			else  ueMac->GBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_UL][ueMac->GBRQueue->nRBLength-1];

	//		ueMac->GBRQueue->nBitcount=TransportBlockSize[nTBSIndex_UL][ueMac->GBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
		}
		if(ueMac->rrcState==RRC_CONNECTED)
		{
			 fn_NetSim_LTE_RLC_FormRLCSDU(ueMac->GBRQueue,ueId,ueMac->nENBId,ueMac->HARQBuffer);
		} 
		info=(LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}        
	//Form non GBR queue
	info=enbMac->associatedUEInfo;
	while(info)
	{
		NETSIM_ID ueId=info->nUEId;
		NETSIM_ID ueInterface=info->nUEInterface;
		LTE_UE_MAC* ueMac=(LTE_UE_MAC*)DEVICE_MACVAR(ueId,ueInterface);
		if(ueMac->rrcState==RRC_CONNECTED && ueMac->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=ueMac->nonGBRQueue->nSize;
			unsigned int nTBSIndex_UL=info->ULInfo.TBSIndex;
			unsigned int nRBGcount,flag=0;
			//if ((MODE_INDEX_MAPPING[info->TransmissionIndex.UL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->ULInfo.modulation == Modulation_64QAM))
			//	size = size/info->ULInfo.Rank;
			size=size*8;
			//calculate the RB count required
			/*while(index<LTE_NRB_MAX)
			{
				if(size<TransportBlockSize[nTBSIndex_UL][index])
					break;
				index++;
			}*/

			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.UL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->ULInfo.modulation== Modulation_64QAM)) 
					switch (info->ULInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_UL][index])
									flag=1;
									break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_UL][index])
									flag=1;
									break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_UL][index])
									flag=1;
									break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_UL][index])
							{
								flag=1;
								break;
							}
				if(flag==1)
					break;
				index++;
				
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			ueMac->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
			ueMac->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.UL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->ULInfo.modulation== Modulation_64QAM))
			{
				switch (info->ULInfo.Rank)
				{
				case 2: ueMac->nonGBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_UL][ueMac->nonGBRQueue->nRBLength-1];
					break;
				case 3: ueMac->nonGBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_UL][ueMac->nonGBRQueue->nRBLength-1];
					break;
				case 4: ueMac->nonGBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_UL][ueMac->nonGBRQueue->nRBLength-1];
					break;
				}
			}
			else 
				ueMac->nonGBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_UL][ueMac->nonGBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
		}
		if(ueMac->rrcState==RRC_CONNECTED)
		{
			fn_NetSim_LTE_RLC_FormRLCSDU(ueMac->nonGBRQueue,ueId,ueMac->nENBId,ueMac->HARQBuffer);
		}
		info=(LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}
	return 1;
}

int CompareGBR(LTE_ASSOCIATEUE_INFO* info1,LTE_ASSOCIATEUE_INFO* info2)
{
	if(info1->GBRQueue->dRatio < info2->GBRQueue->dRatio)
		return 1;
	else
		return 0;
}


int CompareNonGBR(LTE_ASSOCIATEUE_INFO* info1,LTE_ASSOCIATEUE_INFO* info2)
{
	if(info1->nonGBRQueue->dRatio < info2->nonGBRQueue->dRatio)
		return 1;
	else
		return 0;
}

int fn_NetSim_LTE_FormDownlinkFrame_RoundRobin(NETSIM_ID enbId,NETSIM_ID enbInterface)
{
	LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(enbId,enbInterface);
	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
	
	unsigned int Required_Rate;
	unsigned int temp_AllocatedRBG;
	unsigned int nRBGcount;

	enbMac->nAllocatedRBG=0;
	//Send HARQ buffer
	while(info)
	{
		fn_NetSim_LTE_HARQ_RetransmitHARQBuffer_DL(info,enbMac,enbId);
		info=LIST_NEXT(info);
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
	info=enbMac->associatedUEInfo;

//GBR queue
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];	
			
			if(info->GBRQueue->dPrev_throughput == 0)
				info->GBRQueue->dPrev_throughput = 1;
			info->GBRQueue->dRatio = 1/(double)info->GBRQueue->dPrev_throughput;
		}
	//	info->GBRQueue->dRatio  = (double)rand()/(double)RAND_MAX;
		info = (LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);	
	}
	info = enbMac->associatedUEInfo;

	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareGBR);

	info = enbMac->associatedUEInfo;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->GBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 3: info->GBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 4: info->GBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
					}
			else  info->GBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->GBRQueue->nRBLength)
				info->GBRQueue->dPrev_throughput = 0.99*info->GBRQueue->nBitcount + 0.01*info->GBRQueue->dPrev_throughput;
			else 
				info->GBRQueue->dPrev_throughput = 1 + 0.01*info->GBRQueue->dPrev_throughput;
		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId,info->HARQBuffer);	
		info = LIST_NEXT(info);	
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
//non GBR queue
	info=enbMac->associatedUEInfo;
	
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];	
			if(info->nonGBRQueue->dPrev_throughput == 0)
				info->nonGBRQueue->dPrev_throughput = 1;
			info->nonGBRQueue->dRatio = 1/info->nonGBRQueue->dPrev_throughput;
		}
		info = LIST_NEXT(info);
	}
	info = enbMac->associatedUEInfo;
	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareNonGBR);
	info=enbMac->associatedUEInfo;
	enbMac->nAllocatedRBG = temp_AllocatedRBG;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->nonGBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 3: info->nonGBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 4: info->nonGBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
					}
			else  info->nonGBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];

			//	info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->nonGBRQueue->nRBLength)
				info->nonGBRQueue->dPrev_throughput = 0.99*info->nonGBRQueue->nBitcount + 0.01*info->nonGBRQueue->dPrev_throughput;
			else
				info->nonGBRQueue->dPrev_throughput = 1 + 0.01*info->nonGBRQueue->dPrev_throughput;

		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId,info->HARQBuffer);
		info = LIST_NEXT(info);
	}
	return 1;
}

int fn_NetSim_LTE_FormDownlinkFrame_MaxCQI(NETSIM_ID enbId,NETSIM_ID enbInterface)
{
	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(enbId,enbInterface);
	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
	
	unsigned int Required_Rate=0;
	unsigned int temp_AllocatedRBG;
	unsigned int nRBGcount;

	enbMac->nAllocatedRBG=0;
	//Send HARQ buffer
	while(info)
	{
		fn_NetSim_LTE_HARQ_RetransmitHARQBuffer_DL(info,enbMac,enbId);
		info=LIST_NEXT(info);
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
	info=enbMac->associatedUEInfo;

//GBR queue
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];	
			
			if(info->GBRQueue->dPrev_throughput == 0)
				info->GBRQueue->dPrev_throughput = 1;
			info->GBRQueue->dRatio = (double)Required_Rate;
		}
		info = (LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);	
	}
	info = enbMac->associatedUEInfo;
	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareGBR);

	info = enbMac->associatedUEInfo;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->GBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 3: info->GBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 4: info->GBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
					}
			else  info->GBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->GBRQueue->nRBLength)
				info->GBRQueue->dPrev_throughput = 0.99*info->GBRQueue->nBitcount + 0.01*info->GBRQueue->dPrev_throughput;
			else 
				info->GBRQueue->dPrev_throughput = 1 + 0.01*info->GBRQueue->dPrev_throughput;
		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId,info->HARQBuffer);	
		info = LIST_NEXT(info);	
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
//non GBR queue
	info=enbMac->associatedUEInfo;
	
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];	
			if(info->nonGBRQueue->dPrev_throughput == 0)
				info->nonGBRQueue->dPrev_throughput = 1;
			info->nonGBRQueue->dRatio = Required_Rate;
		}
		info = (LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}
	info = enbMac->associatedUEInfo;
	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareNonGBR);
	info=enbMac->associatedUEInfo;
	enbMac->nAllocatedRBG = temp_AllocatedRBG;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->nonGBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 3: info->nonGBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 4: info->nonGBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
					}
			else  info->nonGBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];

			//	info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->nonGBRQueue->nRBLength)
				info->nonGBRQueue->dPrev_throughput = 0.99*info->nonGBRQueue->nBitcount + 0.01*info->nonGBRQueue->dPrev_throughput;
			else
				info->nonGBRQueue->dPrev_throughput = 1 + 0.01*info->nonGBRQueue->dPrev_throughput;

		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId,info->HARQBuffer);
		info = LIST_NEXT(info);
	}
	return 1;
}

int fn_NetSim_LTE_FormDownlinkFrame_ProportionalFairScheduling(NETSIM_ID enbId,NETSIM_ID enbInterface)
{
	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(enbId,enbInterface);
	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
	
	unsigned int Required_Rate=0;
	unsigned int temp_AllocatedRBG;
	unsigned int nRBGcount;

	enbMac->nAllocatedRBG=0;
	//Send HARQ buffer
	while(info)
	{
		fn_NetSim_LTE_HARQ_RetransmitHARQBuffer_DL(info,enbMac,enbId);
		info=LIST_NEXT(info);
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
	info=enbMac->associatedUEInfo;

//GBR queue
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];	
			
			if(info->GBRQueue->dPrev_throughput == 0)
				info->GBRQueue->dPrev_throughput = 1;
			info->GBRQueue->dRatio = (double)Required_Rate/(double)info->GBRQueue->dPrev_throughput;
		}
	//	info->GBRQueue->dRatio  = (double)rand()/(double)RAND_MAX;
		info = (LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);	
	}
	info = enbMac->associatedUEInfo;
	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareGBR);

	info = enbMac->associatedUEInfo;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->GBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->GBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 3: info->GBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
						case 4: info->GBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
								break;
					}
			else  info->GBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->GBRQueue->nRBLength)
				info->GBRQueue->dPrev_throughput = 0.99*info->GBRQueue->nBitcount + 0.01*info->GBRQueue->dPrev_throughput;
			else 
				info->GBRQueue->dPrev_throughput = 1 + 0.01*info->GBRQueue->dPrev_throughput;
		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId,info->HARQBuffer);	
		info = LIST_NEXT(info);	
	}
	temp_AllocatedRBG = enbMac->nAllocatedRBG;
	enbMac->nAllocatedRBG = 0;
//non GBR queue
	info=enbMac->associatedUEInfo;
	
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
				{
					case 2:Required_Rate= TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 3:Required_Rate= TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
					case 4:Required_Rate= TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
							break;
				}
			else Required_Rate= TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];	
			if(info->nonGBRQueue->dPrev_throughput == 0)
				info->nonGBRQueue->dPrev_throughput = 1;
			info->nonGBRQueue->dRatio = (double)Required_Rate/info->nonGBRQueue->dPrev_throughput;
		}
		info = (LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}
	info = enbMac->associatedUEInfo;
	if(info)
		list_sort((void**)&enbMac->associatedUEInfo,enbMac->associatedUEInfo->ele->offset,CompareNonGBR);
	info = enbMac->associatedUEInfo;
	enbMac->nAllocatedRBG = temp_AllocatedRBG;
	while(info)
	{
		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
		{
			unsigned int index=0;
			unsigned int size=info->nonGBRQueue->nSize;
			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
			unsigned int flag=0;
			size = size*8;
			while(index<LTE_NRB_MAX)
			{
				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
					switch (info->DLInfo.Rank)
					{
						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
								flag=1;
								break;
						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
								flag=1;
								break;
						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
								flag=1;
								break;
					}
				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
						{
							flag=1;
							break;
						}
				if(flag==1)
				break;
				index++;
			}
			if(index>=LTE_NRB_MAX)
				index=LTE_NRB_MAX-1;
			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
			info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
				switch (info->DLInfo.Rank)
					{
						case 2: info->nonGBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 3: info->nonGBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
						case 4: info->nonGBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
								break;
					}
			else  info->nonGBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];

			//	info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
			enbMac->nAllocatedRBG+=nRBGcount;
			if(info->nonGBRQueue->nRBLength)
				info->nonGBRQueue->dPrev_throughput = 0.9*(double)info->nonGBRQueue->nBitcount + 0.1*info->nonGBRQueue->dPrev_throughput;
			else
				info->nonGBRQueue->dPrev_throughput = 1 + 0.1*info->nonGBRQueue->dPrev_throughput;

		}
		if(info->rrcState==RRC_CONNECTED)
			fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId,info->HARQBuffer);
		info = LIST_NEXT(info);
	}
	return 0;
}


int fn_NetSim_LTE_FormDownlinkFrame(NETSIM_ID enbId,NETSIM_ID enbInterface)
{
	LTE_ENB_MAC* enbMac = DEVICE_MACVAR(enbId,enbInterface);
	LTE_ASSOCIATEUE_INFO* info;
	unsigned int Scheduling = 1;
	if(strcmp(enbMac->sScheduling,"ROUND_ROBIN")==0)
		Scheduling = RoundRobin;
	else if (strcmp(enbMac->sScheduling,"PROPORTIONAL_FAIR")==0)
		Scheduling = ProportionalFairScheduling;
	else if (strcmp(enbMac->sScheduling,"MAX_THROUGHPUT")==0)
		Scheduling = MaxCQI;
	switch(Scheduling)
	{
		case (RoundRobin):					fn_NetSim_LTE_FormDownlinkFrame_RoundRobin(enbId,enbInterface);
											break;
		case (ProportionalFairScheduling):	fn_NetSim_LTE_FormDownlinkFrame_ProportionalFairScheduling(enbId,enbInterface);
											break;
		case (MaxCQI):						fn_NetSim_LTE_FormDownlinkFrame_MaxCQI(enbId,enbInterface);
											break;		
	}
	//Print to graph
	info=enbMac->associatedUEInfo;
	while(info)
	{
		//add to graph
		if(nLTEGraphFlag && nLTEGraphFlagList[info->nUEId-1])
		{
			double total=0;
			if(info->GBRQueue->nRBLength)
				total += (double)(info->GBRQueue->nBitcount);
			if(info->nonGBRQueue->nRBLength)
				total += (double)(info->nonGBRQueue->nBitcount);
			total=total/1000.0;//mbps
			fprintf(genericLTEGraph[info->nUEId-1]->dataFile,"%d %0.2lf\n",(int)(pstruEventDetails->dEventTime/1000),total);
			fflush(genericLTEGraph[info->nUEId-1]->dataFile);
			genericLTEGraph[info->nUEId-1]->fnCallreplot(genericLTEGraph[info->nUEId-1]);
		}
		info=(LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
	}
	return 1;
}
//int  fn_NetSim_LTE_FormDownlinkFrame_PriorityFairScheduling(enbId,enbInterface)
//{
//	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(enbId,enbInterface);
//	LTE_ENB_PHY* enbPhy=DEVICE_PHYVAR(enbId,enbInterface);
//	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
//	unsigned int i;
//	//Form downlink queue
//	//Form GBR queue
//	enbMac->nAllocatedRBG=0;
//	
//	while(info)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
//		{
//			unsigned int index=0;
//			unsigned int size=info->GBRQueue->nSize;
//			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//			unsigned int nRBGcount;
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				size = size/info->DLInfo.Rank;
//			size=size*8;
//			//calculate the RB count required
//			while(index<LTE_NRB_MAX)
//			{
//				if(size<TransportBlockSize[nTBSIndex_DL][index])
//					break;
//				index++;
//			}
//			if(index>=LTE_NRB_MAX)
//				index=LTE_NRB_MAX-1;
//			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//			info->GBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//			enbMac->nAllocatedRBG+=nRBGcount;
//		}
//		if(info->rrcState==RRC_CONNECTED)
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				for(i=0;i<info->DLInfo.Rank;i++)
//					fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId);
//			else fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId);
//		info=LIST_NEXT(info);
//	}
////	Form non GBR queue
//	info=enbMac->associatedUEInfo;
//	
//	while(info)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
//			{
//				unsigned int index=0;
//				unsigned int size=info->nonGBRQueue->nSize;
//				unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//				unsigned int nRBGcount;
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//					size = size/info->DLInfo.Rank;
//				size=size*8;
//				//calculate the RB count required
//				while(index<LTE_NRB_MAX)
//				{
//					if(size<TransportBlockSize[nTBSIndex_DL][index])
//						break;
//					index++;
//				}
//				if(index>=LTE_NRB_MAX)
//					index=LTE_NRB_MAX-1;
//				nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//				if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//					nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//				info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//				enbMac->nAllocatedRBG+=nRBGcount;	
//			}
//			if(info->rrcState==RRC_CONNECTED)
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//					for(i=0;i<info->DLInfo.Rank;i++)
//						fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId);
//				else 
//						fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId);
//		info=LIST_NEXT(info);
//	}
//
//	//Print to graph
//	info=enbMac->associatedUEInfo;
//	while(info)
//	{
//		//add to graph
//		if(nLTEGraphFlag && nLTEGraphFlagList[info->nUEId-1])
//		{
//			double total=(double)(info->GBRQueue->nBitcount+info->nonGBRQueue->nBitcount);
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))
//				total*=info->DLInfo.Rank;
//			total=total/1000.0;//mbps
//			fprintf(genericLTEGraph[info->nUEId-1]->dataFile,"%d %0.2lf\n",(int)(pstruEventDetails->dEventTime/1000),total);
//			fflush(genericLTEGraph[info->nUEId-1]->dataFile);
//			genericLTEGraph[info->nUEId-1]->fnCallreplot(genericLTEGraph[info->nUEId-1]);
//		}
//		info=LIST_NEXT(info);
//	}
//	return 1;
//}
//

//int fn_NetSim_LTE_FormDownlinkFrame(NETSIM_ID enbId,NETSIM_ID enbInterface)
//{
//	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(enbId,enbInterface);
//	LTE_ENB_PHY* enbPhy=DEVICE_PHYVAR(enbId,enbInterface);
//	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
//	unsigned int i;
//	//Form downlink queue
//	//Form GBR queue
//	enbMac->nAllocatedRBG=0;
//	while(info)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
//		{
//			unsigned int index=0;
//			unsigned int size=info->GBRQueue->nSize;
//			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//			unsigned int nRBGcount;
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				size = size/info->DLInfo.Rank;
//			size=size*8;
//			//calculate the RB count required
//			while(index<LTE_NRB_MAX)
//			{
//				if(size<TransportBlockSize[nTBSIndex_DL][index])
//					break;
//				index++;
//			}
//			if(index>=LTE_NRB_MAX)
//				index=LTE_NRB_MAX-1;
//			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//			info->GBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//			enbMac->nAllocatedRBG+=nRBGcount;
//		}
//		if(info->rrcState==RRC_CONNECTED)
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				for(i=0;i<info->DLInfo.Rank;i++)
//					fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId);
//			else fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId);
//		info=LIST_NEXT(info);
//	}
//	//Form non GBR queue
//	info=enbMac->associatedUEInfo;
//	
//	while(info)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
//			{
//				unsigned int index=0;
//				unsigned int size=info->nonGBRQueue->nSize;
//				unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//				unsigned int nRBGcount;
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//					size = size/info->DLInfo.Rank;
//				size=size*8;
//				//calculate the RB count required
//				while(index<LTE_NRB_MAX)
//				{
//					if(size<TransportBlockSize[nTBSIndex_DL][index])
//						break;
//					index++;
//				}
//				if(index>=LTE_NRB_MAX)
//					index=LTE_NRB_MAX-1;
//				nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//				if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//					nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//				info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//				enbMac->nAllocatedRBG+=nRBGcount;	
//			}
//			if(info->rrcState==RRC_CONNECTED)
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//					for(i=0;i<info->DLInfo.Rank;i++)
//						fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId);
//				else 
//						fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId);
//		info=LIST_NEXT(info);
//	}
//
//	//Print to graph
//	info=enbMac->associatedUEInfo;
//	while(info)
//	{
//		//add to graph
//		if(nLTEGraphFlag && nLTEGraphFlagList[info->nUEId-1])
//		{
//			double total=(double)(info->GBRQueue->nBitcount+info->nonGBRQueue->nBitcount);
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))
//				total*=info->DLInfo.Rank;
//			total=total/1000.0;//mbps
//			fprintf(genericLTEGraph[info->nUEId-1]->dataFile,"%d %0.2lf\n",(int)(pstruEventDetails->dEventTime/1000),total);
//			fflush(genericLTEGraph[info->nUEId-1]->dataFile);
//			genericLTEGraph[info->nUEId-1]->fnCallreplot(genericLTEGraph[info->nUEId-1]);
//		}
//		info=LIST_NEXT(info);
//	}
//	return 1;
//}

//working
//int fn_NetSim_LTE_FormDownlinkFrame_RoundRobin(NETSIM_ID enbId,NETSIM_ID enbInterface)
//{
//	LTE_ENB_MAC* enbMac=DEVICE_MACVAR(enbId,enbInterface);
//	LTE_ENB_PHY* enbPhy=DEVICE_PHYVAR(enbId,enbInterface);
//	LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
//	unsigned int f,i;
//	unsigned int j=0;
//	//Form downlink queue
//	//Form GBR queue
//	enbMac->nAllocatedRBG=0;
//
//	for (f=0;f<enbMac->nAssociatedUECount;f++)
//	{
//		if (info->rrcState == RRC_CONNECTED)
//		{
//			info->RR_count++;
//			if (info->RR_count==enbMac->nAssociatedUECount)
//				info->RR_count=0;
//			j=info->RR_count;
//		}
//		info = LIST_NEXT(info);
//	}
//	
//	info=enbMac->associatedUEInfo;
//	for(f=0;f<j;f++)
//		{
//			info = LIST_NEXT(info);
//		}
//	for (f=0;f<enbMac->nAssociatedUECount;f++)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->GBRQueue->nSize)
//		{
//			unsigned int index=0;
//			unsigned int size=info->GBRQueue->nSize;
//			unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//			unsigned int nRBGcount,flag=0;
//			/*if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				size = size/info->DLInfo.Rank;
//			*/
//			size=size*8;
//			//calculate the RB count required
//			while(index<LTE_NRB_MAX)
//			{
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
//					switch (info->DLInfo.Rank)
//					{
//						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
//									flag=1;
//									break;
//						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
//									flag=1;
//									break;
//						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
//									flag=1;
//									break;
//					}
//				else if(size<TransportBlockSize1[nTBSIndex_DL][index])
//								{
//									flag=1;
//									break;
//								}
//				if(flag==1)
//					break;
//				index++;	
//			}
//			if(index>=LTE_NRB_MAX)
//				index=LTE_NRB_MAX-1;
//			nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//			if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//				nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBStart=enbMac->nAllocatedRBG;
//			info->GBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//			if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
//			switch (info->DLInfo.Rank)
//					{
//						case 2: info->GBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//								break;
//						case 3: info->GBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//								break;
//						case 4: info->GBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//								break;
//					}
//			else  info->GBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//			//info->GBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->GBRQueue->nRBLength-1];
//			enbMac->nAllocatedRBG+=nRBGcount;
//		}
//		if(info->rrcState==RRC_CONNECTED)
//		{
//			fn_NetSim_LTE_RLC_FormRLCSDU(info->GBRQueue,enbId,info->nUEId);
//		}
//		info=LIST_NEXT(info);
//		if(!info)
//		{
//			info=enbMac->associatedUEInfo;
//		}
//	}
//	//Form non GBR queue
//	//info=enbMac->associatedUEInfo;
//	
//	for (f=0;f<enbMac->nAssociatedUECount;f++)
//	{
//		NETSIM_ID ueId=info->nUEId;
//		NETSIM_ID ueInterface=info->nUEInterface;
//		if(info->rrcState==RRC_CONNECTED && info->nonGBRQueue->nSize)
//			{
//				unsigned int index=0;
//				unsigned int size=info->nonGBRQueue->nSize;
//				unsigned int nTBSIndex_DL=info->DLInfo.TBSIndex;
//				unsigned int nRBGcount,flag=0;
//				//if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))	
//				//	size = size/info->DLInfo.Rank;
//				size=size*8;
//				////calculate the RB count required
//				while(index<LTE_NRB_MAX)
//				{
//					if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
//					switch (info->DLInfo.Rank)
//					{
//						case 2:if(size<TransportBlockSize2[nTBSIndex_DL][index])
//								flag=1;
//								break;
//						case 3:if(size<TransportBlockSize3[nTBSIndex_DL][index])
//								flag=1;
//								break;
//						case 4:if(size<TransportBlockSize4[nTBSIndex_DL][index])
//								flag=1;
//								break;
//
//					}
//					else if(size<TransportBlockSize1[nTBSIndex_DL][index])
//							{
//								flag=1;
//								break;
//							}
//					if(flag==1)
//					break;
//					index++;
//				}
//				if(index>=LTE_NRB_MAX)
//					index=LTE_NRB_MAX-1;
//				nRBGcount=(int)ceil((double)(index+1)/enbMac->nRBCountInGroup);
//				if(nRBGcount>enbMac->nRBGCount-enbMac->nAllocatedRBG)
//					nRBGcount=enbMac->nRBGCount-enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBStart=enbMac->nAllocatedRBG;
//				info->nonGBRQueue->nRBLength=nRBGcount*enbMac->nRBCountInGroup;
//				if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation== Modulation_64QAM)) 
//					switch (info->DLInfo.Rank)
//						{
//							case 2: info->nonGBRQueue->nBitcount=TransportBlockSize2[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//									break;
//							case 3: info->nonGBRQueue->nBitcount=TransportBlockSize3[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//									break;
//							case 4: info->nonGBRQueue->nBitcount=TransportBlockSize4[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//									break;
//						}
//				else  info->nonGBRQueue->nBitcount=TransportBlockSize1[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//				//info->nonGBRQueue->nBitcount=TransportBlockSize[nTBSIndex_DL][info->nonGBRQueue->nRBLength-1];
//				enbMac->nAllocatedRBG+=nRBGcount;	
//			}
//			if(info->rrcState==RRC_CONNECTED)
//			{
//				fn_NetSim_LTE_RLC_FormRLCSDU(info->nonGBRQueue,enbId,info->nUEId);
//			}	
//			info=LIST_NEXT(info);
//			if(!info)
//				info = enbMac->associatedUEInfo;
//	}
//
//	//Print to graph
//	info=enbMac->associatedUEInfo;
//	while(info)
//	{
//		//add to graph
//		if(nLTEGraphFlag && nLTEGraphFlagList[info->nUEId-1])
//		{
//			double total=(double)(info->GBRQueue->nBitcount+info->nonGBRQueue->nBitcount);
//			/*if ((MODE_INDEX_MAPPING[info->TransmissionIndex.DL].TRANSMISSION_MODE == SingleUser_MIMO_Spacial_Multiplexing && info->DLInfo.modulation == Modulation_64QAM))
//				total*=info->DLInfo.Rank;*/
//			total=total/1000.0;//mbps
//			fprintf(genericLTEGraph[info->nUEId-1]->dataFile,"%d %0.2lf\n",(int)(pstruEventDetails->dEventTime/1000),total);
//			fflush(genericLTEGraph[info->nUEId-1]->dataFile);
//			genericLTEGraph[info->nUEId-1]->fnCallreplot(genericLTEGraph[info->nUEId-1]);
//		}
//		info=LIST_NEXT(info);
//	}
//	return 1;
//}
