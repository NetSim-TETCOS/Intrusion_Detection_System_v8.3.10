/************************************************************************************
File Based Mobility                                                  *

The name of the trace File generated should be kept as mobility.txt and it should 
be in the NS2 trace File format. The user can genmerate the trace file by any traffic
simulator. One of the Traffic Generator is VanetMobSim.
A user wishing to simulate Vanet Scenario can place the nodes randomly at any position
and start the simulation. The device Initial and Intermediate device Postions will
be taken care by the following three functions

The function FileBasedMobilityInitializeNodePosition(int ); sets the initial positions 
of the nodes. 

The function FileBasedMobility(int , double ); is defined to change the position of 
the current deiveceID and to read the next Position Change of the current deviceID 
and add this Event to the List of events to be performed.

The function FileBasedMobilityPointersFree(int ); closes the file and frees all the
pointers.

#
#nodes: 5  max x = 1000.0, max y: 1000.0
#
$node_(0) set X_ 0.6642883828044
$node_(0) set Y_ 0.2309939067026
$node_(0) set Z_ 0.0
$node_(1) set X_ 50.6642883828044
$node_(1) set Y_ 50.2309939067026
$node_(1) set Z_ 0.0
$node_(2) set X_ 100.1527892303775
$node_(2) set Y_ 100.0017151661647
$node_(2) set Z_ 0.0
$node_(3) set X_ 150.3207048718017
$node_(3) set Y_ 150.7817679768309
$node_(3) set Z_ 0.0
$node_(4) set X_ 200.6792971281983
$node_(4) set Y_ 200.2182340231691
$node_(4) set Z_ 0.0
$time 0.0 "$node_(0) 0.00 0.00 0.00"
$time 0.0 "$node_(1) 50.0 50.0 0.0"
$time 0.0 "$node_(2) 100 100 0"
$time 0.0 "$node_(3) 150 150 0"
$time 0.0 "$node_(4) 200 200 0"
$time 0.05 "$node_(0) 50 0 0"
$time 0.05 "$node_(1) 100 50 0"
...
...
...
********************************************************************************
*******************************************************************************/
#define ANIMATION 0		//Define animation = 0 if user doesnt wish to see animation.
						//i.e due to GUI constraints, the node might go out of the GUI
						//distance limit which would not make the animation vissible.
						//
						//else animation = 1 which will scale the device positions to fit
						//the GUI screen. 
#include "main.h"
#include "Mobility.h"
#include <string.h>
char sent[100]; // 100 is likely to be the max characters per line 
FILE *fp;
fpos_t *pos, startPos;	//to store the file pointer fp locations
						//to define and array pos[] of length nDeviceCount
int maxDeviceCount_fromFile;
double maxX, maxY;			//The max dimesion of simulation environment from file
int scalingFactor;		//scalingFactor = maxX/500 since NetSim's simulation environment is 500 X 500
						//This is if user wishes to see animation in netsim




int FileBasedMobility(int, double );

/** This function is to free the file pointers */
int FileBasedMobilityPointersFree()
{
	if(fp)
		fclose(fp);
	if(pos)
		free(pos);							//free all the position pointers 
	return 0;
}
/** This function is to open the file, to define position pointers and to set the initial positions for all the nodes */
int FileBasedMobilityInitializeNodePositions(int nDeviceCount, int nDeviceID_1)
{
	/*
	maxDeviceCount_fromFile is the maximum number of device from the file mobility.txt
	nDeviceCount is the total number of devices that a user has created in the NetSim 
	Scenario. 
	X,Y,Z are the Positon Coordinates of nodes
	*/

	static int fileOpen = 0 ;
	int i=0;
	double X, Y, Z ;	//Position Co-ordinates
	int nDeviceID_fromFile;	
	
	
	if(fileOpen == 0)
	{
		pos = calloc(nDeviceCount,sizeof *pos);
		fp = fopen(".\\mobility.txt", "r");
		fileOpen++;
	}
	if(fp)
	{
		//scanning the first three sentences of trace file
		if(fileOpen == 1)
		{
			fscanf(fp,"%[^\n]s",sent);
			fseek(fp,2,1);
			fscanf(fp,"#nodes: %d  max x = %lf, max y: %lf ",&maxDeviceCount_fromFile,&maxX,&maxY);
			fscanf(fp,"%[^\n]s",sent);
			fseek(fp,2,1);
			fgetpos(fp, &startPos);		//store this location of file pointer in startPos
			fileOpen++;
			if(maxX>maxY)
				scalingFactor = (int)maxX/500;	//calculating scaling factor
			else
				scalingFactor = (int)maxY/500;	//calculating scaling factor
		}
#ifdef _WIN32
		if(nDeviceID_1 >= maxDeviceCount_fromFile)
		{
			pos[nDeviceID_1]= 0;
			return 1;
		}
#endif
		
		do
		{
			//Scan for initial Positions 
			fscanf(fp,"$node_(%d) set X_ %lf ",&nDeviceID_fromFile, &X);
			fscanf(fp,"$node_(%d) set Y_ %lf ",&nDeviceID_fromFile, &Y);
			fscanf(fp,"$node_(%d) set Z_ %lf ",&nDeviceID_fromFile, &Z);
			if(nDeviceID_fromFile == nDeviceID_1)
			{
				if(ANIMATION)
				{
					//Scale the coordinates if user requires animation
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->X = X/scalingFactor;
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->Y = Y/scalingFactor;
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->Z = Z/scalingFactor;
				}
				else
				{
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->X = X;
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->Y = Y;
					NETWORK->ppstruDeviceList[nDeviceID_fromFile]->pstruDevicePosition->Z = Z;
				
				}
			}
		}while(nDeviceID_fromFile != maxDeviceCount_fromFile-1);
		
		fgetpos(fp,&pos[nDeviceID_1]);
		fsetpos(fp,&startPos);
	}
	else
		fnNetSimError("FileCannotBeOpened");

return 1;	
	
}
/** This function is to take the positions from the file and change the node positions */
int FileBasedMobility(int nDeviceID, double PresentTime)
{
	double X, Y, Z ;
	double time_fromFile ;
	int nDeviceID_fromFile, i=0;
	MOBILITY_VAR* pstruMobilityVar=NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDeviceMobility->pstruMobVar;
	
#ifdef _WIN32
	if(pos[nDeviceID-1] == 0)
		return 7;
#else
	if(nDeviceID >= maxDeviceCount_fromFile) //if nDeviceID is greater than maxDeviceCount_fromFile then return function 
		return 7;
#endif
	
	if(PresentTime <= pstruEventDetails->dEventTime)
	{
		if(fp)
		{
			fsetpos(fp, &pos[nDeviceID-1]);
			do
				{
					if(fscanf(fp,"$time %lf \"$node_(%d) %lf %lf %lf\" ",&time_fromFile,&nDeviceID_fromFile,&X,&Y,&Z) != 5)
					{
						return 0; // End of file
					}
				}while(nDeviceID-1 != nDeviceID_fromFile);
			if(time_fromFile*1000000==PresentTime && nDeviceID-1 == nDeviceID_fromFile)
			{
				//change the position of the node
				if(ANIMATION)
				{
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->X = X/scalingFactor;
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->Y = Y/scalingFactor;
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->Z = Z/scalingFactor;
				}
				else
				{
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->X = X;
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->Y = Y;
					NETWORK->ppstruDeviceList[nDeviceID-1]->pstruDevicePosition->Z = Z;
				}
				
				//Animate the node			
				fn_NetSim_Anim_MoveNode(pstruEventDetails->nDeviceId,
						(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->X,
						(int)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDevicePosition->Y,
						pstruEventDetails->dEventTime);
	
				fgetpos(fp, &pos[nDeviceID-1]);
				//take input till next event where time > PresentTime
				//this is to set next event in the EventList
				do
				{
					if(fscanf(fp,"$time %lf \"$node_(%d) %lf %lf %lf\" ",&time_fromFile,&nDeviceID_fromFile,&X,&Y,&Z) != 5)
					{
						return 1; //End OF File
					}
				}while(nDeviceID-1 != nDeviceID_fromFile);
				pstruMobilityVar->dLastTime = time_fromFile*1000000;
				
				//add event at this time, for this nDeviceID;
				pstruEventDetails->dEventTime = time_fromFile*1000000;
				//pstruEventDetails->nDeviceId = nDeviceID;
				//pstruEventDetails->nEventType = TIMER_EVENT;
				//pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
				fnpAddEvent(pstruEventDetails);
				return 2;
			}
			else if(nDeviceID-1 == nDeviceID_fromFile && time_fromFile*1000000>PresentTime)
			{
				//This else is entered if at time 0 sec, Position Details for 
				//some node are not changed/ not present in the text file


				//Update Time
				pstruMobilityVar->dLastTime = time_fromFile*1000000;		
				
				//Add Event
				pstruEventDetails->dEventTime = time_fromFile*1000000;
				pstruEventDetails->nDeviceId = nDeviceID;
				pstruEventDetails->nEventType = TIMER_EVENT;
				pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
				fnpAddEvent(pstruEventDetails);
				return 3;
			}
		}
		else
		{
			fnSystemError("FileCannotBeOpened");
			return 4;
		}
		return 5;
	}
	return 6; 
	
}
