#include "main.h"
#include "TCP.h"
/*---------------------------------------------------------------------------------
* Copyright (C) 2013															  *
*																				  *
* TETCOS, Bangalore. India                                                        *
																				  *
* Tetcos owns the intellectual property rights in the Product and its content.    *
* The copying, redistribution, reselling or publication of any or all of the      *
* Product or its content without express prior written consent of Tetcos is       *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.     *

* Author:   P.Sathishkumar 
* Date  :    21-May-2013
* --------------------------------------------------------------------------------*/


/**
This function is to send the acknowledgement for the data segments.
*/
int fn_NetSim_TCP_SEND_ACK()
{
	TCB *pstruTCP_Connection_Var;
	NETSIM_ID nSourceID, nDestID;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	NetSim_PACKET *pstruTCP_Ack_Segment;
	SEGMENT_HEADER_TCP *pstruAck_Header;
	SEGMENT_HEADER_TCP *pstruSegment_Header; /* To store received segment's header information */
	double dEventTime;

	nSourceID = pstruEventDetails->pPacket->nDestinationId;
	nDestID = pstruEventDetails->pPacket->nSourceId;
	szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
	szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
	usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort;
	usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

	dEventTime = pstruEventDetails->dEventTime;

	fn_NetSim_TCP_Check_Connection(nSourceID, szSourceIP, szDestIP, usnSourcePort, usnDest_Port, &pstruTCP_Connection_Var);

	if(!pstruTCP_Connection_Var)
	{
		printf("TCP-- Connection not available for the device %d\n",nSourceID);
		return 1;
	}

	pstruAck_Header = fnpAllocateMemory(1,sizeof(SEGMENT_HEADER_TCP));
	pstruSegment_Header = pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

	/* Set the source port number */
	pstruAck_Header->usnSource_Port = usnSourcePort;
	/* Set the destination port number */
	pstruAck_Header->usnDestination_Port = usnDest_Port;
	/* Set the SEQ number */
	pstruAck_Header->ulnSND_SEG_SEQ = SEG_SEQ(pstruEventDetails->pPacket);// changed for connection establishment
	pstruAck_Header->ulnSequence_Number = IRS+1;
	/* Set the ACK number */
	pstruAck_Header->ulnAcknowledgment_Number = pstruTCP_Connection_Var->ulnRCV_NXT;
	/* Set the ACK flag */
	pstruAck_Header->bACK = true;
	pstruAck_Header->nWindow = (unsigned short)pstruTCP_Connection_Var->ulnRCV_NXT;
	pstruAck_Header->bReTransmission_Flag = pstruSegment_Header->bReTransmission_Flag;
	/* Create NetSim packet for Layer 4 */
	pstruTCP_Ack_Segment = fn_NetSim_Packet_CreatePacket(TRANSPORT_LAYER);

	pstruTCP_Ack_Segment->dEventTime = pstruEventDetails->dEventTime;
	pstruTCP_Ack_Segment->nControlDataType = TCP_ACK;
	pstruTCP_Ack_Segment->nSourceId = nSourceID;
	pstruTCP_Ack_Segment->nDestinationId = nDestID;
	pstruTCP_Ack_Segment->nPacketType = PacketType_Control;
	pstruTCP_Ack_Segment->nPacketId = 0;//-1*pstruEventDetails->pPacket->nPacketId;
	pstruTCP_Ack_Segment->nTransmitterId = nSourceID;

	pstruTCP_Ack_Segment->pstruTransportData->dArrivalTime = pstruEventDetails->dEventTime;
	pstruTCP_Ack_Segment->pstruTransportData->dStartTime = pstruEventDetails->pPacket->pstruTransportData->dStartTime;
	pstruTCP_Ack_Segment->pstruTransportData->dEndTime = pstruEventDetails->dEventTime;
	pstruTCP_Ack_Segment->pstruTransportData->dOverhead = TRANSPORT_TCP_OVERHEADS;
	pstruTCP_Ack_Segment->pstruTransportData->dPacketSize = pstruTCP_Ack_Segment->pstruTransportData->dPayload + pstruTCP_Ack_Segment->pstruTransportData->dOverhead;
	pstruTCP_Ack_Segment->pstruTransportData->nSourcePort = usnSourcePort;
	pstruTCP_Ack_Segment->pstruTransportData->nDestinationPort = usnDest_Port;
	pstruTCP_Ack_Segment->pstruTransportData->nTransportProtocol = TX_PROTOCOL_TCP;
	pstruTCP_Ack_Segment->pstruTransportData->Packet_TransportProtocol = pstruAck_Header;

	pstruTCP_Ack_Segment->pstruNetworkData->szSourceIP = IP_COPY(szSourceIP);
	pstruTCP_Ack_Segment->pstruNetworkData->szDestIP = IP_COPY(szDestIP);
	pstruTCP_Ack_Segment->pstruNetworkData->nTTL = MAX_TTL;
	pstruTCP_Ack_Segment->pstruNextPacket = NULL;

	//Specific to NetSim TCP Metrics
	pstruTCP_Connection_Var->pstruMetrics->nAckSent++;

	//Free the Received segment
	//fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

	/* form the event details to add Network out event */
	pstruEventDetails->dPacketSize = pstruTCP_Ack_Segment->pstruTransportData->dPacketSize;
	pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(pstruEventDetails->nDeviceId);
	pstruEventDetails->nPacketId = pstruTCP_Ack_Segment->nPacketId;
	pstruEventDetails->nSegmentId = 0;
	pstruEventDetails->nEventType=NETWORK_OUT_EVENT;
	pstruEventDetails->nSubEventType = 0;
	pstruEventDetails->pPacket =  pstruTCP_Ack_Segment;
	fnpAddEvent(pstruEventDetails);
	return 0;
}
