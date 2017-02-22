/************************************************************************************
 * Copyright (C) 2012						
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    Thamothara Kannan                                                      *
 * ----------------------------------------------------------------------------------*/
#include "main.h"
#include "Main_FE.h"

/**
	This function forward the data from input port of the switch to the output port.
*/
int fn_NetSim_FastEthernet_Switch_Frame_Forwarding(NetSim_EVENTDETAILS *pstruEventDetails)
{
	NETSIM_ID ni;
	NETSIM_ID nOutPort = 0;		 /* Get the out port number from the look up table*/
	NETSIM_ID nLoop = 0;			 /* Loop variable to search Look up table*/
	int nDestNode =0;		/* Destination device Id*/
	int nBufferFlag=0;		/* Buffer checking flag*/
	int nMax_Port=0;		/* Number of port used in the switch*/
	int nInterfaceId=0;
	double dPacket_Size = 0.0;
	double ldLatency=0.0;
	double dEventTime=0.0;
	int nBufferPacketStatus;

	/* To store the copy packet	for broad casting*/
	NetSim_PACKET *pstruPacket_Copy=NULL;	
	/* Store the temporary packet from the event details*/
	NetSim_PACKET *pstruTemp_Data=NULL;		
	/* Type cast Interfacelist declaration*/
	NetSIm_DEVICEINTERFACE *pstruInterface=NULL;
	/*Type cast Device Var declaration*/
	DEVICE_VARIABLES *pstruDeviceVar=NULL;
	/*Type casting Device Var*/
	pstruDeviceVar=((DEVICE_VARIABLES *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->deviceVar));
	/* Assign the packet from even details to temp packet*/
	pstruTemp_Data =pstruEventDetails->pPacket;
	dEventTime = pstruEventDetails->dEventTime;
	/* Get the port Id*/
	nInterfaceId=pstruEventDetails->nInterfaceId;
	/*Get the destination node id*/
	nDestNode= fn_NetSim_Stack_GetDeviceId_asMac(pstruTemp_Data->pstruMacData->szDestMac,&ni);
	dPacket_Size = pstruEventDetails->dPacketSize;
	ldLatency=pstruDeviceVar->dSwitchingLatency;
	
	/*Set the arrival time to include the latency in queuing delay*/
	pstruTemp_Data->pstruMacData->dArrivalTime=dEventTime;
	pstruTemp_Data->pstruMacData->dPayload=pstruTemp_Data->pstruPhyData->dPacketSize - dMac_Over_Heads;
	pstruTemp_Data->pstruMacData->dPacketSize = pstruTemp_Data->pstruMacData->dPayload;
	pstruTemp_Data->pstruMacData->dOverhead= dMac_Over_Heads;
	
	/*If the destination id 0 its broad cast frame*/
	if(nDestNode!=0)
	{
		/* check the destination matches with the switch table*/
		if(pstruDeviceVar->ppstruSwitchTable[nDestNode-1]->nNode_Id ==nDestNode)
		{
			/* assign the switch table Output Interface as Out Port*/
			nOutPort = pstruDeviceVar->ppstruSwitchTable[nDestNode-1]->nOut_Port;
		}
		else	/*there is no entry for the destination in the switch table*/
		{
			nOutPort=0;
		}
	}
	else
	{
		/*Forwards the received frame to all the used port in the switch*/
		nOutPort=0;
	}

	/*check an output port is stored in the switch table,then place the data in the output port*/
	if(nOutPort!=0)
	{
		pstruInterface =NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nOutPort-1];
		/* check the output port is not blocked*/
		if(((PORT_DATA *)(pstruInterface->pstruMACLayer->MacVar))->nPort_state == FORWARDING)
		{
			/*check the buffer*/
			nBufferFlag=fn_NetSim_FastEthernet_Switch_BufferCheck(pstruEventDetails,dPacket_Size,nOutPort);
							
			/*check the buffer is underflow or overflow*/
			if(nBufferFlag==1)
			{

				/* Copy the packet from the source packet*/
				pstruPacket_Copy=fn_NetSim_Packet_CopyPacket(pstruTemp_Data);

				/* Add Packet to tail of port buffer*/
				/*fn_NetSim_Packet_AddPacketToList(&(pstruInterface->pstruAccessInterface->pstruAccessBuffer->pstruPacketlist),\
						pstruPacket_Copy,2);*/
				
				//Check is there any packets in buffer. Return 1 if there is a packet.Return 0 if there no packet.
				nBufferPacketStatus = fn_NetSim_GetBufferStatus(pstruInterface->pstruAccessInterface->pstruAccessBuffer);
				// If Status is 1. Then add packet to buffer. Else add packet to buffer and add MAC_OUT event. 
				fn_NetSim_Packet_AddPacketToList(pstruInterface->pstruAccessInterface->pstruAccessBuffer,pstruPacket_Copy,3);//2);
				if(((DEVICE_PHYSICALLAYER*)pstruInterface->pstruPhysicalLayer->phyVar)->nLinkState == LINK_DOWN && !nBufferPacketStatus)
				{
					pstruEventDetails->dEventTime=dEventTime+ldLatency;
					pstruEventDetails->nSubEventType= ZERO;
					pstruEventDetails->nEventType=MAC_OUT_EVENT;	/* Write Mac Out Event into the EventList*/
					pstruEventDetails->nInterfaceId=nOutPort;
					pstruEventDetails->dPacketSize=pstruTemp_Data->pstruMacData->dPacketSize;
					pstruEventDetails->pPacket=NULL;
					fnpAddEvent( pstruEventDetails);
				}

			}
			else /*buffer overflow,drop the frame*/
			{
				pstruTemp_Data->nPacketStatus = PacketStatus_Buffer_Dropped;
				fn_NetSim_WritePacketTrace(pstruTemp_Data);
				fn_NetSim_Packet_FreePacket(pstruTemp_Data);				
			}
		}
		else
		{
			/*dont copy the data in the blocked port*/

		}
	}
	/*output port is not there,then place the data in all the ports except the input port*/
	else
	{
		/*get the number of port used in the switch*/
		nMax_Port=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->nNumOfInterface;

		for(nLoop =1;nLoop <=nMax_Port;nLoop++)
		{
			pstruInterface=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nLoop-1];
			/*Check the port is not null*/
			if(pstruInterface->pstruMACLayer!=NULL)
			{
				/*check port id is not equal to input port id*/
				if(nLoop!=nInterfaceId)
				{
					/* check the output port is not blocked*/
					if(((PORT_DATA *)(pstruInterface->pstruMACLayer->MacVar))->nPort_state == FORWARDING)
					{
						/*check the buffer*/
						nBufferFlag=fn_NetSim_FastEthernet_Switch_BufferCheck(pstruEventDetails,dPacket_Size,nLoop);
						if(nBufferFlag ==1)
						{
							/* Copy the packet from the source packet*/
							pstruPacket_Copy=fn_NetSim_Packet_CopyPacket(pstruTemp_Data);

							/*Add the packet to Switch output port buffer*/
							//fn_NetSim_Packet_AddPacketToList(&(pstruInterface->pstruAccessInterface->pstruAccessBuffer->pstruPacketlist),pstruPacket_Copy,2);
							//Check is there any packets in buffer. Return 1 if there is a packet.Return 0 if there no packet.
							nBufferPacketStatus = fn_NetSim_GetBufferStatus(pstruInterface->pstruAccessInterface->pstruAccessBuffer);
							// If Status is 1. Then add packet to buffer. Else add packet to buffer and add MAC_OUT event. 
							fn_NetSim_Packet_AddPacketToList(pstruInterface->pstruAccessInterface->pstruAccessBuffer,pstruPacket_Copy,3);//2);
							if(((DEVICE_PHYSICALLAYER*)pstruInterface->pstruPhysicalLayer->phyVar)->nLinkState == LINK_DOWN && !nBufferPacketStatus)
							{
								pstruEventDetails->nSubEventType=ZERO;
								pstruEventDetails->nEventType=MAC_OUT_EVENT;
								pstruEventDetails->dEventTime= dEventTime + ldLatency;
								pstruEventDetails->nInterfaceId=nLoop;
								pstruEventDetails->dPacketSize=pstruTemp_Data->pstruMacData->dPacketSize;
								pstruEventDetails->pPacket=NULL;
								fnpAddEvent(pstruEventDetails);	
							}
						}
						else/* drop the frame frame, buffer is overflow*/
						{
							pstruTemp_Data->nPacketStatus = PacketStatus_Buffer_Dropped;
							fn_NetSim_WritePacketTrace(pstruTemp_Data);
							fn_NetSim_Packet_FreePacket(pstruTemp_Data);							
						}
					}
				}
				else
				{
					/*dont copy the data in the blocked port*/
				}
			}
		}/*end of frame forwarding*/
	}/*end of else*/
	
	fn_NetSim_Packet_FreePacket(pstruTemp_Data);

	return 0;
}
/**
	Check the switch buffer is over flow or under flow								
*/
int fn_NetSim_FastEthernet_Switch_BufferCheck (NetSim_EVENTDETAILS *pstruEventDetails,double dPacket_Size,int nOutPort)
{
	double dMax_BufferSize = 0.0;
	double dCur_BufferSize=0.0;
	struct stru_NetSim_Device_Buffer *pstruBuffer;

	pstruBuffer=NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[nOutPort-1]->pstruAccessInterface->pstruAccessBuffer;

	dMax_BufferSize=pstruBuffer->dMaxBufferSize * 1024 * 1024;
	dCur_BufferSize=pstruBuffer->dCurrentBufferSize + dPacket_Size;

	/*check the buffer is overflow*/
	if(dCur_BufferSize <=dMax_BufferSize)
	{
		/*buffer underflow*/
		pstruBuffer->dCurrentBufferSize=dCur_BufferSize;
		return 1;
	}
	else
		/*buffer overflow*/
		return 2;
}
