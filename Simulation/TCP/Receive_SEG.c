#include "main.h"
#include "TCP.h"
/*--------------------------------------------------------------------------------*
* Copyright (C) 2013															  *
*																				  *
* TETCOS, Bangalore. India                                                        *
																				  *
* Tetcos owns the intellectual property rights in the Product and its content.    *
* The copying, redistribution, reselling or publication of any or all of the      *
* Product or its content without express prior written consent of Tetcos is       *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.     *

* Author:   P.Sathishkumar														  *
* Date  :    29-Oct-2012														  *
* --------------------------------------------------------------------------------*/

/**  The following function is mainly used to receive the segment */
int fn_NetSim_TCP_RECEIVE_SEG()
{
	NetSim_PACKET *pstruReceived_Segment;
	SEGMENT_HEADER_TCP *pstruSegmentHeader;
	TCB *pstruTCP_Connection_Var;
	NETSIM_ID nDeviceID, nSourceID, nDestID;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort;
	unsigned short int usnDest_Port;
	int nDuplicate_SEG = 0;
	unsigned int unDelete_SEG_Count = 0;

	pstruReceived_Segment=pstruEventDetails->pPacket;
	nDeviceID = pstruReceived_Segment->nDestinationId;
	nSourceID = pstruReceived_Segment->nDestinationId;
	nDestID = pstruReceived_Segment->nSourceId;

	pstruSegmentHeader = (SEGMENT_HEADER_TCP*)pstruReceived_Segment->pstruTransportData->Packet_TransportProtocol;
	pstruReceived_Segment->pstruTransportData->dOverhead -= TRANSPORT_TCP_OVERHEADS;
	pstruReceived_Segment->pstruTransportData->dPacketSize = pstruReceived_Segment->pstruTransportData->dPayload + pstruReceived_Segment->pstruTransportData->dOverhead;
	szSourceIP = pstruReceived_Segment->pstruNetworkData->szDestIP;
	szDestIP = pstruReceived_Segment->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruReceived_Segment->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruReceived_Segment->pstruTransportData->nSourcePort; 

	fn_NetSim_TCP_Check_Connection(nDeviceID,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);

	if(!pstruTCP_Connection_Var)
	{
		printf("TCP-- Connection not available for the device %d\n",nDeviceID);
		return 1;
	}

	pstruTCP_Connection_Var->ulnSEG_SEQ = pstruSegmentHeader->ulnSequence_Number;

	nDuplicate_SEG = fn_NetSim_TCP_AddSegmentTo_Receiver_Buffer(pstruTCP_Connection_Var,pstruReceived_Segment);

	if(!nDuplicate_SEG)
	{
		if(pstruSegmentHeader->bReTransmission_Flag)
		{
			pstruTCP_Connection_Var->pstruMetrics->nTotal_SEG_Received++;
			pstruTCP_Connection_Var->pstruMetrics->nTotal_SEG_Recovered++;
		}
		else
		{
			pstruTCP_Connection_Var->pstruMetrics->nTotal_SEG_Received++;
		}
	}
	else
		pstruTCP_Connection_Var->pstruMetrics->nTotal_Dup_SEG_Received++;

	//fn_NetSim_TCP_Data_Sequencing(pstruTCP_Connection_Var);
	fn_NetSim_TCP_Delete_Segments_From_Receiver_Buffer(pstruTCP_Connection_Var,&unDelete_SEG_Count);

	//For connection termination

	if(pstruSegmentHeader->bFIN && pstruTCP_Connection_Var->ulnRCV_NXT == pstruSegmentHeader->ulnSequence_Number + pstruReceived_Segment->pstruTransportData->dPayload)
	{
		/* TCP_SEND_FIN_SEG Event, Add the event details for TRANSPORT_OUT_EVENT 
		and sub event as TCP_SEND_SEG */
 		pstruEventDetails->nDeviceId = pstruReceived_Segment->nDestinationId;
		pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
		pstruEventDetails->nSubEventType = TCP_SEND_FIN_SEG;
		pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstruReceived_Segment);
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetTrnspProtocol(pstruEventDetails->nDeviceId,pstruEventDetails->pPacket);
		fnpAddEvent(pstruEventDetails);

		//For Connection termination change the state
		fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_CLOSE_WAIT);
	}
	return 0;
}


/**  The following function is mainly used to add the received segment to receiver buffer.
when forming a receiver buffer, it is possible to receive out of oreder segment. 
how it is obtained?  by using the variable expected segment sequence number. 
this expected segment sequence number tells about expected segment in 
the receiver buffer. */
int fn_NetSim_TCP_AddSegmentTo_Receiver_Buffer(TCB *pstru_TCP_Connection_Var,NetSim_PACKET *pstru_Received_Segment)
{
	NetSim_PACKET *pstru_Currnet_Buffer=NULL,*prev=NULL;
	NetSim_PACKET *pstru_New_Segment;
	SEGMENT_HEADER_TCP *pstruReceivedSegmentHeader;

	pstruReceivedSegmentHeader = (SEGMENT_HEADER_TCP*)pstru_Received_Segment->pstruTransportData->Packet_TransportProtocol;
	pstru_TCP_Connection_Var->ulnRCV_WND = pstruReceivedSegmentHeader->nWindow;

	/* If Received segment sequence number and expepcted segment sequence number both are same */
	if(pstru_TCP_Connection_Var->ulnRCV_NXT == pstruReceivedSegmentHeader->ulnSequence_Number)
	{
		//Add at top
		pstru_Currnet_Buffer = pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer;
		pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer = pstru_Received_Segment;
		pstru_Received_Segment->pstruNextPacket = pstru_Currnet_Buffer;

		/* set the next expected seq number */
		pstru_TCP_Connection_Var->ulnRCV_NXT += pstru_Received_Segment->pstruTransportData->dPayload;
		while(pstru_Currnet_Buffer)
		{
			pstruReceivedSegmentHeader = (SEGMENT_HEADER_TCP*)pstru_Currnet_Buffer->pstruTransportData->Packet_TransportProtocol;
			if(pstru_TCP_Connection_Var->ulnRCV_NXT == pstruReceivedSegmentHeader->ulnSequence_Number)
			{
				pstru_TCP_Connection_Var->ulnRCV_NXT += pstru_Currnet_Buffer->pstruTransportData->dPayload;
				pstru_Currnet_Buffer = pstru_Currnet_Buffer->pstruNextPacket;
			}
			else
				break;
		}
		return 0;
	}	
	/* If out of order segment is received */ 
	else if(pstruReceivedSegmentHeader->ulnSequence_Number > pstru_TCP_Connection_Var->ulnRCV_NXT)
	{
		if(pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer)
		{
			SEGMENT_HEADER_TCP *tmpHdr;
			pstru_Currnet_Buffer = pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer;

			while(pstru_Currnet_Buffer)
			{
				tmpHdr= (SEGMENT_HEADER_TCP*)pstru_Currnet_Buffer->pstruTransportData->Packet_TransportProtocol;
				if(tmpHdr->ulnSequence_Number == pstruReceivedSegmentHeader->ulnSequence_Number)
					return 1; //Duplicate segment
				if(tmpHdr->ulnSequence_Number > pstruReceivedSegmentHeader->ulnSequence_Number)
					break; // Previous segment received
				prev = pstru_Currnet_Buffer;
				pstru_Currnet_Buffer = pstru_Currnet_Buffer->pstruNextPacket;
			}
			if(!pstru_Currnet_Buffer)
				prev->pstruNextPacket = pstru_Received_Segment; // Highest seq number segment
			else if(!prev)
			{
				//First out of order
				pstru_Received_Segment->pstruNextPacket = pstru_Currnet_Buffer;
				pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer = pstru_Received_Segment;
			}
			else
			{
				//Multiple out of order
				prev->pstruNextPacket = pstru_Received_Segment;
				pstru_Received_Segment->pstruNextPacket = pstru_Currnet_Buffer;
			}
		}
		else
			pstru_TCP_Connection_Var->pstru_TCP_Receiver_Buffer = pstru_Received_Segment;
		return 0;
	}
	else
		return 1; //Duplicate segment
}

/**  The following function is mainly used to deliver the segments to the application.
	when received segment count reaches the advertised window size, the segments  will be
	delivered to the application. */
int fn_NetSim_TCP_Delete_Segments_From_Receiver_Buffer(TCB *pstruTCP_Connection_Var,unsigned int *unDelete_SEG_Count)
{
	NetSim_PACKET *pstruTraverse_Receiver_Buffer;
	NetSim_PACKET *pstruPrevious_SEG=NULL;
	NetSim_PACKET *pstruNext_SEG;
	NetSim_PACKET *pstruTmp_Receiver_Buffer;
	SEGMENT_HEADER_TCP *pstruSegmentHeader;
	SEGMENT_HEADER_TCP *pstruPrevious_SegmentHeader;

	/* if receiver buffer is don't do anythin */
	if (!pstruTCP_Connection_Var->pstru_TCP_Receiver_Buffer) 
		return 0;

	pstruTmp_Receiver_Buffer = pstruTCP_Connection_Var->pstru_TCP_Receiver_Buffer;

	while (pstruTmp_Receiver_Buffer) 
	{
		pstruSegmentHeader = (SEGMENT_HEADER_TCP*)pstruTmp_Receiver_Buffer->pstruTransportData->Packet_TransportProtocol;
		if(pstruSegmentHeader->ulnSequence_Number < pstruTCP_Connection_Var->ulnRCV_NXT)
		{
			/* move to app in */
			pstruEventDetails->nEventType=APPLICATION_IN_EVENT;
			pstruEventDetails->nDeviceId = pstruEventDetails->nDeviceId;
			pstruEventDetails->nPacketId = pstruTmp_Receiver_Buffer->nPacketId;
			pstruEventDetails->pPacket->nControlDataType = pstruSegmentHeader->nPrevControlDatatype;
			pstruEventDetails->nSegmentId = pstruTmp_Receiver_Buffer->pstruAppData->nSegmentId;
			pstruEventDetails->nSubEventType=ZERO;
			pstruEventDetails->dPacketSize=pstruTmp_Receiver_Buffer->pstruTransportData->dPacketSize;
			/*Add the segment to event details */
			pstruNext_SEG = pstruTmp_Receiver_Buffer->pstruNextPacket;
			pstruEventDetails->pPacket = pstruTmp_Receiver_Buffer;
			pstruEventDetails->pPacket->pstruNextPacket = NULL;
			pstruEventDetails->nProtocolId=0;
			fnpAddEvent(pstruEventDetails);
			*unDelete_SEG_Count += pstruTmp_Receiver_Buffer->pstruTransportData->dPayload;
			pstruTmp_Receiver_Buffer = pstruNext_SEG;
			pstruTCP_Connection_Var->pstru_TCP_Receiver_Buffer = pstruNext_SEG;
		}
		else
			break;
	}
	return 0;
}

