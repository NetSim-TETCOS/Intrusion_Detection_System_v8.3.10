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
#include "ZRP.h"
#include "ZRP_Enum.h"

int fn_NetSim_OLSR_UpdateRoutingTable()
{
	//Section 10
	NODE_OLSR* olsr=GetOLSRData(pstruEventDetails->nDeviceId);
	NETSIM_IPAddress subnet = STR_TO_IP4("255.255.255.255");
	OLSR_NEIGHBOR_SET* neighbor;
	OLSR_2HOP_NEIGHBOR_SET* two_hop_neighbor;
	OLSR_TOPOLOGY_INFORMATION_BASE* topology;
	if(!olsr->bRoutingTableUpdate)
	{
		return 0; //No update
	}
	//Condition 1
	olsrFlushroutingTable(olsr->ipTable,pstruEventDetails->nDeviceId);
	olsr->ipTable=NULL;

	//Condition 2
	neighbor=olsr->neighborSet;
	while(neighbor)
	{
		if(neighbor->N_status>=SYM_NEIGH)
		{
			iptable_add(&olsr->ipTable,
				neighbor->N_neighbor_main_addr,
				subnet,
				0,
				neighbor->N_neighbor_main_addr,
				olsr->mainAddress,
				1,
				1);

		}
		neighbor=(OLSR_NEIGHBOR_SET*)LIST_NEXT(neighbor);
	}

	//Condition 3
	two_hop_neighbor=olsr->twoHopNeighborSet;
	while(two_hop_neighbor)
	{
		neighbor=olsrFindNeighborSet(olsr->neighborSet,two_hop_neighbor->N_2hop_addr);
		if(!neighbor && IP_COMPARE(two_hop_neighbor->N_2hop_addr,olsr->mainAddress))
		{
			neighbor=olsrFindNeighborSet(olsr->neighborSet,two_hop_neighbor->N_neighbor_main_addr);
			if(neighbor->N_willingness != WILL_NEVER && neighbor->N_status==MPR_NEIGH)
			{
				iptable_add(&olsr->ipTable,
					two_hop_neighbor->N_2hop_addr,
					subnet,
					0,
					two_hop_neighbor->N_neighbor_main_addr,
					olsr->mainAddress,
					1,
					2);
			}
		}
		two_hop_neighbor=(OLSR_2HOP_NEIGHBOR_SET*)LIST_NEXT(two_hop_neighbor);
	}

	//Condition 3
	topology=olsr->topologyInfoBase;
	while(topology)
	{
		IP_ROUTINGTABLE* table=iptable_check(&olsr->ipTable,topology->T_dest_addr,subnet);
		if(!table && IP_COMPARE(topology->T_dest_addr,olsr->mainAddress))
		{
			table=iptable_check(&olsr->ipTable,topology->T_last_addr,subnet);
			if(table && table->Metric < olsr->nZoneRadius)
			{
				iptable_add(&olsr->ipTable,
					topology->T_dest_addr,
					subnet,
					0,
					table->gateway,
					olsr->mainAddress,
					1,
					table->Metric+1);
				topology=olsr->topologyInfoBase;
				continue;
			}
		}
		topology=(OLSR_TOPOLOGY_INFORMATION_BASE*)LIST_NEXT(topology);
	}
	olsrUpdateIptable(olsr->ipTable,pstruEventDetails->nDeviceId);
	olsr->bRoutingTableUpdate=false;
	if(DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId == NW_PROTOCOL_ZRP)
		fn_NetSim_BRP_Update(olsr->ipTable);
	return 0;
}
int olsrUpdateIptable(IP_ROUTINGTABLE* table,NETSIM_ID nNodeId)
{
	IP_ROUTINGTABLE** iptable=(IP_ROUTINGTABLE**)&DEVICE_NWLAYER(nNodeId)->ipRoutingTables;
	while(table)
	{
		iptable_add(iptable,
			table->networkDestination,
			table->netMask,
			table->prefix_len,
			table->gateway,
			table->Interface,
			table->nInterfaceId,
			table->Metric);
		
		table=(IP_ROUTINGTABLE*)LIST_NEXT(table);
	}
	fprintf(stdout,"\nRouting table of node %d at time %lf:",nNodeId,pstruEventDetails->dEventTime);
	iptable_print(stdout,*iptable);
	return 0;
}
int olsrFlushroutingTable(IP_ROUTINGTABLE* iptable,NETSIM_ID nNodeId)
{
	IP_ROUTINGTABLE** table=(IP_ROUTINGTABLE**)&DEVICE_NWLAYER(nNodeId)->ipRoutingTables;
	while(iptable)
	{
		iptable_delete(table,iptable->networkDestination);
		iptable_delete(&iptable,iptable->networkDestination);
	}
	return 0;
}