/************************************************************************************
* Copyright (C) 2013                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Thangarasu                                                       *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "Routing.h"
#include "List.h"

NETSIM_ID fn_NetSim_FindAS(NETSIM_ID nDeviceId);

int fn_NetSim_BGP_Init_F(struct stru_NetSim_Network *NETWORK_Formal,
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,
	char *pszAppPath_Formal,
	char *pszWritePath_Formal,
	int nVersion_Type,
	void **fnPointer);
int fn_NetSim_BGP_Configure_F(void** var);
int fn_NetSim_BGP_CopyPacket_F(const NetSim_PACKET* destPacket,const NetSim_PACKET* srcPacket);
int fn_NetSim_BGP_FreePacket_F(NetSim_PACKET* packet);
int fn_NetSim_BGP_Metrics_F(char* filename);
int fn_NetSim_BGP_LoadPrimitives(const char* name,const char* path,const char* fun);




/**
	This functon initializes the BGP parameters. 
*/
_declspec(dllexport) int fn_NetSim_BGP_Init(struct stru_NetSim_Network *NETWORK_Formal,
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,
	char *pszAppPath_Formal,
	char *pszWritePath_Formal,
	int nVersion_Type,
	void **fnPointer)
{
	return fn_NetSim_BGP_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,pszWritePath_Formal,nVersion_Type,fnPointer);
}
/**
	This function is called by NetworkStack.dll, while configuring the router 
	for BGP protocol.	
*/
_declspec(dllexport) int fn_NetSim_BGP_Configure(void** var)
{
	return fn_NetSim_BGP_Configure_F(var);
}
int fn_NetSim_BGP_ApplicationIn();

_declspec(dllexport) int fn_NetSim_BGP_Run()
{
	if(pstruEventDetails->pPacket && !fnCheckRoutingPacket())
		fnNetSimError("Unknown packet %d received in BGP protocol for Device %d.",pstruEventDetails->pPacket->nControlDataType,pstruEventDetails->nDeviceId);
	return fnRouter[pstruEventDetails->nProtocolId?pstruEventDetails->nProtocolId%100:pstruEventDetails->pPacket->nControlDataType/100%100]();
}
/**
	This function is called by NetworkStack.dll, while writing the evnt trace 
	to get the sub event as a string.
*/
_declspec(dllexport) char* fn_NetSim_BGP_Trace(NETSIM_ID nSubeventid)
{
	return fn_NetSim_BGP_Trace_F(nSubeventid);
}
/**
	This function is called by NetworkStack.dll, to copy the BGP protocol
	related information to a new packet 
*/
_declspec(dllexport) int fn_NetSim_BGP_CopyPacket(const NetSim_PACKET* destPacket,const NetSim_PACKET* srcPacket)
{
	return fn_NetSim_BGP_CopyPacket_F(destPacket,srcPacket);
}
/**
	This function is called by NetworkStack.dll, to free the BGP protocol control packets.
*/
_declspec(dllexport) int fn_NetSim_BGP_FreePacket(NetSim_PACKET* packet)
{
	return fn_NetSim_BGP_FreePacket_F(packet);
}
/**
	This function writes the BGP metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_BGP_Metrics(char* filename)
{
	return fn_NetSim_BGP_Metrics_F(filename);
}
/**
	This function is called by NetworkStack.dll, once simulation ends, to free the 
	allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_BGP_Finish()
{
	return fn_NetSim_BGP_Finish_F();
}
/**
	This function is used to configure the packet trace
*/
_declspec(dllexport) char* fn_NetSim_BGP_ConfigPacketTrace()
{
	return "";
}
/**
	This function is used to write the packet trace																									
*/
_declspec(dllexport) char* fn_NetSim_BGP_WritePacketTrace()
{
	return "";
}
/**	
		This function is called by NetworkStack.dll, whenever the event gets triggered	
		inside the NetworkStack.dll for BGP protocol
		It includes APPLICATION_OUT,APPLICATION_IN and TIMER events, since the BGP protocol
		is an application layer module
 */
int fn_NetSim_BGP_Run_F()
{
	DEVICE_ROUTER *pstruBGP;
	//Check  the event type
	pstruBGP = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruApplicationLayer->routingVar;
	switch(pstruEventDetails->nEventType)
	{
	case APPLICATION_OUT_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case BGP_UpdateMsg:
				{
					fn_NetSim_BGP_UpdateMessageformation(NETWORK,pstruEventDetails);
				}
				break;
			}
		}
		break;
	case APPLICATION_IN_EVENT:
		{
			switch(pstruEventDetails->pPacket->nControlDataType)
			{
			case ctrlPacket_OPEN:
				pstruBGP->struBGP.bgpPeerInTotalMessages++;
				if(pstruBGP->struBGP.nBGPState==Idle)
				{
					fn_NetSim_BGP_OpenMessageformation(NETWORK,pstruEventDetails);
					pstruBGP->struBGP.nBGPState = OpenSent;

				}
				
				//Free the Open packet
				fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
				pstruEventDetails->pPacket = NULL;
				pstruEventDetails->nEventType=TIMER_EVENT;
				pstruEventDetails->nSubEventType=BGP_KeepaliveTimer_Expires;
				pstruEventDetails->nProtocolId=APP_PROTOCOL_BGP;
				fnpAddEvent(pstruEventDetails);
				break;
			case BGP_BGPOpen_with_DelayOpenTimer_running:
				//
				break;
			case BGP_BGPHeaderErr:
				//
				break;
			case BGP_BGPOpenMsgErr:
				//
				break;
			case BGP_OpenCollisionDump:
				//
				break;
			case BGP_NotifMsgVerErr:
				//
				break;
			case BGP_NotifMsg:
				//
				break;
			case ctrlPacket_KEEPALIVE:
				//
				pstruBGP->struBGP.bgpPeerInTotalMessages++;
				pstruBGP->struBGP.nBGPState = Established;
				//Free the Keepalive packet
				fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
				if(!pstruBGP->struBGP.bgpPeerOutUpdates)
				{
				pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
				pstruEventDetails->nSubEventType=BGP_UpdateMsg;
				pstruEventDetails->nProtocolId=APP_PROTOCOL_BGP;
				fnpAddEvent(pstruEventDetails);
				}
				break;
			case ctrlPacket_UPDATE:
				//
				pstruBGP->struBGP.bgpPeerInTotalMessages++;
				pstruBGP->struBGP.bgpPeerInUpdates++;
				fn_NetSim_BGP_PathVectorRouting(NETWORK,pstruEventDetails);
				break;
			case BGP_UpdateMsgErr:
				//
				break;
			default:
				break;
			}
		}
		break;
	case TIMER_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case BGP_ConnectRetryTimer_Expires:
				//
				break;
			case BGP_HoldTimer_Expires:
				//
				break;
			case BGP_KeepaliveTimer_Expires:
				//
				fn_NetSim_BGP_KeepAliveMessageformation(NETWORK,pstruEventDetails);
				break;
			case BGP_DelayOpenTimer_Expires:
				//
				if(pstruBGP->struBGP.nBGPState==Idle)
				{
					fn_NetSim_BGP_OpenMessageformation(NETWORK,pstruEventDetails);
					pstruBGP->struBGP.nBGPState = OpenSent;
				}
				break;
			case BGP_IdleHoldTimer_Expires:
				//
				break;
			default:
				fprintf(stderr,"\nInvalid sub event in TIMER EVENT\n");
				break;
			}
		}
		break;

	}
	return 0;
}

/**
	This function is to find the autonomous system.
*/
NETSIM_ID fn_NetSim_FindAS(NETSIM_ID nDeviceId)
{
	DEVICE_ROUTER *pstruBGP;
	pstruBGP = NETWORK->ppstruDeviceList[nDeviceId-1]->pstruApplicationLayer->routingVar;
	return pstruBGP->struBGP.nAutonomous_system_number;
}