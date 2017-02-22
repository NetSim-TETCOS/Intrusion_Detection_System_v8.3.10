/************************************************************************************
 * Copyright (C) 2014
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Surabhi Bothra															*
 * ---------------------------------------------------------------------------------*/

#include "main.h"
#include "WiMax.h"

int fn_NetSim_WiMax_CalculateReceivedPower(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId,double Tx_power,double Distance,double Frequency,double PathLossExponent,double *ReceivedPower)
{
	double fpi=3.1417f;		
	double Gain_Tx=0;		
	double Gain_Rcvr=0;	
	int DefaultDistance=1; 
	double A1,WaveLength=0.0; 
	double A1dB, A2dB;
	double Tx_powerdBm; 

	WIMAX_BS_PHY *pstruPhyVar;
	pstruPhyVar = (WIMAX_BS_PHY*)DEVICE_PHYVAR(nDeviceId,nInterfaceId);
	if(pstruPhyVar->ChannelModel == NO_PATHLOSS || Distance == 0)
	{
		*ReceivedPower = 10 * log10( Tx_power )+ 30;
		return 0;
	}
	else
	{
		// get the gain of the Transmitter
		Gain_Tx=0;
		// get the gain of the receiver
		Gain_Rcvr=0;
		// Calculate Lambda
		WaveLength=(double)(3.0/(Frequency * 10.0));//pathloss
		// Calculate  (4*3.14*do)
		A1=WaveLength/(4*(double) fpi * DefaultDistance );
		//Calculate  20log10[ Lambda/(4*3.14*do) ]
		A1dB =  20 * log10(A1);
		//Calculate  10 * n *log10 (do/d)
		A2dB =  10 * PathLossExponent * log10(DefaultDistance /Distance);
		Tx_powerdBm = 10 * log10(Tx_power) + 30;//Power in Watts
		//Calculate  [Pt]  +  [Gt]  +  [Gr]  +  20log10[ Lemda/(4*3.14*do) ] + ( 10 * n *log10 (do/d) )
		*ReceivedPower = Tx_powerdBm + Gain_Tx + Gain_Rcvr + A1dB + A2dB;
	}
	return 0;
}

int fn_NetSim_WiMax_CalculateShadowLoss(unsigned long* ulSeed1, unsigned long *ulSeed2, double* ShadowReceivedPower,double StandardDeviation)
{
	double GaussianDeviate=0.0;
	double RandomNumber = 0.0;
	static int Iset = 0;
	static double Gset=0;
	double fFac,fRsq,fV1,fV2;	
	double d_sign1;
	if(Iset==0)
	{
		do
		{		
			// call the random number generator function
			RandomNumber = fn_NetSim_Utilities_GenerateRandomNo(ulSeed1,ulSeed2)/NETSIM_RAND_MAX;		
			fV1=(double)(2.0*RandomNumber-1.0);
			RandomNumber = fn_NetSim_Utilities_GenerateRandomNo(ulSeed1,ulSeed2)/NETSIM_RAND_MAX;		
			fV2=(double)(2.0*RandomNumber-1.0);
			fRsq=fV1*fV1+fV2*fV2;
		}while(fRsq>=1.0 || fRsq==0.0);
		fFac=(double)(sqrt(-2.0*log(fRsq)/fRsq));
		Gset=fV1*fFac;	
		Iset=1;
		GaussianDeviate = fV2*fFac;
	}
	else
	{
		Iset=0;
		GaussianDeviate = Gset;
	}
	d_sign1 = fn_NetSim_Utilities_GenerateRandomNo(ulSeed1,ulSeed2)/NETSIM_RAND_MAX;
	if(d_sign1 <= 0.5) 
		{
			// Assign the Received power due to shadowloss.	
			*ShadowReceivedPower = -GaussianDeviate * sqrt(StandardDeviation); 
			// This is done to ensure there is constructive and destructive shadowing
		}
	else
		{	
			// Assign the Received power due to shadowloss.	
			*ShadowReceivedPower = GaussianDeviate * sqrt(StandardDeviation);
		}

	return 0;
}

int fn_NetSim_WiMax_CalculateAndSetReceivedPower(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId)
{
	double Distance;
	double ReceivedPower;
	double ShadowReceivedPower;
	double TotalRxPower;
	unsigned long *ulSeed1, *ulSeed2;
	NETSIM_ID DeviceCount;	
	
	WIMAX_BS_PHY* BSPhy;
	WIMAX_SS_PHY* SSPhy;

	WIMAX_BS_MAC* BSMac;

	BSPhy = (WIMAX_BS_PHY* )DEVICE_PHYVAR(BS_Id,IntId);
	SSPhy = (WIMAX_SS_PHY* )DEVICE_PHYVAR(SS_Id,InterfaceId);

	BSMac = (WIMAX_BS_MAC*)DEVICE_MACVAR(BS_Id,IntId);
	DeviceCount = NETWORK->nDeviceCount;

	BSPhy->PathLossExponent = DEVICE_PHYLAYER(BS_Id,IntId)->pstruNetSimLinks->puniMedProp.pstruWirelessLink.dPathLossExponent;
	BSPhy->FadingFigure = DEVICE_PHYLAYER(BS_Id,IntId)->pstruNetSimLinks->puniMedProp.pstruWirelessLink.dFadingFigure;
	BSPhy->StandardDeviation = DEVICE_PHYLAYER(BS_Id,IntId)->pstruNetSimLinks->puniMedProp.pstruWirelessLink.dStandardDeviation;

	if(BSPhy->PathLossExponent ==0 && BSPhy->FadingFigure==0 && BSPhy->StandardDeviation==0)
		BSPhy->ChannelModel = NO_PATHLOSS;	
	else if(BSPhy->PathLossExponent!= 0 && BSPhy->FadingFigure==0)
		BSPhy->ChannelModel = LINE_OF_SIGHT;	
	else if(BSPhy->PathLossExponent!= 0 && BSPhy->FadingFigure!=0 && BSPhy->StandardDeviation==0)
		BSPhy->ChannelModel = FADING;	
	else
		BSPhy->ChannelModel = FADING_AND_SHADOWING;

	Distance = fn_NetSim_Utilities_CalculateDistance(DEVICE_POSITION(BS_Id),DEVICE_POSITION(SS_Id));
	BSPhy->Distance = Distance;		
	BSMac->struBS_Metrics->SS_Id = SS_Id;
	BSMac->struBS_Metrics->distance[SS_Id] = BSPhy->Distance;
						
	fn_NetSim_WiMax_CalculateReceivedPower(BS_Id,IntId,BSPhy->Tx_power,Distance,BSPhy->Frequency,BSPhy->PathLossExponent,&ReceivedPower);
	ShadowReceivedPower=0.0;										
	if(BSPhy->StandardDeviation != 0)
	{
		ulSeed1 = &(NETWORK->ppstruDeviceList[BS_Id-1]->ulSeed[0]);
		ulSeed2 = &(NETWORK->ppstruDeviceList[BS_Id-1]->ulSeed[1]);	
		fn_NetSim_WiMax_CalculateShadowLoss(ulSeed1,ulSeed2,&ShadowReceivedPower,BSPhy->StandardDeviation);
	}								
	TotalRxPower = ReceivedPower + ShadowReceivedPower;
	CummulativeReceivedPower[BS_Id][SS_Id] = TotalRxPower; 
	SSPhy->TotalReceivedPower = TotalRxPower;
		
	return 0;
}


int fn_NetSim_WiMax_Calculate_SNR(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId)
{
	double NoisePowerdB;
	double SNR;
	NETSIM_ID DeviceCount;	

	WIMAX_BS_PHY* BSPhy;
	WIMAX_SS_PHY* SSPhy;
	DeviceCount = NETWORK->nDeviceCount;


	BSPhy = (WIMAX_BS_PHY*)DEVICE_PHYVAR(BS_Id,IntId);
	SSPhy = (WIMAX_SS_PHY*)DEVICE_PHYVAR(SS_Id,InterfaceId);

	NoisePowerdB =  -174 + 90 + 10* log10(BSPhy->Frequency);
	SNR = SSPhy->TotalReceivedPower - NoisePowerdB;
	SSPhy->SNR = SNR;
	
	return 0;
}

int fn_NetSim_WiMax_Mod(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId)
{
	int i;
	WIMAX_BS_PHY* bsPhy;
	WIMAX_SS_PHY* ssPhy;
	WIMAX_BS_MAC* BSMac;
	bsPhy = (WIMAX_BS_PHY* )DEVICE_PHYVAR(BS_Id,IntId);
	ssPhy = (WIMAX_SS_PHY* )DEVICE_PHYVAR(SS_Id,InterfaceId);

	BSMac = (WIMAX_BS_MAC*) DEVICE_MACVAR(BS_Id,IntId);

	for(i=1;i<=MAX_PHY_PARAMETER;i++)
	{
		if(ssPhy->SNR >= struPhyParameters[MAX_PHY_PARAMETER-i].SNR)
		{
			bsPhy->Modulation = struPhyParameters[MAX_PHY_PARAMETER-i].Modulation;
			bsPhy->CodingRate = struPhyParameters[MAX_PHY_PARAMETER-i].CodingRate;
			bsPhy->BitsCountInOneSymbol = struPhyParameters[MAX_PHY_PARAMETER-i].BitsCountInOneSymbol;
			bsPhy->Datarate = bsPhy->BitsCountInOneSymbol/bsPhy->pstruSymbolParameter->OFDMSymbolTime;
			//Metrics
			BSMac->struBS_Metrics->Datarate[SS_Id] = bsPhy->Datarate;
			break;
		}
	}

	return 0;
}
