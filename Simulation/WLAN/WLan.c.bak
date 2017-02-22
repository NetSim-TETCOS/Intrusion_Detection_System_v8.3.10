/************************************************************************************
* Copyright (C) 2013    
*
* TETCOS, Bangalore. India                                                         *

* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *

* Author:   Basamma YB 
*  
************************************************************************************/
#include "main.h"
#include "WLAN.h"

/**
This functon initializes the WLAN parameters. 
*/
_declspec(dllexport) int fn_NetSim_WLAN_Init(struct stru_NetSim_Network *NETWORK_Formal,\
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,\
	char *pszWritePath_Formal,int nVersion_Type,void **fnPointer)
{
	fn_NetSim_WLAN_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,pszWritePath_Formal,nVersion_Type,fnPointer);
	return 1;
}

/**
This function is called by NetworkStack.dll, whenever the event gets triggered		
inside the NetworkStack.dll for the WLAN protocol.									
*/
_declspec (dllexport) int fn_NetSim_WLAN_Run()
{

	pstruEventDetails->nSubEventType = pstruEventDetails->nSubEventType % pstruEventDetails->nProtocolId;

	switch(pstruEventDetails->nEventType)
	{
	case MAC_OUT_EVENT:
		switch(pstruEventDetails->nSubEventType)
		{		
		case DIFS_END:				
			fn_NetSim_WLAN_DIFS();
			break;
		case BACKOFF:		
			if(fn_NetSim_WLAN_BackOff()) // Backoff Successfull
			{
				switch(pstruEventDetails->nSubEventType % pstruEventDetails->nProtocolId)
				{
				case SEND_RTS:	
					//fn_NetSim_WLAN_SendRTS();
					fn_NetSim_WLAN_SendRTS(dCummulativeReceivedPower);
					break;
				case SEND_MPDU:	
				default:
					fn_NetSim_WLAN_SendMPDU(dCummulativeReceivedPower);
					break;	
				}
			}
			break;		
		case SEND_CTS:				
			fn_NetSim_WLAN_SendCTS();
			break;
		case SEND_MPDU:	// After CTS received
			fn_NetSim_WLAN_SendMPDU(dCummulativeReceivedPower);
			break;	
		case SEND_ACK:			
			fn_NetSim_WLAN_SendACK();
			break;	
		case SEND_BLOCK_ACK:
			fn_NetSim_WLAN_SendBlockACK();
			break;
		case CS:			
			if(fn_NetSim_WLAN_CarrierSense()) // Carrier sense successfull
			{
				switch(pstruEventDetails->nSubEventType % pstruEventDetails->nProtocolId)
				{
				case CHECK_NAV:
					fn_NetSim_WLAN_CheckNAV();
					break;
				}
			}
			break;
		default:
			fn_NetSim_WLAN_802_11e_PacketArrive_MacLayer();
			break;
		}
		break;
	case MAC_IN_EVENT:		
		fn_NetSim_WLAN_MacIn();
		switch(pstruEventDetails->pPacket->nControlDataType % (pstruEventDetails->nProtocolId * 100))
		{
		case WLAN_ACK:			
			fn_NetSim_WLAN_ReceiveACK();
			break;
		case WLAN_BlockACK:
			fn_NetSim_WLAN_ReceiveBlockACK();
			break;
		case WLAN_RTS:			
			fn_NetSim_WLAN_ReceiveRTS();
			break;
		case WLAN_CTS:			
			fn_NetSim_WLAN_ReceiveCTS();
			break;
		default:
			fn_NetSim_WLAN_ReceiveMPDU();
			break;
		}
		break;
	case PHYSICAL_OUT_EVENT:
		fn_NetSim_WLAN_PhysicalOut();			
		break;
	case PHYSICAL_IN_EVENT:
		fn_NetSim_WLAN_PhysicalIn();			
		break;
	case TIMER_EVENT:
		switch(pstruEventDetails->nSubEventType)
		{
		case NAV_END:			
			fn_NetSim_WLAN_ResetNAV(pstruEventDetails);
			break;
		case ACK_TIMEOUT:	
		case BLOCK_ACK_TIMEOUT:
			fn_NetSim_WLAN_AckTimeOut();
			break;
		case CTS_TIMEOUT:				
			fn_NetSim_WLAN_CtsTimeOut();
			break;
		case UPDATE_DEVICE_STATUS:			
			fn_NetSim_WLAN_Receiver_UpdateMedium_UpdateDeviceStatus();	
			break;
		default:
			printf("WLAN--- Unknown Timer event type %d \n",pstruEventDetails->nEventType);
			break;
		}
		break;
	default:
		printf("WLAN--- Unknown event type %d \n",pstruEventDetails->nEventType);
		break;
	}
	return 1;
}
/**
This function is called by NetworkStack.dll, once simulation end to free the 
allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_WLAN_Finish()
{
	fn_NetSim_WLAN_Finish_F();
	return 0;
}	
/**
This function is called by NetworkStack.dll, while writing the evnt trace 
to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_WLAN_Trace(int nSubEvent)
{
	return (fn_NetSim_WLAN_Trace_F(nSubEvent));
}
/**
This function is called by NetworkStack.dll, while configuring the device 
MAC and PHYSICAL layer for WLAN protocol.	
*/
_declspec(dllexport) int fn_NetSim_WLAN_Configure(void** var)
{
	return fn_NetSim_WLAN_Configure_F(var);
}
/**
This function is called by NetworkStack.dll, to free the WLAN protocol
pstruMacData->Packet_MACProtocol.
*/
_declspec(dllexport) int fn_NetSim_WLAN_FreePacket(NetSim_PACKET* pstruPacket)
{
	return fn_NetSim_WLAN_FreePacket_F(pstruPacket);	
}
/**
This function is called by NetworkStack.dll, to copy the WLAN protocol
pstruMacData->Packet_MACProtocol from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_WLAN_CopyPacket(NetSim_PACKET* pstruSrcPacket,NetSim_PACKET* pstruDestPacket)
{
	return fn_NetSim_WLAN_CopyPacket_F(pstruSrcPacket,pstruDestPacket);	
}
/**
This function call WLAN Metrics function in lib file to write the Metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_WLAN_Metrics(char* szMetrics)
{
	return fn_NetSim_WLAN_Metrics_F(szMetrics);	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
}

char pszTrace[500];
/**
This function is to configure the WLAN protocol packet trace parameter. 
This function return a string which has the parameters separated by comma.
*/
_declspec(dllexport) char* fn_NetSim_WLAN_ConfigPacketTrace(const void* xmlNetSimNode)
{
	string szStatus;
	//char *pszTrace;
	static int nFlag = 0; // This is to add the packet trace fields only one time.
	if(nFlag)
		return ",";
	*pszTrace = 0;
	//pszTrace = fnpAllocateMemory(1,sizeof(char)+500);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"CW");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		gnWLANPacketTraceField[0] = 1;
		strcat(pszTrace,"CW,");
	}
	else
		gnWLANPacketTraceField[0] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"BO_TIME");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		gnWLANPacketTraceField[1] = 1;
		strcat(pszTrace,"BO_TIME,");
	}
	else
		gnWLANPacketTraceField[1] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"DATARATE");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		gnWLANPacketTraceField[2] = 1;
		strcat(pszTrace,"DATARATE,");
	}
	else
		gnWLANPacketTraceField[2] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"RX_POWER");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		gnWLANPacketTraceField[3] = 1;
		strcat(pszTrace,"RX_POWER,");
	}
	else
		gnWLANPacketTraceField[3] = 0;
	free(szStatus);

	szStatus = fn_NetSim_xmlConfigPacketTraceField(xmlNetSimNode,"RX_SENSITIVITY");
	_strupr(szStatus);
	if(!strcmp(szStatus,"ENABLE"))
	{
		gnWLANPacketTraceField[4] = 1;
		strcat(pszTrace,"RX_SENSITIVITY,");
	}
	else
		gnWLANPacketTraceField[4] = 0;
	free(szStatus);	
	nFlag = 1;
	return pszTrace;	
}
/**
This function is called while writing the Packet trace for WLAN protocol. 
This function is called for every packet while writing the packet trace.
*/
_declspec(dllexport) int fn_NetSim_WLAN_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	int i=0;
	char szTmpTrace[500];
	int nCW;			//sets the value of current contention window size
	double dBackOffTime;
	double dDataRate;	
	double dRxPower;
	double dRxSensitivity;
	char *pszTrace;	
	if(!gn80211PktTraceFlag) // Write packet trace once
	{	
		MAC_VAR *pstruMac = DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
		PHY_VAR *pstruPhy = DEVICE_PHYVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);	
		NETSIM_ID nProtocolId = pstruPacket->pstruMacData->nMACProtocol;
		pszTrace = fnpAllocateMemory(1,sizeof(char)+500);
		*ppszTrace = pszTrace;		
		switch(pstruPacket->pstruMacData->nMACProtocol)
		{
		case MAC_PROTOCOL_IEEE802_11:		
			{
				nCW = pstruMac->nCWcurrent;		
				dRxPower = pstruPhy->dTotalReceivedPower; //10 * log10(pstruPhy->dTotalReceivedPower);
				dRxSensitivity = pstruPhy->dReceiverSensivity;
				//dBackOffTime = pstruMac->dBackOffTime;
				switch(pstruPacket->nControlDataType % (nProtocolId * 100) )
				{
				case WLAN_RTS:
					//dBackOffTime = pstruMac->dBackOffCounter;
					dBackOffTime = ((RTS_FRAME*)(pstruPacket->pstruMacData->Packet_MACProtocol))->dBOTime;   //pstruMac->dBackOffCounter;
					dDataRate = pstruPhy->dControlFrameDataRate;
					break;
				case WLAN_CTS:
					dBackOffTime = 0;
					dDataRate = pstruPhy->dControlFrameDataRate;
					break;
				case WLAN_ACK:
					dBackOffTime = 0;
					dDataRate = pstruPhy->dControlFrameDataRate;
					break;
				case WLAN_BlockACK:
					dBackOffTime = 0;
					dDataRate = pstruPhy->dControlFrameDataRate;
					break;
				default: // Data packets
					if(pstruPacket->pstruMacData->Packet_MACProtocol)//
					{
						dBackOffTime = ((MAC_HEADER*)(pstruPacket->pstruMacData->Packet_MACProtocol))->dBOTime;   //pstruMac->dBackOffCounter;
						dDataRate = pstruPhy->dDataRate;
					}
					else
					{
						dBackOffTime = 0;
						dDataRate = 0;
					}
					break;
				}
				i=0;
				if(gnWLANPacketTraceField[i++] == ENABLE)
				{
					sprintf(szTmpTrace,"%d,",nCW);
					strcpy(pszTrace,szTmpTrace);
				}
				if(gnWLANPacketTraceField[i++] == ENABLE)
				{
					sprintf(szTmpTrace,"%f,",dBackOffTime);
					strcat(pszTrace,szTmpTrace);
				}
				if(gnWLANPacketTraceField[i++] == ENABLE)
				{
					sprintf(szTmpTrace,"%f,",dDataRate);
					strcat(pszTrace,szTmpTrace);
				}
				if(gnWLANPacketTraceField[i++] == ENABLE)
				{
					sprintf(szTmpTrace,"%f,",dRxPower);
					strcat(pszTrace,szTmpTrace);
				}	
				if(gnWLANPacketTraceField[i++] == ENABLE)
				{
					sprintf(szTmpTrace,"%f,",dRxSensitivity);
					strcat(pszTrace,szTmpTrace);
				}
			}			
			break;
		default:
			i=0;
			if(gnWLANPacketTraceField[i++] == ENABLE)
			{			
				strcpy(pszTrace,"N/A,");
			}
			if(gnWLANPacketTraceField[i++] == ENABLE)
			{
				strcat(pszTrace,"N/A,");
			}
			if(gnWLANPacketTraceField[i++] == ENABLE)
			{
				strcat(pszTrace,"N/A,");
			}
			if(gnWLANPacketTraceField[i++] == ENABLE)
			{
				strcat(pszTrace,"N/A,");
			}	
			if(gnWLANPacketTraceField[i++] == ENABLE)
			{			
				strcat(pszTrace,"N/A,");
			}		
			break;
		}
		gn80211PktTraceFlag = 1;
	}
	if(gn80211PktTraceFlag == gn80211ProtocolEnabledCount)
		gn80211PktTraceFlag = 0;
	else
		gn80211PktTraceFlag = gn80211PktTraceFlag + 1;
	return 1;
}

/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function is called when PHYSICAL_OUT_EVENT is triggered.
Calculate the Transmission time of the packet. Add PHYSICAL_IN event at event time + 
transmission time of the packet. If destination id = 0, broadcast the frame else
transmit unicast frame. 
While transmitting check whether the Receiver radio state is CHANNEL_IDLE and also 
is the receiver is reachable. If both condition satisfied then add the PHY IN EVENT,
else drop the frame.


******************Updating the Medium Status**********************
There are 3 functions which are involved in the updating the Medium from idle to busy and vice versa. They are –
1.fn_NetSim_WLAN_CalculateAndSetReceivedPower( )
2.fn_NetSim_WLAN_Transmitter_UpdateMedium_UpdateDeviceStatus(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)
3.fn_NetSim_WLAN_Receiver_UpdateMedium_UpdateDeviceStatus( )

The first function–
fn_NetSim_WLAN_CalculateAndSetReceivedPower( ) is called before the events start to 
calculate and set received power. This function is also called whenever a mobility 
event occurs, since the node positions change and hence received power changes. There 
are two variables defined in this function –
1. dCummulativeReveivedPower[transmitterID][reveiverID]   
2. dCummulativeReveivedPower2[transmitterID][reveiverID] 
Initially when the function is called for the first time, both the variables are 
assigned memory and the received power is calculated and stored according to the 
distance between the devices. When the function,gets called after mobility happens, 
only dCummulativeReveivedPower is updated and not dCummulativeReveivedPower2.


The Second function - 
fn_NetSim_WLAN_Transmitter_UpdateMedium_UpdateDeviceStatus(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)  
is called in PHY_OUT when a particular device wants to transmit a packet. It adds to 
the total_received_power of the PHY_Variable of that device the value 
dCummulativeReveivedPower2[transmitterID][reveiverID].  For any transmitting node 
the received power is set as infinite (a very high number) by adding 100 mw (a very 
high value compared to the usual received power values) the total received power.
In the PHY_OUT, a  timer event is added at the transmission END time so that the 
medium status can be updated upon completion of transmission. 

The Third Function -  
fn_NetSim_WLAN_Receiver_UpdateMedium_UpdateDeviceStatus( ) is called at the timer 
event which is called at the transmission END time. Here, from the total received 
power, the value dCummulativeReveivedPower2[transmitterID][reveiverID] is subtracted 
from the receiver device, and for the transmitter,100 mw is subtracted from the 
totalReceivedPower of the device and dCummulativeReveivedPower2[transmitterID][reveiverID]  
is set to be equal to dCummulativeReveivedPower[transmitterID][reveiverID]  

The reason for using two similar variables is - Suppose a PHY_OUT was called by some 
device and we added dCummulativeReveivedPower[transmitterID][reveiverID]  to 
total_received_power. Now if a mobility event occurs, the cumulative received power i.e. 
dCummulativeReveivedPower gets updated. Now when the transmission timeout happened, 
we subtracted dCummulativeReveivedPower from total_received_power. The difficulty however 
is that what we subtracted was not what we added from total_received_power. This left 
some of device medium permanently busy that causing collisions whenever mobility occurs, 
thus reducing throughput. This was rectified by defining dCummulativeReveivedPower2 which 
is updated to dCummulativeReveivedPower only in the third function i.e  
fn_NetSim_WLAN_Receiver_UpdateMedium_UpdateDeviceStatus() 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_WLAN_PhysicalOut()
{	
	double dTransmissionTime, dTransmissionTime_AMPDU=0;
	NETSIM_ID nProtocolId = pstruEventDetails->nProtocolId;
	double dTime = pstruEventDetails->dEventTime;
	NETSIM_ID nDeviceId = pstruEventDetails->nDeviceId;
	NETSIM_ID nInterfaceId = pstruEventDetails->nInterfaceId;	
	NetSim_PACKET* pstruPacket = pstruEventDetails->pPacket;
	MAC_VAR *pstruMac = DEVICE_MACVAR(nDeviceId,nInterfaceId);
	PHY_VAR	*pstruPhy = DEVICE_PHYVAR(nDeviceId,nInterfaceId);
	NetSim_EVENTDETAILS pevent;
	double dPresentTime=pstruEventDetails->dEventTime;
	double dStartTime,dEndTime;

	if((pstruPhy->nPHYprotocol != IEEE_802_11n && pstruPhy->nPHYprotocol != IEEE_802_11ac)  || pstruEventDetails->pPacket->nDestinationId==0 )
	{
		//Update phy layer details in packet
		pstruPacket->pstruPhyData->dArrivalTime = dTime;
		pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruMacData->dPacketSize;
		pstruPacket->pstruPhyData->dOverhead = 0; 
		pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dOverhead + pstruPacket->pstruPhyData->dPayload;
		pstruPacket->pstruPhyData->dStartTime = pstruEventDetails->dEventTime;	
		pstruPacket->pstruPhyData->nPhyMedium = PHY_MEDIUM_WIRELESS;
		fn_NetSim_WLAN_Add_Phy_Header();
		dTransmissionTime = fn_NetSim_WLAN_CalculateTransmissionTime(pstruPacket,nDeviceId,nInterfaceId,1);
		switch(pstruPacket->nControlDataType % (nProtocolId * 100))
		{
		case WLAN_ACK:
		case WLAN_BlockACK:
		case WLAN_RTS:			
		case WLAN_CTS:			
			break;
		default:		
			pstruMac->pstruMetrics->dTotalTransmissionTime +=  dTransmissionTime;		
			pstruMac->pstruMetrics->dQueuingDelay += pstruPacket->pstruMacData->dStartTime - pstruPacket->pstruMacData->dArrivalTime; 
			pstruMac->pstruMetrics->dMediaAccessTime += pstruPacket->pstruPhyData->dStartTime - pstruPacket->pstruMacData->dStartTime;
			break;
		}
		pstruPacket->pstruPhyData->dEndTime = dTime + dTransmissionTime;	
		// Check for medium
		if(pstruMac->Medium == BUSY)	
			pstruPacket->nPacketStatus = PacketStatus_Collided;
		// Update the Medium
		fn_NetSim_WLAN_Transmitter_UpdateMedium_UpdateDeviceStatus(nDeviceId,nInterfaceId);
		// Add UPDATE_DEVICE_STATUS Timer event
		pstruEventDetails->dEventTime = pstruPacket->pstruPhyData->dEndTime;	
		pstruEventDetails->nDeviceId = nDeviceId;	
		pstruEventDetails->nInterfaceId = nInterfaceId;
		pstruEventDetails->nPacketId = pstruPacket->nPacketId;
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);
		pstruEventDetails->nEventType = TIMER_EVENT;
		pstruEventDetails->nSubEventType = WLAN_SUBEVENT(UPDATE_DEVICE_STATUS,nProtocolId); 
		pstruEventDetails->pPacket = fn_NetSim_Packet_CopyAggregatedPacket(pstruPacket);//NULL;
		fnpAddEvent(pstruEventDetails);
		// Transmit the packet
		if(pstruPacket->nDestinationId)
			fn_NetSim_WLAN_TransmitUnicastFrame(pstruPacket,nDeviceId,nInterfaceId,0);	
		else
		{
			fn_NetSim_WLAN_TransmitBroadCastFrame(pstruPacket,nDeviceId,nInterfaceId,0);
			fn_NetSim_Packet_FreePacket(pstruPacket);
			pstruPacket = NULL;
		}
		return 0;
	}
	else if(pstruPhy->nPHYprotocol==IEEE_802_11n || pstruPhy->nPHYprotocol==IEEE_802_11ac)
	{

		pstruPacket=pstruEventDetails->pPacket;
		memcpy(&pevent,pstruEventDetails,sizeof* pstruEventDetails);
		if(pstruEventDetails->nSubEventType==AMPDU_FRAME) //Akhilesh
		{
			pstruPacket->pstruPhyData->dArrivalTime = dTime;
			pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruMacData->dPacketSize;
			pstruPacket->pstruPhyData->dOverhead = 0; 
			pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dOverhead + pstruPacket->pstruPhyData->dPayload;

			pstruPacket->pstruPhyData->dStartTime = dTime+dTransmissionTime_AMPDU;	
			pstruPacket->pstruPhyData->nPhyMedium = PHY_MEDIUM_WIRELESS;
			dTransmissionTime_AMPDU += fn_NetSim_WLAN_CalculateTransmissionTime(pstruPacket,nDeviceId,nInterfaceId,1);
			pstruPacket->pstruPhyData->dEndTime = dTime + dTransmissionTime_AMPDU;

			pstruPacket=pstruPacket->pstruNextPacket;

			while(pstruPacket)
			{
				pstruPacket->pstruPhyData->dArrivalTime = dTime;
				pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruMacData->dPacketSize;
				pstruPacket->pstruPhyData->dOverhead = 0; 
				pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dOverhead + pstruPacket->pstruPhyData->dPayload;

				pstruPacket->pstruPhyData->dStartTime = dTime+dTransmissionTime_AMPDU;	
				pstruPacket->pstruPhyData->nPhyMedium = PHY_MEDIUM_WIRELESS;
				dTransmissionTime_AMPDU += fn_NetSim_WLAN_CalculateTransmissionTime(pstruPacket,nDeviceId,nInterfaceId,0);
				pstruPacket->pstruPhyData->dEndTime = dTime + dTransmissionTime_AMPDU;

				pstruPacket=pstruPacket->pstruNextPacket;
			}
			fn_NetSim_WLAN_Add_Phy_Header();

			pstruPacket = pstruEventDetails->pPacket;
			switch(pstruPacket->nControlDataType % (nProtocolId * 100))
			{
			case WLAN_ACK:
			case WLAN_BlockACK:
			case WLAN_RTS:			
			case WLAN_CTS:			
				break;
			default:		
				pstruMac->pstruMetrics->dTotalTransmissionTime +=  dTransmissionTime_AMPDU;		
				pstruMac->pstruMetrics->dQueuingDelay += pstruPacket->pstruMacData->dStartTime - pstruPacket->pstruMacData->dArrivalTime; 
				pstruMac->pstruMetrics->dMediaAccessTime += pstruPacket->pstruPhyData->dStartTime - pstruPacket->pstruMacData->dStartTime;
				break;
			}
		}
		// Check for medium
		if(pstruMac->Medium == BUSY)	
		{
			double dPresentTime=pstruEventDetails->dEventTime;
			double dStartTime,dEndTime;
			int nDiffStart, nDiffEnd;
			while(pstruPacket)
			{
				dStartTime=pstruPacket->pstruPhyData->dStartTime;
				dEndTime=pstruPacket->pstruPhyData->dEndTime;

				nDiffStart = (int)(dPresentTime-dStartTime);
				nDiffEnd = (int)(dPresentTime-dEndTime);

				if(nDiffEnd==0 || nDiffStart==0)
				{

					pstruPacket->nPacketStatus = PacketStatus_Collided;


				}
				if(nDiffEnd<0)
					break;
			}
		}

		// Update the Medium
		pstruPacket=pstruEventDetails->pPacket;
		fn_NetSim_WLAN_Transmitter_UpdateMedium_UpdateDeviceStatus(nDeviceId,nInterfaceId);
		// Add UPDATE_DEVICE_STATUS Timer event

		if(pstruEventDetails->nSubEventType==AMPDU_FRAME)
		{

			dEndTime=pstruPacket->pstruPhyData->dEndTime;
			pevent.dEventTime = dEndTime;	
			pevent.nDeviceId = nDeviceId;	
			pevent.nInterfaceId = nInterfaceId;
			pevent.nPacketId = pstruPacket->nPacketId;
			pevent.nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);
			pevent.nEventType = TIMER_EVENT;
			pevent.nSubEventType = WLAN_SUBEVENT(UPDATE_DEVICE_STATUS,nProtocolId); 

			//What is this copy function????????????????????
			pevent.pPacket = fn_NetSim_Packet_CopyPacket(pstruEventDetails->pPacket);//NULL;
			fnpAddEvent(&pevent);

			///Adding Corresponding PHYin to see if packet is collided while receiving
			if(pstruPacket->nDestinationId)
			{
				if(!fn_NetSim_WLAN_TransmitUnicastFrame(pstruPacket,nDeviceId,nInterfaceId,AMPDU_SUBFRAME))
					return 0;
			}



			pstruPacket=pstruPacket->pstruNextPacket;
			while(pstruPacket)
			{
				dStartTime = pstruPacket->pstruPhyData->dStartTime;
				//dPresentTime = pstruEventDetails->dEventTime;
				//nDiffStart = dStartTime-dPresentTime;

				//Adding PHYouts for collision Calculation
				pevent.dEventTime = dStartTime;
				pevent.pPacket=pstruPacket;
				pevent.nPacketId = pstruPacket->nPacketId;
				//	pevent.nSegmentId = pstruPacket->;
				pevent.nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);
				pevent.nDeviceId = nDeviceId;
				pevent.nInterfaceId = nInterfaceId;
				if(pstruPacket->pstruAppData)
					pevent.nSegmentId = pstruPacket->pstruAppData->nSegmentId;
				else
					pevent.nSegmentId = 0;
				pevent.nSubEventType = AMPDU_SUBFRAME;
				pevent.nEventType = PHYSICAL_OUT_EVENT;
				pevent.dPacketSize=pstruPacket->pstruMacData->dPacketSize;
				fnpAddEvent(&pevent);

				// Add UPDATE_DEVICE_STATUS Timer event
				dEndTime=pstruPacket->pstruPhyData->dEndTime;
				pevent.dEventTime = dEndTime;	
				pevent.nDeviceId = nDeviceId;

				pevent.nInterfaceId = nInterfaceId;
				pevent.nPacketId = pstruPacket->nPacketId;
				pevent.nProtocolId = fn_NetSim_Stack_GetMacProtocol(nDeviceId,nInterfaceId);
				pevent.nEventType = TIMER_EVENT;
				pevent.nSubEventType = WLAN_SUBEVENT(UPDATE_DEVICE_STATUS,nProtocolId); 

				//pevent.pPacket = fn_NetSim_Packet_CopyAggregatedPacket(pstruEventDetails->pPacket);//NULL;
				pevent.pPacket = fn_NetSim_Packet_CopyPacket(pstruEventDetails->pPacket);//NULL;
				fnpAddEvent(&pevent);

				///Adding PhyIn for collision calculation
				if(pstruPacket->nDestinationId)
				{
					fn_NetSim_WLAN_TransmitUnicastFrame(pstruPacket,nDeviceId,nInterfaceId,AMPDU_SUBFRAME);	
				}
				pstruPacket=pstruPacket->pstruNextPacket;
			}

		}
		pstruPacket=pstruEventDetails->pPacket;
		// Transmit the packet only when it is AMPDU_FRAME
		if(pstruEventDetails->nSubEventType == AMPDU_FRAME)
		{
			//Note that broadcast Packets wont enter here
			if(pstruPacket->nDestinationId)
				fn_NetSim_WLAN_TransmitUnicastFrame(pstruPacket,nDeviceId,nInterfaceId,AMPDU_FRAME);	

		}
		return 0;
	}
	return 0;
}


/**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This function is called when PHYSICAL_IN_EVENT is triggered.
Check for collision
checking wheather the medium is BYSY or 
PacketStatus is PacketStatus_Collided. 
If any one condition satisfied, write packet trace, calculate metrics, then update 
ransmitter and receiver radio State, then free the packet
Check for Error: Call BER function
If PacketStatus is PacketStatus_Error
write packet trace, calculate metrics, then update transmitter and receiver 
radio State, then free the packet.
If PacketStatus is PacketStatus_NoError
write packet trace, calculate metrics, then update ransmitter and receiver 
radio State, then add MAC_IN_EVENT
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int fn_NetSim_WLAN_PhysicalIn()
{
	PHY_VAR *pstruPhyTransmitter ;
	double dFadingPower = 0.0;
	int nPktStatus = 0;	
	int nCollisionFlag = 0,nOtherStationTransmittingFlag=0;	
	NETSIM_ID nProtocolId = pstruEventDetails->nProtocolId;
	NETSIM_ID nDeviceId = pstruEventDetails->nDeviceId;
	NETSIM_ID nInterfaceId = pstruEventDetails->nInterfaceId;	
	NETSIM_ID nLinkId;
	int nMoveFlag = 1; //Flag to indicate that packet to add MAC IN or not.
	PHY_VAR *pstruPhy = DEVICE_PHYVAR(nDeviceId, nInterfaceId);
	MAC_VAR *pstruMac = DEVICE_MACVAR(nDeviceId, nInterfaceId);
	NetSim_PACKET *pstruPacket = pstruEventDetails->pPacket;
	double dPresentTime=pstruEventDetails->dEventTime;
	double dEndTime;
	int nDiffEnd;
	//pstruPacket->pstruPhyData->dPacketSize= pstruPacket->pstruPhyData->dPacketSize;			
	//fn_NetSim_Assign_RXSensitivityDataRateAtReceiver(nDeviceId, nInterfaceId, pstruPacket);
	nLinkId = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->pstruPhysicalLayer->nLinkId;

	pstruPhyTransmitter = DEVICE_PHYVAR(pstruPacket->nTransmitterId, nInterfaceId);
	if((pstruPhy->nPHYprotocol!=IEEE_802_11n && pstruPhy->nPHYprotocol!=IEEE_802_11ac)||pstruEventDetails->pPacket->nDestinationId==0)
	{

		if(pstruMac->Medium == BUSY || pstruPacket->nPacketStatus == PacketStatus_Collided )
		{	
			// call function to write packet trace and calculate metrics
			pstruPacket->nPacketStatus = PacketStatus_Collided;
			fn_NetSim_WritePacketTrace(pstruPacket);	 		
			fn_NetSim_Metrics_Add(pstruPacket);				
			pstruMac->pstruMetrics->nCollidedFrameCount++;
			// Change Radio states of Transmitter and Receiver
			WLAN_CHANGERADIOSTATE(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE
			WLAN_CHANGERADIOSTATE(nDeviceId,nInterfaceId,RX_ON_IDLE);// nTxFlag = 2 for CHANNEL_IDLE			
			//Free packet
			fn_NetSim_Packet_FreePacket(pstruPacket);				
			pstruPacket = NULL;
			pstruPhy->nCollisionFlag = 0;
			return 0;			
		}
		// calculate BER	
		switch(pstruPacket->nControlDataType % (nProtocolId * 100))
		{
		case WLAN_ACK:
		case WLAN_BlockACK:
		case WLAN_RTS:		
		case WLAN_CTS:
			nPktStatus = PacketStatus_NoError;
			break;
		default:
			pstruPhy->dTotalReceivedPower = dCummulativeReceivedPower[nDeviceId][pstruPacket->nTransmitterId];
			if(pstruPhy->dFadingFigure==1)
			{
				//fading loss function is called before calculating BER because fading is time dependent and needs to be calculated before each packet gets transmitted
				fnCalculateFadingLoss(&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[0],&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[1],&dFadingPower,(int)pstruPhy->dFadingFigure); //this function is used calculate the fading losses
				pstruPhy->dTotalReceivedPower = pstruPhy->dTotalReceivedPower - dFadingPower;
				if(pstruPacket->nTransmitterId == 1 && nDeviceId == 3)
					printf("hello\n");
				fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruPacket);
			}
			else
				fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruPacket);
			pstruPhy->nErrorRate = pstruPhyTransmitter->nErrorRate;
			nPktStatus = fn_NetSim_Packet_DecideError(pstruPhy->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
			//nPktStatus = fn_NetSim_Packet_DecideError(pstruPhyTransmitter->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
			break;
		}
		if(nPktStatus == PacketStatus_Error) 
		{ 
			// call function to write packet trace and calculate metrics
			pstruPacket->pstruPhyData->nPacketErrorFlag = 1;
			pstruPacket->nPacketStatus=PacketStatus_Error;			
			fn_NetSim_WritePacketTrace(pstruPacket);  
			fn_NetSim_Metrics_Add(pstruPacket);
			// Increament Errored frame count
			pstruMac->pstruMetrics->nErroredFrameCount++;			
			nMoveFlag = 0; // To drop the packet
		}
		else //Packet is not error. Continue		
		{
			// call function to write packet trace and calculate metrics
			pstruPacket->pstruPhyData->nPacketErrorFlag = 0;			
			pstruPacket->nPacketStatus =PacketStatus_NoError; 	
			fn_NetSim_WritePacketTrace(pstruPacket); 		
			fn_NetSim_Metrics_Add(pstruPacket);
			nMoveFlag = 1;  // To add MAC_IN_EVENT
		}				
		WLAN_CHANGERADIOSTATE(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE
		WLAN_CHANGERADIOSTATE(pstruPacket->nReceiverId,nInterfaceId,RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE
		if(nMoveFlag)
		{		
			pstruPacket->pstruPhyData->dArrivalTime =pstruPacket->pstruPhyData->dEndTime;
			pstruPacket->pstruPhyData->dStartTime =pstruPacket->pstruPhyData->dEndTime;
			pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruPhyData->dPacketSize - pstruPacket->pstruPhyData->dOverhead;
			pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dPayload;
			pstruPacket->pstruPhyData->dOverhead = 0;
			pstruPacket->pstruPhyData->dEndTime =pstruPacket->pstruPhyData->dEndTime;
			// Add MAC IN event
			switch(pstruPacket->nControlDataType % (nProtocolId * 100))
			{
			case WLAN_ACK:	
				pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_ACK,nProtocolId);
				break;
			case WLAN_BlockACK:
				pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_BLOCK_ACK,nProtocolId);

			case WLAN_RTS:	
				pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_RTS,nProtocolId);
				break;
			case WLAN_CTS:
				pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_CTS,nProtocolId);
				break;
			default:
				pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_MPDU,nProtocolId);
				break;
			}		
			pstruEventDetails->nEventType = MAC_IN_EVENT;
			pstruEventDetails->nDeviceId = nDeviceId;
			pstruEventDetails->nInterfaceId = nInterfaceId;
			pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nDeviceId-1]->nDeviceType;
			pstruEventDetails->pPacket = pstruPacket;
			fnpAddEvent(pstruEventDetails);
		}
		else // Drop the packet
		{
			fn_NetSim_Packet_FreePacket(pstruPacket);
			pstruPacket = NULL;
			if(pstruMac->Medium)
				fn_NetSim_WLAN_CheckPacketsInPacketlist(nDeviceId, nInterfaceId);		
		}	
		return 0;
	}
	else if(pstruPhy->nPHYprotocol==IEEE_802_11n || pstruPhy->nPHYprotocol==IEEE_802_11ac)
	{
		if(pstruEventDetails->nSubEventType==AMPDU_SUBFRAME)
		{
			pstruPacket=pstruEventDetails->pPacket;
			while(pstruPacket)
			{
				dEndTime=pstruPacket->pstruPhyData->dEndTime;
				nDiffEnd=(int)(dPresentTime - dEndTime);
				if(nDiffEnd==0)
				{

					if(pstruMac->Medium == BUSY || pstruPacket->nPacketStatus == PacketStatus_Collided )
					{	
						// call function to write packet trace and calculate metrics
						pstruPacket->nPacketStatus = PacketStatus_Collided;
						fn_NetSim_WritePacketTrace(pstruPacket);	 		
						fn_NetSim_Metrics_Add(pstruPacket);				
						pstruMac->pstruMetrics->nCollidedFrameCount++;
						// Change Radio states of Transmitter and Receiver
						WLAN_CHANGERADIOSTATE(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE
						WLAN_CHANGERADIOSTATE(nDeviceId,nInterfaceId,RX_ON_IDLE);// nTxFlag = 2 for CHANNEL_IDLE			
						//Free packet
						//fn_NetSim_Packet_FreePacket(pstruPacket);				
						//pstruPacket = NULL;
						pstruPhy->nCollisionFlag = 0;
						return 0;			
					}

				}
				if(nDiffEnd<0)
					break;
				pstruPacket=pstruPacket->pstruNextPacket;
			}

			return 0;			
		}
		else if(pstruEventDetails->nSubEventType==AMPDU_FRAME)
		{

			pstruPacket=pstruEventDetails->pPacket;
			pstruPacket->pstruPhyData->dArrivalTime =pstruPacket->pstruPhyData->dEndTime;
			pstruPacket->pstruPhyData->dStartTime =pstruPacket->pstruPhyData->dEndTime;
			pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruPhyData->dPacketSize - pstruPacket->pstruPhyData->dOverhead;
			pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dPayload;
			pstruPacket->pstruPhyData->dOverhead = 0;
			pstruPacket->pstruPhyData->dEndTime =pstruPacket->pstruPhyData->dEndTime;

			//If first packet is errored, the so are all the packets
			{
				// calculate BER for first packet	
				switch(pstruPacket->nControlDataType % (nProtocolId * 100))
				{
				case WLAN_ACK:
				case WLAN_BlockACK:
				case WLAN_RTS:		
				case WLAN_CTS:
					nPktStatus = PacketStatus_NoError;
					break;
				default:
					pstruPhy->dTotalReceivedPower = dCummulativeReceivedPower[nDeviceId][pstruPacket->nTransmitterId];
					if(pstruPhy->dFadingFigure==1)
					{
						//fading loss function is called before calculating BER because fading is time dependent and needs to be calculated before each packet gets transmitted
						fnCalculateFadingLoss(&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[0],&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[1],&dFadingPower,(int)pstruPhy->dFadingFigure); //this function is used calculate the fading losses
						pstruPhy->dTotalReceivedPower = pstruPhy->dTotalReceivedPower - dFadingPower;
						fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruEventDetails->pPacket);
					}
					else
						fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruEventDetails->pPacket);
					pstruPhy->nErrorRate = pstruPhyTransmitter->nErrorRate;
					nPktStatus = fn_NetSim_Packet_DecideError(pstruPhy->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
					//nPktStatus = fn_NetSim_Packet_DecideError(pstruPhyTransmitter->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
					break;
				}


				// call function to write packet trace and calculate metrics
				if(nPktStatus == PacketStatus_Error)
				{
					pstruPacket->pstruPhyData->nPacketErrorFlag = 1;
					pstruPacket->nPacketStatus=PacketStatus_Error;
					pstruMac->pstruMetrics->nErroredFrameCount++;	
				}
				else
				{
					pstruPacket->pstruPhyData->nPacketErrorFlag = 0;			
					pstruPacket->nPacketStatus =PacketStatus_NoError; 	
				}
				fn_NetSim_WritePacketTrace(pstruPacket);  
				fn_NetSim_Metrics_Add(pstruPacket);
				// Increament Errored frame count
				//
			}
			pstruPacket=pstruPacket->pstruNextPacket;
			if(nPktStatus!=PacketStatus_Error)
			{
				//If first packet is not errored, then check for the rest packets
				//else make all the packets as errored
				while(pstruPacket)
				{	pstruPacket->pstruPhyData->dArrivalTime =pstruPacket->pstruPhyData->dEndTime;
				pstruPacket->pstruPhyData->dStartTime =pstruPacket->pstruPhyData->dEndTime;
				pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruPhyData->dPacketSize - pstruPacket->pstruPhyData->dOverhead;
				pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dPayload;
				pstruPacket->pstruPhyData->dOverhead = 0;
				pstruPacket->pstruPhyData->dEndTime =pstruPacket->pstruPhyData->dEndTime;


				// calculate BER	
				switch(pstruPacket->nControlDataType % (nProtocolId * 100))
				{
				case WLAN_ACK:
				case WLAN_BlockACK:
				case WLAN_RTS:		
				case WLAN_CTS:
					nPktStatus = PacketStatus_NoError;
					break;
				default:
					pstruPhy->dTotalReceivedPower = dCummulativeReceivedPower[nDeviceId][pstruPacket->nTransmitterId];
					if(pstruPhy->dFadingFigure==1)
					{
						//fading loss function is called before calculating BER because fading is time dependent and needs to be calculated before each packet gets transmitted
						fnCalculateFadingLoss(&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[0],&NETWORK->ppstruDeviceList[nDeviceId-1]->ulSeed[1],&dFadingPower,(int)pstruPhy->dFadingFigure); //this function is used calculate the fading losses
						pstruPhy->dTotalReceivedPower = pstruPhy->dTotalReceivedPower - dFadingPower;
						fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruEventDetails->pPacket);
					}
					else
						fn_NetSim_WLAN_CalculateBER(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),pstruEventDetails->pPacket);
					pstruPhy->nErrorRate = pstruPhyTransmitter->nErrorRate;
					nPktStatus = fn_NetSim_Packet_DecideError(pstruPhy->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
					//nPktStatus = fn_NetSim_Packet_DecideError(pstruPhyTransmitter->nErrorRate,pstruPacket->pstruPhyData->dPacketSize);
					break;
				}


				// call function to write packet trace and calculate metrics
				if(nPktStatus == PacketStatus_Error)
				{
					pstruPacket->pstruPhyData->nPacketErrorFlag = 1;
					pstruPacket->nPacketStatus=PacketStatus_Error;
					pstruMac->pstruMetrics->nErroredFrameCount++;	
				}
				else
				{
					pstruPacket->pstruPhyData->nPacketErrorFlag = 0;			
					pstruPacket->nPacketStatus =PacketStatus_NoError; 	
				}
				fn_NetSim_WritePacketTrace(pstruPacket);  
				fn_NetSim_Metrics_Add(pstruPacket);
				// Increament Errored frame count
				//		

				pstruPacket=pstruPacket->pstruNextPacket;
				}
			}
			else
			{
				//all the packets errored
				while(pstruPacket)
				{	
					pstruPacket->pstruPhyData->dArrivalTime =pstruPacket->pstruPhyData->dEndTime;
					pstruPacket->pstruPhyData->dStartTime =pstruPacket->pstruPhyData->dEndTime;
					pstruPacket->pstruPhyData->dPayload = pstruPacket->pstruPhyData->dPacketSize - pstruPacket->pstruPhyData->dOverhead;
					pstruPacket->pstruPhyData->dPacketSize = pstruPacket->pstruPhyData->dPayload;
					pstruPacket->pstruPhyData->dOverhead = 0;
					pstruPacket->pstruPhyData->dEndTime =pstruPacket->pstruPhyData->dEndTime;


					// call function to write packet trace and calculate metrics		if(nPktStatus == PacketStatus_Error)

					pstruPacket->pstruPhyData->nPacketErrorFlag = 1;
					pstruPacket->nPacketStatus=PacketStatus_Error;
					pstruMac->pstruMetrics->nErroredFrameCount++;	
					fn_NetSim_WritePacketTrace(pstruPacket);  
					fn_NetSim_Metrics_Add(pstruPacket);
					// Increament Errored frame count

					pstruPacket=pstruPacket->pstruNextPacket;
				}
			}

			pstruPacket=pstruEventDetails->pPacket;
			WLAN_CHANGERADIOSTATE(pstruPacket->nTransmitterId,fn_NetSim_Stack_GetWirelessInterface(nLinkId,pstruPacket->nTransmitterId),RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE
			WLAN_CHANGERADIOSTATE(pstruPacket->nReceiverId,nInterfaceId,RX_ON_IDLE);	// nTxFlag = 2 for CHANNEL_IDLE

			{		
				pstruPacket=pstruEventDetails->pPacket;
				// Add MAC IN event
				pstruPacket=pstruEventDetails->pPacket;
				switch(pstruPacket->nControlDataType % (nProtocolId * 100))
				{
				case WLAN_ACK:	
					pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_ACK,nProtocolId);
					break;
				case WLAN_BlockACK:
					pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_BLOCK_ACK,nProtocolId);
					break;
				case WLAN_RTS:	
					pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_RTS,nProtocolId);
					break;
				case WLAN_CTS:
					pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_CTS,nProtocolId);
					break;
				default:
					pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_MPDU,nProtocolId);
					break;
				}		
				pstruEventDetails->nEventType = MAC_IN_EVENT;
				pstruEventDetails->nDeviceId = nDeviceId;
				pstruEventDetails->nInterfaceId = nInterfaceId;
				pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nDeviceId-1]->nDeviceType;
				//pstruEventDetails->pPacket = pstruPacket;
				fnpAddEvent(pstruEventDetails);
			}

			return 0;
		}

		return 0;
	}
	return 0;
}

/**
This function is called when MAC_IN_EVENT is triggered.
It checks the frame type and update the relevant sub event to process the frame.
*/
int fn_NetSim_WLAN_MacIn()
{
	NetSim_PACKET *pstruPacket = pstruEventDetails->pPacket;
	NETSIM_ID nProtocolId = pstruEventDetails->nProtocolId;	
	switch(nProtocolId)
	{
	case MAC_PROTOCOL_IEEE802_11:	
		switch(pstruPacket->nControlDataType % (nProtocolId * 100))
		{
		case WLAN_ACK:
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_ACK,nProtocolId);
			break;
		case WLAN_BlockACK:
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_BLOCK_ACK,nProtocolId);
			break;
		case WLAN_RTS:
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_RTS,nProtocolId);			
			break;
		case WLAN_CTS:
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_CTS,nProtocolId);			
			break;
		default:
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(RECEIVE_MPDU,nProtocolId);			
			break;
		}
		break;
	default:
		printf("WLAN--- Unknown protocol MAC_IN function %d \n",nProtocolId);
		break;
	}		
	return 0;
}

/**
This function is called when UPDATE_DEVICE_STATUS sub event is trigged.
This event is added in the PHYSICAL_OUT event at the PHYSICAL_IN_EVENT time.
Function used to update the medium at PHYSICAL IN event time. Check any change in 
the Medium before (previous) and present when the nDevice reached the receiver. 
If any change then add the CS event to satrt transmission by other devices.This 
event resume the BACKOFF and start transmission of other devices ifthey have
packet to transmit. 
*/

int	fn_NetSim_WLAN_Receiver_UpdateMedium_UpdateDeviceStatus()
{
	NETSIM_ID nDeviceId = pstruEventDetails->nDeviceId;
	NETSIM_ID nInterfaceId = pstruEventDetails->nInterfaceId;
	NetSim_PACKET *pstruPacket, *pstruPacketHead;
	NetSim_PACKET *pstruTempPacket = pstruEventDetails->pPacket;
	CCA_STATUS nPrevMedium;
	NETSIM_ID nLoop, nLoopInterface;
	PHY_VAR *pstruPhy;
	MAC_VAR *pstruMac;
	NETSIM_ID nLinkId;

	// Get the linkId
	nLinkId = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->pstruPhysicalLayer->nLinkId;
	for(nLoop = 1;nLoop <= NETWORK->nDeviceCount;nLoop++)
	{
		//if(nDeviceId == nLoop)
		//	continue;	
		for(nLoopInterface = 1;nLoopInterface<= NETWORK->ppstruDeviceList[nLoop-1]->nNumOfInterface;nLoopInterface++)
		{
			if(!NETWORK->ppstruDeviceList[nLoop-1]->ppstruInterfaceList[nLoopInterface-1]->pstruMACLayer)
				continue;			
			if((DEVICE_MACLAYER(nLoop,nLoopInterface)->nMacProtocolId != MAC_PROTOCOL_IEEE802_11) && \
				(nLinkId != NETWORK->ppstruDeviceList[nLoop-1]->ppstruInterfaceList[nLoopInterface-1]->pstruPhysicalLayer->nLinkId ))
				continue;	
			pstruMac = DEVICE_MACVAR(nLoop,nLoopInterface);
			pstruPhy = DEVICE_PHYVAR(nLoop,nLoopInterface);				
			nPrevMedium = pstruMac->Medium;

			//ReceivedPowerCalculation
			//Subtract the Received Power of transmitter by the transmitter(i.e. 1)
			if(nDeviceId == nLoop)
			{
				pstruPhy->dTotalRxPower_mw -=100;
				continue;
			}
			pstruPhy->dTotalRxPower_mw -= pow(10,dCummulativeReceivedPower2[nDeviceId][nLoop]/10);	
			//Update the dCummulativeReceivedPower2 variable
			dCummulativeReceivedPower2[nDeviceId][nLoop]=dCummulativeReceivedPower[nDeviceId][nLoop];
			if(pstruPhy->dTotalRxPower_mw < pow(10,pstruPhy->dReceiverSensivity/10))
			{
				pstruMac->Medium = IDLE;	
				pstruPhy->dTotalRxPower_mw = 0.0;
			}
			else
			{
				pstruMac->Medium = BUSY;
			}

			if(nPrevMedium != pstruMac->Medium)
			{
				if(pstruMac->CurrentState == MAC_IDLE && pstruMac->Medium == IDLE && pstruPhy->RadioState != RX_ON_BUSY)
				{		
					if(pstruMac->pstruTempData)
						pstruPacket = pstruMac->pstruTempData;
					else					
						pstruPacket = fn_NetSim_WLAN_Packet_GetPacketFromBuffer(pstruMac->pstruQosList,0);

					if(pstruPacket)
					{
						pstruEventDetails->nDeviceId = nLoop;
						pstruEventDetails->nInterfaceId = nLoopInterface;
						pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nLoop-1]->nDeviceType;
						pstruEventDetails->pPacket = pstruPacket;
						pstruEventDetails->nPacketId = pstruPacket->nPacketId;
						if(pstruPacket->pstruAppData)
						{
							pstruEventDetails->nApplicationId = pstruPacket->pstruAppData->nApplicationId;
							pstruEventDetails->nSegmentId = pstruPacket->pstruAppData->nSegmentId;
						}
						else
						{
							pstruEventDetails->nApplicationId = 0;
							pstruEventDetails->nSegmentId = 0;
						}
						if(pstruPacket->pstruNetworkData) // Data packets
							pstruEventDetails->dPacketSize = pstruPacket->pstruNetworkData->dPacketSize;
						else // WLAN Control packets
							pstruEventDetails->dPacketSize = pstruPacket->pstruMacData->dPacketSize;
						pstruEventDetails->nEventType = MAC_OUT_EVENT;
						pstruEventDetails->nSubEventType = WLAN_SUBEVENT(CS,pstruEventDetails->nProtocolId);//CS;
						fnpAddEvent(pstruEventDetails);	
					}
				}
			}			
		}
	}
	// For the Transmitter 
	pstruMac = DEVICE_MACVAR(nDeviceId,nInterfaceId);
	pstruPhy = DEVICE_PHYVAR(nDeviceId,nInterfaceId);
	if(pstruPhy->dTotalRxPower_mw < pow(10,pstruPhy->dReceiverSensivity/10))
	{
		pstruMac->Medium = IDLE;	
		pstruPhy->dTotalRxPower_mw = 0.0;
	}

	if(pstruMac->CurrentState == MAC_IDLE && pstruMac->Medium == IDLE && pstruPhy->RadioState != RX_ON_BUSY)
	{		
		if(pstruMac->pstruTempData)
			pstruPacket = pstruMac->pstruTempData;
		else		
			pstruPacket = fn_NetSim_WLAN_Packet_GetPacketFromBuffer(pstruMac->pstruQosList,0);				
		if(pstruPacket)
		{
			pstruEventDetails->nDeviceId = nDeviceId;
			pstruEventDetails->nInterfaceId = nInterfaceId;
			pstruEventDetails->pPacket = pstruPacket;
			pstruEventDetails->nPacketId = pstruPacket->nPacketId;
			if(pstruPacket->pstruAppData)
				pstruEventDetails->nSegmentId = pstruPacket->pstruAppData->nSegmentId;
			else
				pstruEventDetails->nSegmentId = 0;
			if(pstruPacket->pstruNetworkData) // Data packets
				pstruEventDetails->dPacketSize = pstruPacket->pstruNetworkData->dPacketSize;
			else // WLAN Control packets
				pstruEventDetails->dPacketSize = pstruPacket->pstruMacData->dPacketSize;
			pstruEventDetails->nEventType = MAC_OUT_EVENT;
			pstruEventDetails->nSubEventType = WLAN_SUBEVENT(CS,pstruEventDetails->nProtocolId);//CS;
			fnpAddEvent(pstruEventDetails);
		}
	}

	if(pstruPhy->RadioState != RX_ON_BUSY || pstruPhy->RadioState != RX_OFF)
	{
		WLAN_CHANGERADIOSTATE(nDeviceId,nInterfaceId,RX_ON_IDLE);
	}
	if(pstruTempPacket)
	{
		pstruPacketHead=pstruTempPacket;
		while(pstruPacketHead)
		{ 
			pstruTempPacket=pstruPacketHead;
			pstruPacketHead=pstruPacketHead->pstruNextPacket;
			pstruTempPacket->pstruNextPacket=NULL;
			fn_NetSim_Packet_FreePacket(pstruTempPacket);
			pstruTempPacket = NULL;
		}
	}
	pstruEventDetails->pPacket=NULL;

	return 0;	
}

FILE *fp;
/**
Function used to update the medium during the PHYSICAL OUT event. Check 
change in the Medium before (previous) and present when the nDevice started 
transmission. If any change then, if the device in the BACKING_OFF state, then 
pauseBackoff.  
*/
int fn_NetSim_WLAN_Transmitter_UpdateMedium_UpdateDeviceStatus(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)
{
	double dTime = pstruEventDetails->dEventTime;
	CCA_STATUS nPrevMedium;
	NETSIM_ID nLoop, nLoopInterface;
	MAC_VAR *pstruMac;
	PHY_VAR *pstruPhy;



	NETSIM_ID nLinkId = NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->pstruPhysicalLayer->nLinkId;


	for(nLoop = 1;nLoop <= NETWORK->nDeviceCount;nLoop++)
	{
		for(nLoopInterface = 1;nLoopInterface<= NETWORK->ppstruDeviceList[nLoop-1]->nNumOfInterface;nLoopInterface++)
		{
			if(!NETWORK->ppstruDeviceList[nLoop-1]->ppstruInterfaceList[nLoopInterface-1]->pstruMACLayer)
				continue;			
			if(DEVICE_MACLAYER(nLoop,nLoopInterface)->nMacProtocolId != MAC_PROTOCOL_IEEE802_11)
				continue;
			if(nLinkId != NETWORK->ppstruDeviceList[nLoop-1]->ppstruInterfaceList[nLoopInterface-1]->pstruPhysicalLayer->nLinkId)
				continue;			
			pstruMac = DEVICE_MACVAR(nLoop,nLoopInterface);

			pstruPhy = DEVICE_PHYVAR(nLoop,nLoopInterface);
			nPrevMedium = pstruMac->Medium;	
			//ReceivedPower
			//Make the Received Power of transmitter to be infinite
			if(nDeviceId == nLoop)
			{
				pstruPhy->dTotalRxPower_mw +=100 ;
				continue;
			}

			pstruPhy->dTotalRxPower_mw += pow(10,dCummulativeReceivedPower2[nDeviceId][nLoop]/10);
			if(pstruPhy->dTotalRxPower_mw < pow(10,pstruPhy->dReceiverSensivity/10))
				pstruMac->Medium = IDLE;		
			else	
				pstruMac->Medium = BUSY;
			if(nPrevMedium != pstruMac->Medium)			
			{			
				pstruMac->Medium = BUSY;
				if(pstruMac->CurrentState == BACKING_OFF)
					fn_NetSim_WLAN_PauseBackOff(nLoop,nLoopInterface,dTime);	
			}		
		}
	}
	pstruMac = DEVICE_MACVAR(nDeviceId,nInterfaceId);	
	pstruMac->Medium = BUSY;


	return 0;
}


