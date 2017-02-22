
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
* Date  :    29-Oct-2012
* --------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_DEPRECATE

#include "main.h"
#include "TCP.h"

/**	
		This function is called by NetworkStack.dll, whenever the event gets triggered	
		inside the NetworkStack.dll for the Transport layer TCP protocol
		It includes TRANSPORT_OUT,TRANSPORT_IN and TIMER_EVENT.		
 */
_declspec (dllexport) int fn_NetSim_TCP_Run()
{
	/* Get the EventType from Event details */
	SUB_EVENT nSub_Event_Type;
	unsigned int unSegmentType;
	TCB *pstruTCP_Connection_Var=NULL;
	NETSIM_ID nDeviceId;
	unsigned int nSocketId;
	NETSIM_ID usnSourcePort, usnDest_Port;
	NETSIM_IPAddress szSourceIP, szDestIP;
	NetSim_PACKET *pstru_Tmp_Segment;
	NetSim_SOCKETINTERFACE *pstru_Socket_Interface;

	nEventType=pstruEventDetails->nEventType;	/* Get the EventType from Event details */
	nSub_Event_Type = pstruEventDetails->nSubEventType;	/* Get the sub EventType from Event details*/
	nSocketId = (unsigned int)pstruEventDetails->szOtherDetails;

	switch(nEventType)
	{
	case TRANSPORT_OUT_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case TCP_SEND_SEG:
				{
					/* Send the segment */
					fn_NetSim_TCP_SEND_SEG();
					break;
				}
			case TCP_SEND_ACK:
				{
					// Send the ack
					//	fn_NetSim_TCP_SEND_ACK();
					break;
				}
			case TCP_SEND_SYN_ACK:
				{
					// Send syn ack
					fn_NetSim_TCP_SEND_ACK_FOR_SYN(g_ppstru_TCP_Config_Variables);
					break;
				}
			case TCP_SEND_FIN_SEG:
				{
					fn_NetSim_TCP_SEND_FIN_SEG();
					break;
				}
			case TCP_SEND_FIN_ACK:
				{
					fn_NetSim_TCP_SEND_FIN_ACK();
					break;
				}
			case TCP_ACTIVE_OPEN:
				{
					if(pstruEventDetails->pPacket)
					{
						nDeviceId = pstruEventDetails->pPacket->nSourceId;
						szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
						szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
						usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;
						usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort; 

						fn_NetSim_TCP_ACTIVE_OPEN(usnSourcePort,usnDest_Port,szSourceIP,szDestIP,nDeviceId,g_ppstru_TCP_Config_Variables,&g_pstruTCPMetrics);
					}
					break;
				}
			case TCP_PASSIVE_OPEN:
				{
					nDeviceId = pstruEventDetails->pPacket->nDestinationId;
					szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
					szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
					usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort; 
					usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

					fn_NetSim_TCP_PASSIVE_OPEN(usnSourcePort,usnDest_Port,szSourceIP,szDestIP,g_ppstru_TCP_Config_Variables,&g_pstruTCPMetrics);
					break;
				}
			default:
				{
					NETSIM_ID nAppId;
					nAppId = pstruEventDetails->nApplicationId;
					/* Get the socket interface address and assign to local variable */
					pstru_Socket_Interface = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface;
					pstru_Tmp_Segment = pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist;

					if(pstru_Tmp_Segment)
					{
						nDeviceId = pstru_Tmp_Segment->nSourceId;
						szSourceIP = pstru_Tmp_Segment->pstruNetworkData->szSourceIP;
						szDestIP = pstru_Tmp_Segment->pstruNetworkData->szDestIP;
						usnSourcePort = pstru_Tmp_Segment->pstruTransportData->nSourcePort;
						usnDest_Port = pstru_Tmp_Segment->pstruTransportData->nDestinationPort; 

						fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);	

						if(pstruTCP_Connection_Var && pstruTCP_Connection_Var->TCP_Present_Connection_State == TCP_ESTABLISHED)
						{
							pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
							pstruEventDetails->nSubEventType = TCP_SEND_SEG;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);	 
						}
						else if(!pstruTCP_Connection_Var)
						{
							pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
							pstruEventDetails->nSubEventType = TCP_ACTIVE_OPEN;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							/* Copy the packet from socket buffer to eventdetaile packet, for action open */
							if(pstru_Tmp_Segment)
								pstruEventDetails->pPacket = fn_NetSim_Packet_CopyPacket(pstru_Tmp_Segment);
							fnpAddEvent(pstruEventDetails);	 
						}
					}
					break;
				}
			}
			break;
		}
	case TRANSPORT_IN_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case TCP_RECEIVE_SEG:
				{
					NetSim_EVENTDETAILS pevent;					
					memcpy(&pevent,pstruEventDetails,sizeof(pevent));
					fn_NetSim_TCP_RECEIVE_SEG();
					memcpy(pstruEventDetails,&pevent,sizeof(pevent));					
					fn_NetSim_TCP_SEND_ACK();
					break;
				}
			case TCP_RECEIVE_ACK:
				{
					fn_NetSim_TCP_RECEIVE_ACK();
					break;
				}
			case TCP_RECEIVE_SYN_ACK:
				{
					fn_NetSim_TCP_RECEIVE_ACK_FOR_SYN(g_ppstru_TCP_Config_Variables);
					break;
				}
			case TCP_RECIEVE_FIN_SEG:
				{
					fn_NetSim_TCP_RECEIVE_FIN_SEG();
					break;
				}
			case TCP_RECEIVE_FIN_ACK:
				{
					fn_NetSim_TCP_RECEIVE_FIN_ACK();
					break;
				}
			default:
				{
					if(pstruEventDetails->pPacket)
					{
						nDeviceId = pstruEventDetails->pPacket->nDestinationId;
						szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
						szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
						usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort; 
						usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;

						fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);	
					}

					if(pstruTCP_Connection_Var && (pstruTCP_Connection_Var->TCP_Present_Connection_State == TCP_ESTABLISHED || pstruTCP_Connection_Var->TCP_Present_Connection_State == TCP_FIN_WAIT_1))
					{
						unSegmentType = pstruEventDetails->pPacket->nControlDataType; 
						/*check Segment type of TRANSPORT_IN_EVENT*/
						if(unSegmentType == TCP_ACK)
						{
							pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
							pstruEventDetails->nSubEventType = TCP_RECEIVE_ACK;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);	 
						}
						else if(unSegmentType == TCP_FIN)
							goto RECEIVE_FIN;
						else
						{
							pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
							pstruEventDetails->nSubEventType = TCP_RECEIVE_SEG;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);
						}
					}
					//Waiting for connection termination request from remote TCP
					else if(pstruTCP_Connection_Var && (pstruTCP_Connection_Var->TCP_Present_Connection_State == TCP_FIN_WAIT_2))
					{
						unSegmentType = pstruEventDetails->pPacket->nControlDataType; 
						/*check Segment type of TRANSPORT_IN_EVENT*/
						if(unSegmentType == TCP_FIN)
						{
							RECEIVE_FIN:
							pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
							pstruEventDetails->nSubEventType = TCP_RECIEVE_FIN_SEG;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);	
						}
					}
					//Waiting for ack of connection termination request that previously sent to remote TCP
					else if(pstruTCP_Connection_Var && pstruTCP_Connection_Var->TCP_Present_Connection_State == TCP_LAST_ACK)
					{
						unSegmentType = pstruEventDetails->pPacket->nControlDataType; 
						/*check Segment type of TRANSPORT_IN_EVENT*/
						if(unSegmentType == TCP_ACK)
						{
							pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
							pstruEventDetails->nSubEventType = TCP_RECEIVE_FIN_ACK;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);
						}
					}
					else if(pstruTCP_Connection_Var && pstruTCP_Connection_Var->TCP_Present_Connection_State < TCP_ESTABLISHED)
					{
						SEGMENT_HEADER_TCP *pstruSegHeader;

						pstruSegHeader = pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

						if(pstruSegHeader->bSYN && pstruSegHeader->bACK)
						{
							//Receive the syn and send the ack
							pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
							pstruEventDetails->nSubEventType = TCP_SEND_SYN_ACK;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);
						}
						else if(pstruSegHeader->bACK)
						{
							fn_NetSim_TCP_RECEIVE_ACK_FOR_SYN(g_ppstru_TCP_Config_Variables);
							/* Commented by Shashi kant Suman on 25-Apr-2014.
							Adding new event creates problem when ACK and Data packet arrives
							together to transport layer.
							Changed to direct function call

							pstruEventDetails->nEventType = TRANSPORT_IN_EVENT;
							pstruEventDetails->nSubEventType = TCP_RECEIVE_SYN_ACK;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);*/
						}
						else if(pstruSegHeader->bSYN)
						{
							//duplicate syn is received
						}
						else
						{
							//Before connection established, data segment is received
							if(pstruEventDetails->pPacket)
								fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
						}
					}
					else if(!pstruTCP_Connection_Var)
					{
						SEGMENT_HEADER_TCP *pstruSegHeader;
						pstruSegHeader = pstruEventDetails->pPacket->pstruTransportData->Packet_TransportProtocol;

						if(pstruSegHeader->bSYN)
						{
							pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
							pstruEventDetails->nSubEventType = TCP_PASSIVE_OPEN;
							pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
							fnpAddEvent(pstruEventDetails);
						}
					}
					break;
				}
			}
			break;
		}
	case TIMER_EVENT:
		{
			switch(nSub_Event_Type)
			{
			case TCP_RETRANSMISSION_TIMEOUT:
				/*TCP_RETRANSMISSION_TIMEOUT is the subevent of TIMER_EVENT*/
				fn_NetSim_TCP_Retransmission_Timeout(); //Perform segment retransmission
				break;
			case TCP_TIME_WAIT_TIMEOUT:
				{
					NetSim_SOCKETINTERFACE *pstru_Socket_Interface;
					nDeviceId = pstruEventDetails->pPacket->nSourceId;
					szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
					szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
					usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;
					usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort; 

					fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);	

					fn_NetSim_TCP_Change_Connection_State(pstruTCP_Connection_Var,TCP_CLOSED);

					fn_NetSim_TCP_Delete_TCB(pstruTCP_Connection_Var,pstruEventDetails->nDeviceId);

					/* Get the socket interface address and assign to local variable */
					pstru_Socket_Interface = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruSocketInterface;
					
					if(pstru_Socket_Interface->pstruSocketBuffer[nSocketId]->pstruPacketlist)
					{
						pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
						pstruEventDetails->nSubEventType = 0;
						pstruEventDetails->nProtocolId = TX_PROTOCOL_TCP;
						fnpAddEvent(pstruEventDetails);
					}

					//Free received segment
					if(pstruEventDetails->pPacket)
						fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);

					break;
				}
			case TCP_USER_TIME_OUT:
				{
					nDeviceId = pstruEventDetails->pPacket->nSourceId;
					szSourceIP = pstruEventDetails->pPacket->pstruNetworkData->szSourceIP;
					szDestIP = pstruEventDetails->pPacket->pstruNetworkData->szDestIP;
					usnSourcePort = pstruEventDetails->pPacket->pstruTransportData->nSourcePort;
					usnDest_Port = pstruEventDetails->pPacket->pstruTransportData->nDestinationPort; 

					fn_NetSim_TCP_Check_Connection(nDeviceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);	

					fn_NetSim_TCP_Delete_TCB(pstruTCP_Connection_Var,pstruEventDetails->nDeviceId);
					break;
				}
				break;
			default:
				fnNetSimError("TCP-- Unknown sub event type for TCP Timer event\n");
				break;
			}
			break;
		}
	default:
		fnNetSimError("TCP-- Unknown event type for TCP protocol\n");
		break;
	}
	return 0;
}



/**
	This function is called by NetworkStack.dll, while configuring the device 
	TRANSPORT layer for TCP protocol.	
*/
_declspec(dllexport) int fn_NetSim_TCP_Configure(void** var)
{
	return fn_NetSim_TCP_Configure_F(var);
}


/**
	This functon initializes the TCP parameters. 
*/
_declspec (dllexport) int fn_NetSim_TCP_Init(struct stru_NetSim_Network *NETWORK_Formal,\
											 NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,\
											 char *pszWritePath_Formal,int nVersion_Type,void **fnPointer)
{
	fn_NetSim_TCP_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,\
		pszWritePath_Formal,nVersion_Type,fnPointer);

	return 0;
}

/**
	This function is called by NetworkStack.dll, once simulation end to free the 
	allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_TCP_Finish()
{
	fn_NetSim_TCP_Finish_F();
	return 0;
}	

/**
	This function is called by NetworkStack.dll, while writing the evnt trace 
	to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_TCP_Trace(int nSubEvent)
{
	return (fn_NetSim_TCP_Trace_F(nSubEvent));
}

/**
	This function is called by NetworkStack.dll, to free the TCP protocol
	pstruTransportData->Packet_TransportProtocol.
*/
_declspec(dllexport) int fn_NetSim_TCP_FreePacket(NetSim_PACKET* pstruPacket)
{
	return fn_NetSim_TCP_FreePacket_F(pstruPacket);	
}

/**
	This function is called by NetworkStack.dll, to copy the TCP protocol
	pstruTransportData->Packet_TransportProtocol from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_TCP_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	return fn_NetSim_TCP_CopyPacket_F(pstruDestPacket,pstruSrcPacket);	
}

/**
	This function write the Metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_TCP_Metrics(char* szMetrics)
{
	return fn_NetSim_TCP_Metrics_F(szMetrics);	
}


char pszTrace[500];
/**
	This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_TCP_ConfigPacketTrace(const void* xmlNetSimNode)
{
	string szStatus;
	//char *pszTrace;

	*pszTrace = 0;
	//pszTrace = fnpAllocateMemory(1,sizeof(char)+500);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"LOCAL_ADDRESS");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[0] = 1;
		strcat(pszTrace,"LOCAL_ADDRESS,");
	}
	else
		nTCPPacketTraceField[0] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"FOREIGN_ADDRESS");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[1] = 1;
		strcat(pszTrace,"FOREIGN_ADDRESS,");
	}
	else
		nTCPPacketTraceField[1] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"CWND");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[2] = 1;
		strcat(pszTrace,"CWND,");
	}
	else
		nTCPPacketTraceField[2] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"SEQ_NO");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[3] = 1;
		strcat(pszTrace,"SEQ_NO,");
	}
	else
		nTCPPacketTraceField[3] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"ACK_NO");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[4] = 1;
		strcat(pszTrace,"ACK_NO,");
	}
	else
		nTCPPacketTraceField[4] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"RTT");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[5] = 1;
		strcat(pszTrace,"RTT,");
	}
	else
		nTCPPacketTraceField[5] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"RTO");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[6] = 1;
		strcat(pszTrace,"RTO,");
	}
	else
		nTCPPacketTraceField[6] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"CONNECTION_STATE");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		nTCPPacketTraceField[7] = 1;
		strcat(pszTrace,"CONNECTION_STATE,");
	}
	else
		nTCPPacketTraceField[7] = 0;
	free(szStatus);

	return pszTrace;
}


/**
	This function will return the string to write packet trace.																									
*/
_declspec(dllexport) int fn_NetSim_TCP_WritePacketTrace(NetSim_PACKET* pstruPacket,char** ppszTrace)
{
	SEGMENT_HEADER_TCP *pstruSegmentHeader=NULL;
	NETSIM_IPAddress szSourceIP, szDestIP;
	unsigned short int usnSourcePort, usnDest_Port;
	unsigned long int ulnSEQ_NO, ulnACK_NO;
	unsigned int un_cwnd;
	double dRTT, dRTO;
	char *pszTrace, pszConnectionState[50];

	pszTrace = fnpAllocateMemory(1,sizeof(char)+500);
	*ppszTrace = pszTrace;
	if(pstruPacket->pstruTransportData && pstruPacket->pstruTransportData->nTransportProtocol == TX_PROTOCOL_TCP)
	{
		int i=0;
		char szTmpTrace[500];
		char ips[_NETSIM_IP_LEN];
		TCB *pstruTCP_Connection_Var;

		pstruSegmentHeader = pstruPacket->pstruTransportData->Packet_TransportProtocol;
		szSourceIP = pstruPacket->pstruNetworkData->szSourceIP;
		szDestIP = pstruPacket->pstruNetworkData->szDestIP;
		usnSourcePort = pstruPacket->pstruTransportData->nSourcePort;
		usnDest_Port = pstruPacket->pstruTransportData->nDestinationPort;
		fn_NetSim_TCP_Check_Connection(pstruPacket->nSourceId,szSourceIP,szDestIP,usnSourcePort,usnDest_Port,&pstruTCP_Connection_Var);
		un_cwnd = pstruSegmentHeader->nWindow;
		ulnSEQ_NO = pstruSegmentHeader->ulnSequence_Number;
		ulnACK_NO = pstruSegmentHeader->ulnAcknowledgment_Number;
		dRTT = pstruTCP_Connection_Var->dRTT_TIME;
		dRTO = pstruTCP_Connection_Var->dRTO_TIME;
		fn_NetSim_TCP_State_PacketTrace(pstruTCP_Connection_Var->TCP_Present_Connection_State,pszConnectionState);
		
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			IP_TO_STR(szSourceIP,ips);
			sprintf(szTmpTrace,"%s:%d,",ips,usnSourcePort);
			strcpy(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			IP_TO_STR(szDestIP,ips);
			sprintf(szTmpTrace,"%s:%d,",ips,usnDest_Port);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%d,",un_cwnd);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%d,",ulnSEQ_NO);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%d,",ulnACK_NO);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%f,",dRTT);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%f,",dRTO);
			strcat(pszTrace,szTmpTrace);
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			sprintf(szTmpTrace,"%s,",pszConnectionState);
			strcat(pszTrace,szTmpTrace);
		}		
		return 1;
	}
	else
	{
		int i=0;

		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcpy(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}
		if(nTCPPacketTraceField[i++] == ENABLE)
		{
			strcat(pszTrace,"N/A,");
		}		
	}
	return 1;
}

