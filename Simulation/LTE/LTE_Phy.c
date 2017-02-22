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
#include "LTE.h"
NETSIM_ID fn_NetSim_LTE_FindNearesteNB(NETSIM_ID nDeviceId);
int fn_NetSim_LTE_CalculateReceivedPower()
{
	NETSIM_ID i;
	for(i=0;i<NETWORK->nDeviceCount;i++)
	{
		if(NETWORK->ppstruDeviceList[i]->nDeviceType==eNB)
		{
			LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(i+1,1);
			LTE_ASSOCIATEUE_INFO* info=enbMac->associatedUEInfo;
			while(info)
			{
				fn_NetSim_LTE_CalculateRxPower(i+1,1,info);
				fn_NetSim_LTE_CalculateSNR(i+1,1,info);
				fn_NetSim_LTE_GetCQIIndex(i+1,1,info);
				fn_NetSim_LTE_GetMCS_TBS_Index(info);
				info=(LTE_ASSOCIATEUE_INFO*)LIST_NEXT(info);
			}
		}
	}
	return 1;
}
int fn_NetSim_LTE_CalculateRxPower(NETSIM_ID enbId,NETSIM_ID enbInterface,LTE_ASSOCIATEUE_INFO* info)
{
	LTE_ENB_PHY* enbPhy=(LTE_ENB_PHY*)DEVICE_PHYVAR(enbId,enbInterface);
	double dTXPower_DL=enbPhy->dTXPower;
	NETSIM_ID nLinkID=DEVICE_PHYLAYER(enbId,enbInterface)->nLinkId;

	LTE_UE_PHY* uePhy=(LTE_UE_PHY*)DEVICE_PHYVAR(info->nUEId,info->nUEInterface);
	double dTXPower_UL=uePhy->dTXPower;

	double fpi=3.1417;	// TO GET THE PI VALUE
	double dGainTxr=0;	// TO GET THE TRANSMITTER GAIN
	double dGainRver=0;	// TO GET THE RECEIVER GAIN
	int nDefaultDistance=1;	// TO GET THE DEFULT DISTANCE
	double fA1,fWaveLength=0.0; // TO GET THE WAVELENGTH VALUE
	double fA1dB, fA2dB;
	double dDefaultExponent=2;
	double dRxPower_UL,dRxPower_DL;
	double dDistance=fn_NetSim_Utilities_CalculateDistance(DEVICE_POSITION(enbId),DEVICE_POSITION(info->nUEId));

	//TO CALCULATE LEMDA
	fWaveLength=(double)(300.0/(NETWORK->ppstruNetSimLinks[nLinkID-1]->puniMedProp.pstruWirelessLink.dFrequency *1.0));//pathloss

	// TO CALCULATE (4*3.14*do)
	fA1=fWaveLength/(4*(double) fpi * nDefaultDistance);

	// TO CALCULATE 20log10[ lemda/(4*3.14*do) ]
	fA1dB =  10 * dDefaultExponent * log10(1.0/fA1);

	// TO CALCULATE 10 * n *log10 (d/do)
	fA2dB =  10 * NETWORK->ppstruNetSimLinks[nLinkID-1]->puniMedProp.pstruWirelessLink.dPathLossExponent * log10(dDistance/nDefaultDistance);

	//TO CALCULATE [Pt]  +  [Gt]  +  [Gr]  +  20log10[ Lemda/(4*3.14*do) ] + ( 10 * n *log10 (do/d) )
	dRxPower_DL = dTXPower_DL + dGainTxr + dGainRver - fA1dB - fA2dB;
	dRxPower_UL = dTXPower_UL + dGainTxr + dGainRver - fA1dB - fA2dB;
	info->DLInfo.dReceivedPower=dRxPower_DL+30;//in dbm
	info->ULInfo.dReceivedPower=dRxPower_UL+30;//in dbm

	return 0;
}
int fn_NetSim_LTE_CalculateSNR(NETSIM_ID enbId,NETSIM_ID enbInterface,LTE_ASSOCIATEUE_INFO* info)
{
	LTE_ENB_PHY* enbPhy=(LTE_ENB_PHY*)DEVICE_PHYVAR(enbId,enbInterface);
	double dThermalNoisePower_1Hz = -174.0; //At room temp. In dBm
	double dThermalNoise; //in dBm

	//Calculate the thermal noise
	dThermalNoise = dThermalNoisePower_1Hz + 10 * log10((enbPhy->dChannelBandwidth)*1000000);

	//Calculate the SNR
	info->DLInfo.dSNR = info ->DLInfo.dReceivedPower - dThermalNoise;
	info->ULInfo.dSNR = info ->ULInfo.dReceivedPower - dThermalNoise;
	return 1;
}


 int fn_NetSim_LTE_GetTransmissionIndex(LTE_ENB_PHY* enbPhy,LTE_UE_PHY* uePhy,LTE_ASSOCIATEUE_INFO* info)
{
	//uplink
		if (enbPhy->nTransmissionMode == Single_Antenna && uePhy->nTXAntennaCount == 1 && enbPhy->nRXAntennaCount ==1)
		{
			info->TransmissionIndex.UL = 0;
		}
		else if (enbPhy->nTransmissionMode == Transmit_Diversity && uePhy->nTXAntennaCount == 1 && enbPhy->nRXAntennaCount ==2)
		{
			info->TransmissionIndex.UL = 1;
		}
		else if (enbPhy->nTransmissionMode == SingleUser_MIMO_Spacial_Multiplexing && uePhy->nTXAntennaCount == 1 && enbPhy->nRXAntennaCount ==2)
		{
			info->TransmissionIndex.UL = 2;
		}
		else if(enbPhy->nTransmissionMode == SingleUser_MIMO_Spacial_Multiplexing && uePhy->nTXAntennaCount == 1 && enbPhy->nRXAntennaCount == 2 && enbPhy->nTXAntennaCount == 4)
		{
				info->TransmissionIndex.UL = 3;
		}
		else
		{
			fnNetSimError("Unknown transmission mode, Tx antenna count, Rx antenna count\nValid combinations are...\nFor eNB.....\n"
				"TRANSMISSION MODE INDEX \t TRANSMISSION MODE \t TX_ANTENNA \t RX_ANTENNA\n"
				"0 = 1 1 1\n"
				"1 = 2 2 2\n" 
				"2 = 3 2 2\n"
				"3 = 3 4 2\n"
				"For UE....\n"
				"0 = 1 1 1\n"
				"1 = 2 1 2\n" 
				"2 = 3 1 2\n"
				"3 = 3 1 2\n"
				"1) Transmission Mode 1- Using of a single antenna at eNodeB\n"
				"2) Transmission Mode 2- Transmit Diversity (TxD)\n"
				"3) Transmission Mode 3- SU-MIMO Spatial Multiplexing: Open-Loop\n"
				"Running without MIMO..\n");
			info->TransmissionIndex.UL = 1;
		}

	//downlink
		if (enbPhy->nTransmissionMode == Single_Antenna && enbPhy->nTXAntennaCount == 1 && uePhy->nRXAntennaCount ==1)
		{
			info->TransmissionIndex.DL = 0;
		}
		else if (enbPhy->nTransmissionMode == Transmit_Diversity && enbPhy->nTXAntennaCount == 2 && uePhy->nRXAntennaCount ==2)
		{
			info->TransmissionIndex.DL = 1;
		}
		else if (enbPhy->nTransmissionMode == SingleUser_MIMO_Spacial_Multiplexing && enbPhy->nTXAntennaCount == 2 && uePhy->nRXAntennaCount ==2)
		{
			info->TransmissionIndex.DL = 2;
		}
		else if(enbPhy->nTransmissionMode == SingleUser_MIMO_Spacial_Multiplexing && enbPhy->nTXAntennaCount == 4 && uePhy->nRXAntennaCount ==2)
		{
			info->TransmissionIndex.DL = 3;
		}
		else
		{
			fnNetSimError("Unknown transmission mode, Tx antenna count, Rx antenna count\nValid combinations are...\nFor eNB.....\n"
				"TRANSMISSION MODE INDEX \t TRANSMISSION MODE \t TX_ANTENNA \t RX_ANTENNA\n"
				"0 = 1 1 1\n"
				"1 = 2 2 2\n" 
				"2 = 3 2 2\n"
				"3 = 3 4 2\n"
				"For UE....\n"
				"0 = 1 1 1\n"
				"1 = 2 1 2\n" 
				"2 = 3 1 2\n"
				"3 = 3 1 2\n"
				"1) Transmission Mode 1- Using of a single antenna at eNodeB\n"
				"2) Transmission Mode 2- Transmit Diversity (TxD)\n"
				"3) Transmission Mode 3- SU-MIMO Spatial Multiplexing: Open-Loop\n"
				"Running without MIMO..\n");
			info->TransmissionIndex.DL = 1;
		}
		
		return 0;
}

int fn_NetSim_LTE_GetCQIIndex(NETSIM_ID enbId,NETSIM_ID enbInterface,LTE_ASSOCIATEUE_INFO* info)
{
	unsigned int i;
	LTE_ENB_PHY* enbPhy=(LTE_ENB_PHY*)DEVICE_PHYVAR(enbId,enbInterface);
	LTE_UE_PHY* uePhy=(LTE_UE_PHY*)DEVICE_PHYVAR(info->nUEId,info->nUEInterface);
	double dSNR_UL=info->ULInfo.dSNR;
	double dSNR_DL=info->DLInfo.dSNR;
	fn_NetSim_LTE_GetTransmissionIndex(enbPhy,uePhy,info);
	//uplink
	info ->ULInfo.Rank= 1;
	for(i=0;i<CQI_SNR_TABLE_LEN;i++)  
	{
		if(dSNR_UL<=CQI_SNR_TABLE[info->TransmissionIndex.UL][i])
		{
			info->ULInfo.nCQIIndex=i;
			break;
		}
	}
	if(i==CQI_SNR_TABLE_LEN)
		info->ULInfo.nCQIIndex=CQI_SNR_TABLE_LEN;
	if(i==0)
		info->ULInfo.nCQIIndex=1;
	
	if (info->ULInfo.nCQIIndex>= 10)
	{
		info ->ULInfo.Rank= min(MODE_INDEX_MAPPING[info->TransmissionIndex.UL].Tx_Antenna_UL,MODE_INDEX_MAPPING[info->TransmissionIndex.UL].Rx_Antenna_UL);
	}
	//downlink
	info ->DLInfo.Rank= 1;
	for(i=0;i<CQI_SNR_TABLE_LEN;i++)  
	{
		if(dSNR_DL<=CQI_SNR_TABLE[info->TransmissionIndex.DL][i])
		{
			info->DLInfo.nCQIIndex=i;
			break;
		}
	}
	if(i==CQI_SNR_TABLE_LEN)
		info->DLInfo.nCQIIndex=CQI_SNR_TABLE_LEN;
	if(i==0)
		info->DLInfo.nCQIIndex=1;

	if (info->DLInfo.nCQIIndex>= 10)
	{
		info ->DLInfo.Rank= min(MODE_INDEX_MAPPING[info->TransmissionIndex.DL].Tx_Antenna_DL,MODE_INDEX_MAPPING[info->TransmissionIndex.DL].Rx_Antenna_DL);
	}
	return 1;
}
int fn_NetSim_LTE_GetMCS_TBS_Index(LTE_ASSOCIATEUE_INFO* info)
{	
	//uplink
	info->ULInfo.MCSIndex=CQI_MCS_MAPPING[info->ULInfo.nCQIIndex-1].MCS_Index;
	info->ULInfo.modulation=CQI_MCS_MAPPING[info->ULInfo.nCQIIndex-1].modulation;
	info->ULInfo.TBSIndex=MCS_TBS_MAPPING[info->ULInfo.MCSIndex].TBS_Index;
	
	//downlink
	info->DLInfo.MCSIndex=CQI_MCS_MAPPING[info->DLInfo.nCQIIndex-1].MCS_Index;
	info->DLInfo.modulation=CQI_MCS_MAPPING[info->DLInfo.nCQIIndex-1].modulation;
	info->DLInfo.TBSIndex=MCS_TBS_MAPPING[info->DLInfo.MCSIndex].TBS_Index;
	
	return 1;
}

int fn_NetSim_LTE_Mobility(NETSIM_ID nDeviceId)
{
	if(DEVICE_MACLAYER(nDeviceId,1)->nMacProtocolId == MAC_PROTOCOL_LTE)
	{
		LTE_UE_MAC* ueMac=(LTE_UE_MAC*)DEVICE_MACVAR(nDeviceId,1);
		NETSIM_ID enbId=ueMac->nENBId;
		NETSIM_ID enbInterface=ueMac->nENBInterface;
		LTE_ENB_MAC* enbMac=(LTE_ENB_MAC*)DEVICE_MACVAR(enbId,enbInterface);
		LTE_ASSOCIATEUE_INFO* info=fn_NetSim_LTE_FindInfo(enbMac,nDeviceId);
	
		fn_NetSim_LTE_InitHandover(nDeviceId,enbId);
	
		fn_NetSim_LTE_CalculateRxPower(enbId,enbInterface,info);
		fn_NetSim_LTE_CalculateSNR(enbId,enbInterface,info);
		fn_NetSim_LTE_GetCQIIndex(enbId,enbInterface,info);
		fn_NetSim_LTE_GetMCS_TBS_Index(info);
	}
	else
	{
		//Do nothing other mac protocol
	}
	return 1;
}
NETSIM_ID fn_NetSim_LTE_FindNearesteNB(NETSIM_ID nDeviceId)
{
	NETSIM_ID i;
	double distance=0xFFFFFFFF;
	NETSIM_ID enb=0;
	for(i=0;i<NETWORK->nDeviceCount;i++)
	{
		if(DEVICE_TYPE(i+1)==eNB)
		{
			double temp=fn_NetSim_Utilities_CalculateDistance(DEVICE_POSITION(nDeviceId),DEVICE_POSITION(i+1));
			if(temp<distance)
			{
				distance=temp;
				enb=i+1;
			}
		}
	}
	return enb;
}
