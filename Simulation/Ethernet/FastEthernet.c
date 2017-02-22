/*-----------------------------------------------------------------------------------
 * Copyright (C) 2012     
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    Thamothara Kannan      
 * Date  :    23-April-2012
 * ---------------------------------------------------------------------------------*/

/************************************************************************************
 *  																				*
 *  						Revision History										*
 *	REVISION NUMBER:- 1																*
 *																					*	
 *	Review Date :-  29-Jun-2012														*
 *	Reviewed By :-  Pranav, Shashikanth,TV,Basamma									*
 *																					*
 *	REVISION NUMBER:- 2																*
 *  Review Date :- 	06-July-2012													*
 *	Reviewed By :-  Pranav, Shashikanth,TV,Basamma									*
 *
 *	REVISION NUMBER:- 3																*
 *  Review Date :- 	21-July-2012													*
 *	Reviewed By :-  Pranav, Shashikanth												*
 * 
 *	REVISION NUMBER:- 4																*
 *  Review Date :- 	24-July-2012													*
 *	Reviewed By :-  Pranav, Shashikanth												*
 *																					*
 ************************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include "main.h"
#include "Main_FE.h"

/**
		This function is called by NetworkStack.dll, whenever the event gets triggered		
		inside the NetworkStack.dll for the MAC layer FastEthernet protocol						
*/

_declspec (dllexport) int fn_NetSim_Ethernet_Run()
{
	//static int nDisplay_Spanning_Flag=0;
	
	nEventType=pstruEventDetails->nEventType;			/* Get the EventType from Event details */
	nSub_Event_Type=pstruEventDetails->nSubEventType;	/* Get the sub EventType from Event details*/
	

	/*Check  event type*/
	switch(nEventType)	
	{	
	case MAC_OUT_EVENT:
		{
		/*check sub event type of MAC_OUT_EVENT*/
		switch(nSub_Event_Type)		
		{
		case CARRIER_SENSE:		/*Carrier Sense is the subevent of MAC_OUT_EVENT*/
			fn_NetSim_FastEthernet_Carrier_Sense(pstruEventDetails);
			
			break;

		case WAIT_FOR_IFG:			/*Interframe gap is subevent of MAC_OUT_EVENT*/
			fn_NetSim_FastEthernet_IFG(pstruEventDetails);
			
			break;

		case SPANNING_TREE_PROTOCOL:	/*Spanning tree is subevent of MAC_OUT_EVENT*/
			
			 fn_NetSim_FastEthernet_Spanning_Tree_Protocol(pstruEventDetails);
			break;

		case GET_PACKET:	/* Get the packet from Upper layer, invoke the first subevent type of Fast Ethernet protocol*/
			if(fn_NetSim_GetBufferStatus(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->pstruAccessInterface->pstruAccessBuffer))
			{
				pstruEventDetails->nSubEventType=CARRIER_SENSE;
				fnpAddEvent(pstruEventDetails);
			}
			break;

		default:
			break;
		}
		break;
		}
	case MAC_IN_EVENT:
		{
			/*check sub event type of MAC_IN_EVENT*/
			switch (pstruEventDetails->nSubEventType)		
			{
			case SWITCHING_TECHNIQUE:/*Switching Technique is subevent of MAC_IN_EVENT */

				/*Performs switching using one of the techniques*/
					fn_NetSim_FastEthernet_Switching_Techniques(pstruEventDetails);
				break;
				/* Performs SwitchTable Formation*/
			case SWITCH_TABLE_FORMATION:	/*Switching Table Formation is subevent of MAC_IN_EVENT */			
				fn_NetSim_FastEthernet_Switch_Table_Formation(pstruEventDetails);
				break;
				/* switch forwards the frame to respective port*/
			case SWITCH_FRAME_FORWARDING:	/*Switch frame Forwarding is subevent of MAC_IN_EVENT */
				fn_NetSim_FastEthernet_Switch_Frame_Forwarding(pstruEventDetails);
				break;

			case SPANNING_TREE_PROTOCOL: /*Spanning Tree protocol is subevent of MAC_IN_EVENT */
				/*Call the STP function to receive CBPDU and update present CBPDU*/
			 fn_NetSim_FastEthernet_Spanning_Tree_Protocol(pstruEventDetails);
				break;
			case GET_PACKET:

				if(pstruEventDetails->nDeviceType ==SWITCH)
				{
					/* If packet Type is CBPDU*/
					if(((FRAME_MAC *)(pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol))->nPacket_Type == CBPDU_PACKET)
						pstruEventDetails->nSubEventType=SPANNING_TREE_PROTOCOL;
					else /* if Packet Type is DATA*/
						pstruEventDetails->nSubEventType=SWITCHING_TECHNIQUE;
					fnpAddEvent(pstruEventDetails);

				}
				/* Call this function to forward the frame to next layer in the Device */
				else if(pstruEventDetails->nDeviceType != SWITCH)
					fn_NetSim_FastEthernet_Frame_Forward_MacIn(pstruEventDetails);


				break;

			default:
				break;
			}
			break;
		}
	case PHYSICAL_OUT_EVENT:
		{
			switch(nSub_Event_Type)
			{
			case ZERO:
				 fn_NetSim_FastEthernet_Physical_Out_Event();
				break;
				/* Check the link status, after transmitting the frame */
			case PHY_SENSE_LINK:
				fn_NetSim_FastEthernet_Phy_Sense_Link();
				break;
			default:
				break;
			}
			break;
		}
	case PHYSICAL_IN_EVENT: 
		{
			 fn_NetSim_FastEthernet_Physical_In_Event();
				break;
		}
			default:
				break;
	}

	return 0;
}
/**
 The Following function is present in the Ethernet.lib.						
 so NetworkStack.dll can not call Ethernet.lib, NetworkStack.dll is calling	
 libEthernet.dll. From libEthernet.dll, Ethernet.lib functions are called.	
*/
_declspec(dllexport) int fn_NetSim_Ethernet_Init(struct stru_NetSim_Network *NETWORK_Formal,\
 NetSim_EVENTDETAILS *pstruEventDetails_Formal,const char *pszAppPath_Formal,\
 const char *pszIOPath_Formal,int nVersion_Type)
{
	fn_NetSim_Ethernet_Init_F(NETWORK_Formal,pstruEventDetails_Formal,pszAppPath_Formal,\
		pszIOPath_Formal,nVersion_Type);
	return 0;
}
/**
	This function is called by NetworkStack.dll, while writing the evnt trace 
	to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_Ethernet_Trace(int nSubEvent)
{

	return (fn_NetSim_Ethernet_Trace_F(nSubEvent));

}
/**
This function write the Metrics in Metrics.txt	
*/
_declspec (dllexport) int fn_NetSim_Ethernet_Metrics(char* szMetricsFile)
{
	fn_NetSim_FastEthernet_Switch_Display_Spanning_Tree();
	return 1;
}
/**
	This function is called by NetworkStack.dll, to free the Ehernet protocol.
*/
_declspec (dllexport) int fn_NetSim_Ethernet_FreePacket(NetSim_PACKET* pstruNetSimPacket)
{

	fn_NetSim_Ethernet_FreePacket_F(pstruNetSimPacket);
	return 0;
}
/**
	This function is called by NetworkStack.dll, to copy the Ehernet protocol
	details from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_Ethernet_CopyPacket(NetSim_PACKET *pstruDestinationPacket,NetSim_PACKET *pstruSourcePacket)
{
	fn_NetSim_Ethernet_CopyPacket_F(pstruDestinationPacket,pstruSourcePacket);
	return 0;
}
/**
	This function is called by NetworkStack.dll, once simulation end to free the 
	allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_Ethernet_Finish()
{
		fn_NetSim_Ethernet_Finish_F();
		return 0;
}
/**
	This function is called by NetworkStack.dll, while configuring the device 
	for Ehernet protocol.	
*/
_declspec(dllexport) int fn_NetSim_Ethernet_Configure(void** var)
{
	fn_NetSim_Ethernet_Configure_F(var);
	return 1;
}

_declspec(dllexport) char* fn_NetSim_Ethernet_ConfigPacketTrace(void* var)
{
	return "";
}
_declspec(dllexport) char* fn_NetSim_Ethernet_WritePacketTrace(NetSim_PACKET* pstruPacket)
{
	return "";
}

