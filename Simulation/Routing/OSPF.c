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
#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "Routing.h"
/**
	This function is called by NetworkStack.dll, while configuring the device 
	Application layer for OSPF protocol.	
*/
_declspec(dllexport)int fn_NetSim_OSPF_Configure(void** var)
{
	return fn_NetSim_OSPF_Configure_F(var);
}
/**
	This function is called by NetworkStack.dll, while configuring the device 
	Application layer for OSPF protocol.	
*/
_declspec (dllexport) int fn_NetSim_OSPF_Init(struct stru_NetSim_Network *NETWORK_Formal,\
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,\
	char *pszWritePath_Formal,int nVersion_Type,void **fnPointer)
{
	fn_NetSim_OSPF_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,\
		pszWritePath_Formal,nVersion_Type,fnPointer);
	return 0;
}
/**
	This function is called by NetworkStack.dll, once simulation end to free the 
	allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_OSPF_Finish()
{
	fn_NetSim_OSPF_Finish_F();
	return 0;
}	
/**
	This function is called by NetworkStack.dll, while writing the evnt trace 
	to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_OSPF_Trace(int nSubEvent)
{
	return (fn_NetSim_OSPF_Trace_F(nSubEvent));
}
/**
	This function is called by NetworkStack.dll, to free the packet of OSPF protocol
*/
_declspec(dllexport) int fn_NetSim_OSPF_FreePacket(NetSim_PACKET* pstruPacket)
{
	return fn_NetSim_OSPF_FreePacket_F(pstruPacket);	
}
/**
	This function is called by NetworkStack.dll, to copy the packet of OSPF protocol
	 from source to destination.
*/
_declspec(dllexport) int fn_NetSim_OSPF_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	return fn_NetSim_OSPF_CopyPacket_F(pstruDestPacket,pstruSrcPacket);	
}
/**
	This function writes the various metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_OSPF_Metrics(char* szMetrics)
{
	return fn_NetSim_OSPF_Metrics_F(szMetrics);	
}
/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport)char* fn_NetSim_OSPF_ConfigPacketTrace()
{
	return "";
}
/**
 This function will return the string to write packet trace.																									
*/
_declspec(dllexport)char* fn_NetSim_OSPF_WritePacketTrace()
{
	return "";
}
/**
	This function is called by NetworkStack.dll, whenever the event gets triggered	<br/>
	inside the NetworkStack.dll for the Application layer OSPF protocol<br/>
	It includes APPLICATION_OUT,APPLICATION_IN,NETWORK_OUT,NETWORK_IN and TIMER_EVENT.<br/>
		
 */
_declspec(dllexport)int fn_NetSim_OSPF_Run()
{
	if(pstruEventDetails->pPacket && !fnCheckRoutingPacket())
		fnNetSimError("Unknown packet %d received in OSPF protocol for Device %d.",pstruEventDetails->pPacket->nControlDataType,pstruEventDetails->nDeviceId);
	return fnRouter[pstruEventDetails->nProtocolId?pstruEventDetails->nProtocolId%100:pstruEventDetails->pPacket->nControlDataType/100%100]();
}
int fn_NetSim_OSPF_Run_F()
{
	SUB_EVENT_TYPE nSub_Event_Type;
	nEventType=pstruEventDetails->nEventType;
	nSub_Event_Type = pstruEventDetails->nSubEventType;
	//	
	//Check  the event type
	switch(nEventType)	
	{			
	case TIMER_EVENT:
		switch(nSub_Event_Type)
		{
		case WAITTIMER:
			fn_NetSim_OSPF_DesignatedRouterElection(NETWORK,pstruEventDetails);
			break;
		case INACTIVITYTIMER:
			//
			break;	
		default:
			printf("Invalid Subevent\n");
			break;
		}
		break;
	case APPLICATION_OUT_EVENT:
		switch(nSub_Event_Type)
		{
		case INTERFACEUP:
			fn_NetSim_OSPF_Hellopacketformation(NETWORK,pstruEventDetails);
			break;
		case NEGOTIATIONDONE:
			fn_NetSim_OSPF_SendDDPackets(NETWORK,pstruEventDetails);
			break;
		case SEND_LSU:
			fn_NetSim_OSPF_LSUpacketformation(NETWORK,pstruEventDetails); 
			break;
		case SEND_LSA:
			fn_NetSim_OSPF_SendLSAPackets(NETWORK,pstruEventDetails);
			break;
		case EXCHANGEDONE:
			fn_NetSim_OSPF_LSRpacketformation(NETWORK,pstruEventDetails); 
			break; 
		case MAXAGE:
			//
			break;
		default:
			printf("Invalid Subevent\n");
			break;
		}
		break;
	case APPLICATION_IN_EVENT:
		switch(pstruEventDetails->pPacket->nControlDataType)
		{
		case HELLO: 
			fn_NetSim_OSPF_ReceiveHelloPackets(NETWORK,pstruEventDetails);
			break;
		case DATABASE_DESCRIPTION:
			fn_NetSim_OSPF_ReceiveDDPackets(NETWORK,pstruEventDetails);
			break;
		case LINKSTATEREQUEST:
			fn_NetSim_OSPF_ReceiveLSRPackets(NETWORK,pstruEventDetails); 
			break;
		case LINKSTATEUPDATE:
			fn_NetSim_OSPF_ReceiveLSUPackets(NETWORK,pstruEventDetails); 
			fn_NetSim_OSPF_LinkStateDatbaseformation(NETWORK,pstruEventDetails);
			break;
		case LINKSTATEACKNOWLEDGEMENT:
			fn_NetSim_OSPF_ReceiveLSAPackets(NETWORK,pstruEventDetails);
			break;
		default:
			printf("Invalid Packet is received in Router's Application In event\n");
		}
	}
	return 0;
}