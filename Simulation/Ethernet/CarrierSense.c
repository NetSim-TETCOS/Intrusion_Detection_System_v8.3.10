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
 * ---------------------------------------------------------------------------------*/

/********************************************************************************/
/* This function is called when the MAC_OUT_EVENT is triggered with subevent type*/
/* CARRIER_SENSE.This function senses Physical layer link status.				*/
/* It adds the IFG event to the event list.										*/		
/********************************************************************************/

#include "main.h"
#include "Main_FE.h"
/**
 This function is called when the MAC_OUT_EVENT is triggered with subevent type
 CARRIER_SENSE.This function senses Physical layer link status.			
 It adds the IFG event to the event list.										
*/
int fn_NetSim_FastEthernet_Carrier_Sense(NetSim_EVENTDETAILS *pstruEventDetails)
{
	NetSim_PACKET *pstruTemp_Packet;
	DEVICE_PHYSICALLAYER *pstruTemp_Phy;

	/*Type casting the Physical layer variable*/
	pstruTemp_Phy=((DEVICE_PHYSICALLAYER *)(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->ppstruInterfaceList[pstruEventDetails->nInterfaceId-1]->pstruPhysicalLayer->phyVar));

	
	switch(pstruEventDetails->nDeviceType)
	{
	/*check the device type as Switch*/
	
	case SWITCH:
	{

		/*check the frame is control frame i.e CPDU*/
		if(pstruEventDetails->pPacket !=NULL)
		{
			if(pstruEventDetails->pPacket->nControlDataType==CBPDU_PACKET)
			{
				/*Set Arrival Time*/
				pstruTemp_Packet=pstruEventDetails->pPacket;
				pstruTemp_Packet->pstruMacData->dArrivalTime=pstruEventDetails->dEventTime;
				/*Set Start Time*/
				pstruTemp_Packet->pstruMacData->dStartTime=pstruEventDetails->dEventTime;
				/*For Control frames there is no medium sense,so directly move the frame to physical layer*/
				pstruEventDetails->nApplicationId=ZERO;
				pstruEventDetails->nEventType=PHYSICAL_OUT_EVENT;
				pstruEventDetails->nSubEventType=ZERO;
				pstruEventDetails->pPacket=pstruTemp_Packet;
				fnpAddEvent(pstruEventDetails);
			}

		}
		/*Data frames*/
		else 
		{
			/*Check the arrival time is less than or equal to event time and the ports uplink is free*/
			if(pstruTemp_Phy->nLinkState == LINK_DOWN)
			{			
				pstruEventDetails->nSubEventType=WAIT_FOR_IFG;	/*Add the IFG event*/

				if(strcmp(pstruTemp_Phy->pszEthernet_Standard,"IEEE802.3U")==0 || strcmp(pstruTemp_Phy->pszEthernet_Standard,"IEEE802.3Z")==0 )
				{
					pstruEventDetails->dEventTime += 0.96f; //960 nano secs
					fnpAddEvent(pstruEventDetails);
				}
				else
					fnNetSimError("Unknown ethernet standard '%s' for device %d inteface %d\n",pstruTemp_Phy->pszEthernet_Standard,pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
			}
			else
			{
				pstruEventDetails->dEventTime=pstruTemp_Phy->dData_End_Time;
				fnpAddEvent(pstruEventDetails);
			
			}

		}

		break;
	}
	/* Check the device type is not Switch*/
	default:
	{		

		if(pstruTemp_Phy->nLinkState == LINK_DOWN)
		{
			
			pstruEventDetails->nSubEventType=WAIT_FOR_IFG;
			/*Check the physical layer standard is Fast ethernet*/
			if(strcmp(pstruTemp_Phy->pszEthernet_Standard,"IEEE802.3U")==0 || strcmp(pstruTemp_Phy->pszEthernet_Standard,"IEEE802.3Z")==0 )
			{
				pstruEventDetails->dEventTime += 0.96f;	/*as per the 802.3u Std inter frame gap is .96 micro sec */
				fnpAddEvent(pstruEventDetails);
			}
			else
				fnNetSimError("Unknown ethernet standard '%s' for device %d inteface %d\n",pstruTemp_Phy->pszEthernet_Standard,pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId);
		}
		else
		{
			//pstruEventDetails->dEventTime=pstruTemp_Phy->dData_End_Time;
			//fnpAddEvent(pstruEventDetails);
		}

		break;
	}

	}

	return 1;
}


