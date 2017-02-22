
/***********************************************************************************/
/***********************************************************************************/
#include "main.h"
#include "WLAN.h"
#include "802_11n_ac.h"
/**
Packets with same Receiver ID are aggregated. Broadcast packets are not aggregated
*/
NetSim_PACKET* fn_NetSim_WLAN_Packet_GetAggregatedPacketFromBuffer(struct stru_802_11e_Packetlist* pstruQosList,int nDeviceId,int nFlag)
{
	 
	NetSim_PACKET* pstruPacketHead, *pstruPacket, *pstruAggListPrevPacket;
	NetSim_PACKET *pstruList0,*pstruList1,*pstruList2=NULL,*pstruList3, *pstruListPrevPacket ;
	MAC_VAR *pstruMacVar ; 
	int n, nInterfaceId;
	n=0;

	for(nInterfaceId=1; nInterfaceId <= (int)NETWORK->ppstruDeviceList[nDeviceId-1]->nNumOfInterface;nInterfaceId++)
	{
		pstruMacVar=(MAC_VAR*)DEVICE_INTERFACE(nDeviceId,nInterfaceId)->pstruMACLayer->MacVar;
		n=pstruMacVar->nNumberOfAggregatedPackets;
		if(n>0 && n<=1024)
			break;

	}
	if(n>0 && n<=1024)
	{}
	else
		n=1;
	
	if(pstruQosList->pstruList[0])
	{
		pstruPacketHead = pstruQosList->pstruList[0];
		//dArrivalTime=pstruPacket->pstruMacData->dArrivalTime;
		
		if(nFlag)
		{

			n--;
			pstruAggListPrevPacket=pstruPacketHead;
			pstruQosList->pstruList[0] = pstruQosList->pstruList[0]->pstruNextPacket;
			pstruAggListPrevPacket->pstruNextPacket=NULL;
			pstruList0=pstruQosList->pstruList[0];


			while(n>0 && pstruList0!=NULL && pstruPacketHead->nDestinationId!=0)
			{
				n--;
				if(pstruList0->nDestinationId==0||pstruList0->nReceiverId != pstruPacketHead->nReceiverId)
				{
					pstruListPrevPacket=pstruList0;
					pstruList0=pstruList0->pstruNextPacket;
					continue;
				}
				if(pstruQosList->pstruList[0]==pstruList0)	//deleting packet from front
				{
					pstruPacket=pstruList0;
					pstruList0=pstruList0->pstruNextPacket;
					pstruQosList->pstruList[0]=pstruQosList->pstruList[0]->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
				}
				else//deleting packet from middle
				{
					pstruPacket=pstruList0;
					pstruListPrevPacket->pstruNextPacket=pstruList0->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
					pstruList0=pstruListPrevPacket->pstruNextPacket;
				}
				pstruAggListPrevPacket->pstruNextPacket=pstruPacket;
				pstruAggListPrevPacket=pstruAggListPrevPacket->pstruNextPacket;
			}
			
		}
	
		return pstruPacketHead;
	}
	else if(pstruQosList->pstruList[1])
	{
		pstruPacketHead = pstruQosList->pstruList[1];
		
		if(nFlag)
		{
			n--;
			pstruAggListPrevPacket=pstruPacketHead;
			pstruQosList->pstruList[1] = pstruQosList->pstruList[1]->pstruNextPacket;
			pstruAggListPrevPacket->pstruNextPacket=NULL;
			pstruList1=pstruQosList->pstruList[1];


			while(n>0 && pstruList1!=NULL && pstruPacketHead->nDestinationId!=0)
			{
				n--;
				if(pstruList1->nDestinationId==0||pstruList1->nReceiverId != pstruPacketHead->nReceiverId)
				{
					pstruListPrevPacket=pstruList1;
					pstruList1=pstruList1->pstruNextPacket;
					continue;
				}
				if(pstruQosList->pstruList[1]==pstruList1)	//deleting packet from front
				{
					pstruPacket=pstruList1;
					pstruList1=pstruList1->pstruNextPacket;
					pstruQosList->pstruList[1]=pstruQosList->pstruList[1]->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
				}
				else//deleting from middle
				{
					pstruPacket=pstruList1;
					pstruListPrevPacket->pstruNextPacket=pstruList1->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
					pstruList1=pstruListPrevPacket->pstruNextPacket;
				}
				pstruAggListPrevPacket->pstruNextPacket=pstruPacket;
				pstruAggListPrevPacket=pstruAggListPrevPacket->pstruNextPacket;
			}
			
		}
		
		return pstruPacketHead;
	}
	else if(pstruQosList->pstruList[2])
	{
		pstruPacketHead = pstruQosList->pstruList[2];
		
	
		if(nFlag)
		{
			n--;
			pstruAggListPrevPacket=pstruPacketHead;
			pstruQosList->pstruList[2] = pstruQosList->pstruList[2]->pstruNextPacket;
			pstruAggListPrevPacket->pstruNextPacket=NULL;
			pstruList2=pstruQosList->pstruList[2];


			while(n>0 && pstruList2!=NULL && pstruPacketHead->nDestinationId!=0)
			{
				n--;
				if(pstruList2->nDestinationId==0||pstruList2->nReceiverId != pstruPacketHead->nReceiverId)
				{
					pstruListPrevPacket=pstruList2;
					pstruList2=pstruList2->pstruNextPacket;
					continue;
				}
				if(pstruQosList->pstruList[2]==pstruList2)	//deleting packet from front
				{
					pstruPacket=pstruList2;
					pstruList2=pstruList2->pstruNextPacket;
					pstruQosList->pstruList[2]=pstruQosList->pstruList[2]->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
				}
				else//deleting from middle
				{
					pstruPacket=pstruList2;
					pstruListPrevPacket->pstruNextPacket=pstruList2->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
					pstruList2=pstruListPrevPacket->pstruNextPacket;
				}
				pstruAggListPrevPacket->pstruNextPacket=pstruPacket;
				pstruAggListPrevPacket=pstruAggListPrevPacket->pstruNextPacket;
			}
			
		}
		
		return pstruPacketHead;
	}
	else if(pstruQosList->pstruList[3])
	{
		pstruPacketHead = pstruQosList->pstruList[3];
		pstruPacket = pstruQosList->pstruList[3];
		
		if(nFlag)
		{
			n--;
			pstruAggListPrevPacket=pstruPacketHead;
			pstruQosList->pstruList[3] = pstruQosList->pstruList[3]->pstruNextPacket;
			pstruAggListPrevPacket->pstruNextPacket=NULL;
			pstruList3=pstruQosList->pstruList[3];


			while(n>0 && pstruList3!=NULL && pstruPacketHead->nDestinationId!=0)
			{
				n--;
				if(pstruList3->nDestinationId==0||pstruList3->nReceiverId != pstruPacketHead->nReceiverId)
				{
					pstruListPrevPacket=pstruList2;
					pstruList3=pstruList3->pstruNextPacket;
					continue;
				}
				if(pstruQosList->pstruList[3]==pstruList3)	//deleting packet from front
				{
					pstruPacket=pstruList3;
					pstruList3=pstruList3->pstruNextPacket;
					pstruQosList->pstruList[3]=pstruQosList->pstruList[3]->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
				}
				else//deleting from middle
				{
					pstruPacket=pstruList3;
					pstruListPrevPacket->pstruNextPacket=pstruList3->pstruNextPacket;
					pstruPacket->pstruNextPacket=NULL;
					pstruList3=pstruListPrevPacket->pstruNextPacket;
				}
				pstruAggListPrevPacket->pstruNextPacket=pstruPacket;
				pstruAggListPrevPacket=pstruAggListPrevPacket->pstruNextPacket;
			}
			
		}
		
		return pstruPacketHead;
	}
	return NULL;
}
/// This function is used to free the aggregated Packet
void Aggregated_Packet_FreePacket(NetSim_PACKET *pstruPacketHead)
{
	NetSim_PACKET *pstruPacket;
	while(pstruPacketHead)
	{
		pstruPacket=pstruPacketHead;
		pstruPacketHead=pstruPacketHead->pstruNextPacket;
		pstruPacket->pstruNextPacket=NULL;
		fn_NetSim_Packet_FreePacket(pstruPacket);
		pstruPacket=NULL;
	}
	pstruPacketHead=NULL;
}
/** 
This function returns 1 if entire AMPDU collide i.e. header frame gets collided
else return 0
*/
int Check_AMPDU_Collide()
{
	NetSim_PACKET *pstruPacketHead;
	NetSim_PACKET *pstruPacket=pstruEventDetails->pPacket;
	pstruPacketHead=pstruPacket;
	
	if(pstruPacketHead->nPacketStatus==PacketStatus_Collided)
	{
		Aggregated_Packet_FreePacket(pstruPacketHead);
		return 1;
	}
	else 
		return 0;
}
/**
This function is used to generate the cummulative acknowledgement packet.
*/
NetSim_PACKET *Generate_Block_Ack_Packet()
{
	
	NetSim_PACKET *pstruBlockAckPkt,*pstruPacket;
	BlockACK_FRAME *pstruBlockAck;
	int i;
	PHY_VAR *pstruPhy = DEVICE_PHYVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
	pstruPacket=pstruEventDetails->pPacket;

	//Generate a BLOCK ACK Packet Format
	pstruBlockAckPkt = fn_NetSim_Packet_CreatePacket(MAC_LAYER);
	pstruBlockAckPkt->nPacketType = PacketType_Control;
	pstruBlockAckPkt->nPacketPriority = Priority_High;
	pstruBlockAckPkt->nControlDataType = WLAN_CNTRL_PKT(WLAN_BlockACK,pstruEventDetails->nProtocolId);
	pstruBlockAck =  fnpAllocateMemory(1, sizeof(BlockACK_FRAME));
	pstruBlockAck->pstruFrameControl = fnpAllocateMemory(1,sizeof(FRAME_CONTROL));	
	pstruBlockAck->pstruFrameControl->FrameType = CONTROL;
	pstruBlockAck->pstruFrameControl->usnSubType = BlockAck;	
	pstruBlockAck->RA =_strdup( ((MAC_HEADER*)(pstruPacket->pstruMacData->Packet_MACProtocol))->Address2 );
	pstruBlockAck->TA =_strdup( ((MAC_HEADER*)(pstruPacket->pstruMacData->Packet_MACProtocol))->Address1 );
	if(pstruPhy->nPHYprotocol==IEEE_802_11n)
	{
		for(i=0;i<MAX_802_11n_AMPDU_SIZE;i++)
		{
			pstruBlockAck->BitMap[i]=0;
		}
	}
	else if(pstruPhy->nPHYprotocol==IEEE_802_11ac)
		for(i=0;i<MAX_802_11ac_AMPDU_SIZE;i++)
		{
			pstruBlockAck->BitMap[i]=0;
		}

	pstruBlockAckPkt->nReceiverId  = pstruPacket->nReceiverId; 
	pstruBlockAckPkt->nDestinationId = pstruPacket->nDestinationId;
	pstruBlockAckPkt->nTransmitterId=pstruPacket->nTransmitterId;
	pstruBlockAckPkt->nSourceId=pstruPacket->nSourceId;
	pstruBlockAckPkt->pstruMacData->Packet_MACProtocol = pstruBlockAck;

	return pstruBlockAckPkt;
}

/// This fuction is used to copy the Aggregated Packet
NetSim_PACKET* fn_NetSim_Packet_CopyAggregatedPacket(NetSim_PACKET *packetList)//NULL;
{
	NetSim_PACKET *packetHead=NULL;
	NetSim_PACKET *packetHeadList=NULL;
	NetSim_PACKET *packet=NULL;

	while(packetList)
	{
		packet=fn_NetSim_Packet_CopyPacket(packetList);
		packet->pstruNextPacket=NULL;

		if(packetHead)
		{
			packetHeadList->pstruNextPacket=packet;
			packetHeadList=packetHeadList->pstruNextPacket;
			packet=NULL;
			
		}
		else
		{
			packetHead=packet;
			packetHead->pstruNextPacket=NULL;
			packetHeadList=packetHead;
			packet=NULL;

		}
		
		packetList=packetList->pstruNextPacket;

	}
	return packetHead;
}