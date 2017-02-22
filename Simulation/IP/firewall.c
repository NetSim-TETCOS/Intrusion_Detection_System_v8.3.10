/************************************************************************************
 * Copyright (C) 2014                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Shashi Kant Suman                                                     *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#include "main.h"
#include "List.h"
#include "IP.h"
#include "NetSim_utility.h"

#define PACKET_ALLOWED 1
#define PACKET_BLOCKED 0
/**
	This function is to configure the firewall.
*/
_declspec(dllexport) int fn_NetSim_IP_FirewallConfig(NETSIM_ID nDeviceId)
{   
	int flag=0;
	NETSIM_ID id=0;
	char input[BUFSIZ];
	IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipVar;
	FILE* fp=fopen(devVar->firewallConfig,"r");
	if(!fp)
	{
		perror(devVar->firewallConfig);
		fnSystemError("Unable to open firewall config file.\nRunning without firewall.");
		devVar->nFirewallStatus=0;
		return 0;
	}
	while(fgets(input,BUFSIZ,fp))
	{
		char* temp;
		char* str=input;
		str=lskip(str);
		if(*str == '#')
			continue;
		if(*str)
		{
			temp=strtok(str,"=");
			str=temp+strlen(temp)+1;
			temp=lskip(temp);
			rstrip(temp);
			_strupr(temp);
			if(!strcmp(temp,"DEVICE_ID"))
			{
				flag=0;
				temp=str;
				temp=lskip(temp);
				rstrip(temp);
				str=temp;
				temp=strtok(str,",");
				while(temp)
				{
					if((NETSIM_ID)atoi(temp)==NETWORK->ppstruDeviceList[nDeviceId-1]->nConfigDeviceId)
					{
						flag=1;
						break;
					}
					temp=strtok(NULL,",");
				}
			}
			else if(*temp && flag)
			{
				rstrip(temp);
				IP_ADD_TO_LIST(&(devVar->blockedIP),&(devVar->blockedIPCount),STR_TO_IP4(temp));
			}
		}
	}
	fclose(fp);
	return 1;
}
/**
	This function is to check whether the particular packet is blocked or allowed by firewall
*/
_declspec(dllexport) int fn_NetSim_IP_Firewall(NETSIM_ID nDeviceId,NetSim_PACKET* packet)
{
	IP_DEVVAR* devVar=NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->ipVar;
	if(devVar && devVar->nFirewallStatus)
	{
		//check for source
		if(IP_CHECK_IN_LIST(devVar->blockedIP,devVar->blockedIPCount,packet->pstruNetworkData->szSourceIP))
			return PACKET_BLOCKED;
		//check for destination
		if(IP_CHECK_IN_LIST(devVar->blockedIP,devVar->blockedIPCount,packet->pstruNetworkData->szDestIP))
			return PACKET_BLOCKED;
		//check for nexthop
		if(IP_CHECK_IN_LIST(devVar->blockedIP,devVar->blockedIPCount,packet->pstruNetworkData->szNextHopIp))
			return PACKET_BLOCKED;
	}
	return PACKET_ALLOWED;
}