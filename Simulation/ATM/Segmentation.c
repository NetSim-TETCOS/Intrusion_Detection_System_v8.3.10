/************************************************************************************

 * Copyright (C) 2012                                                               *

 * TETCOS, Bangalore. India                                                         *

 *                                                                                  *

 * Tetcos owns the intellectual property rights in the Product and its content.     *

 * The copying, redistribution, reselling or publication of any or all of the       *

 * Product or its content without express prior written consent of Tetcos is        *

 * prohibited. Ownership and / or any other right relating to the software and all *

 * intellectual property rights therein shall remain at all times with Tetcos.      *

 *                                                                                  *

 * Author:    Shashi Kant Suman                                                        *

 *                                                                                  *

 * ---------------------------------------------------------------------------------*/

# ifndef ATM_H_
# include "ATM.h"
# endif

_declspec(dllexport) int m_fnAtmLayer(int,USERDATA*);

/*******  int m_fnAtmAdaptionLayerSegmentation(ATM_DATASTRUCTURE**) ***********

	 Purpose: Fragment the packet into cell and call ATM layer
				Called in Node and Router in DataLinkOut Event.
	 Algorithm:
		¢	Declare necessary variable
		¢	Get the packet length
		¢	Get the AAL type
		¢	Check for valid packet size
		¢	Check for valid AAL type
		¢	For all AAL type:
			o	Calculate no of cells
			o	Allocate memory for new cell list
			o	Calculate padding
			o	Set cell destination and source node
			o	Form cell
		¢	Call ATM function


  **************************************************************************************/

 _declspec(dllexport) int m_fnAtmAdaptionLayerSegmentation(ATM_DATASTRUCTURE** pstruATMData)
{

	USERCELL *Temp_Cell;	//Temporary cell
	USERCELL *Temp_Cell1;	//Temporary cell1
	USERDATA* pstru_NP_ATM_DATA=NULL;
	int nPacket_Size;	//packet size in integer
	int i=0;			//Loop counter
	int nPadding=0;		//Padding value (Max 48 bytes)
	int nAAL_Type=0;		//AAL type (Possible value = 0,1,2,3,4,5).
	double ldArrivalTime;//Arrival time of packet
	int nNo_of_Cells=0;	//No of cells formed from a packet

	USERDATA *pstru_NP_ATM_DATAHEAD;	/* Head Data*/
	double ld_NN_PacketSize;	/* Packet size*/
	int a; /*Number of cell*/
	pstru_NP_ATM_DATAHEAD = (*pstruATMData)->pstruHeadData; // Get the current data
	ld_NN_PacketSize = (*pstruATMData)->dPacketsize; //Get the packet size
	a = (*pstruATMData)->nNoOfCell; // Get the number of cell

	//Get the AAL types based on data type
	switch(pstru_NP_ATM_DATAHEAD->nDataType)
	{
	case 1: //Voice
		nAAL_Type = 2;
		break;
	case 2: //Data
		nAAL_Type = 5;
		break;
	case 3: //Control packet
		nAAL_Type = 0;
		break;
	}//End of switch

	//Getting the arrival time
	ldArrivalTime=pstru_NP_ATM_DATAHEAD->dArrivalTime; // Arrival time

	//Convert double packet size to int packet size.
	nPacket_Size=(int)ld_NN_PacketSize;

	/********************* Go to AAL type condition ***********************************/
	if(nAAL_Type==0)// For AAL0
	{
		nNo_of_Cells=nPacket_Size/48; //AAL0 has payload of 48 byte so divided by 48
		i=0;

		//Calculate padding value
		if(nPacket_Size%48!=0) 	//Last cell
		{
			nNo_of_Cells++;		// Increase the number of cell by 1.
			nPadding=48-(nPacket_Size%48);
		}

		pstru_NP_ATM_DATA=pstru_NP_ATM_DATAHEAD;

		//Traverse through each cell
		for(i=0;i<nNo_of_Cells;i++)
		{
			if(i!=nNo_of_Cells-1 && i!=0) //For intermediate cell
			{
				// Forming intermediate cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellOverhead=0; // AAL0 add no overhead
				Temp_Cell->n_NI_CellPadd=0;     // No padding for intermediate cell
				Temp_Cell->n_NI_CellPayload=48; // AAL0 has cell payload = 48 bytes
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; // Cell arrival time = packet arrival time
				Temp_Cell->ld_CellSize = 53;     // Cell size = 53 //ATM
				Temp_Cell->n_NI_CellSequenceNo=i+1; // cell no.
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->pstru_NI_NextCells=NULL; // next cell = NULL


				Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
				Temp_Cell->n_NI_Cellflag=0;  // Cell flag is 0 for intermediate cell

				// Adding new cell to last of cell list
				while(Temp_Cell1->pstru_NI_NextCells!=NULL)
				{
					Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
				}
				Temp_Cell1->pstru_NI_NextCells=Temp_Cell;

			}//End of if(Intermediate cell)

			if(i==0) //for first cell
			{
				// Forming first cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1; // Cell seq. no
				Temp_Cell->n_NI_CellOverhead=0;     // AAL0 add no overhead
				Temp_Cell->n_NI_CellPadd=0;			// No padding for first cell
				Temp_Cell->n_NI_CellPayload=48;		// AAL0 cell payload = 48
				Temp_Cell->ld_CellSize = 53;        // ATM cell Size = 53
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; // Cell arrival time = packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL; // Next cell = NULL
				Temp_Cell->n_NI_Cellflag=0;			// Cell flag = 0 for first cell

				//Adding cell to ATM data list
				pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;

			}//End of if(First cell)

			if(i==nNo_of_Cells-1) //for last cell
			{
				// Forming last cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1;   // Cell seq. no
				Temp_Cell->n_NI_CellOverhead=0;		  // AAL0 add no overhead
				Temp_Cell->n_NI_CellPadd=nPadding;    // Padding in the last cell
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_CellPayload=48-nPadding; // payload of last cell is less due to padding
				Temp_Cell->ld_CellSize = 53;          // ATM cell size = 53
				Temp_Cell->n_NI_Cellflag=1; 		  // cell flag is 1 for last cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; // Cell arrival time = packet arrival time

				Temp_Cell->pstru_NI_NextCells=NULL;
				if(i!=0)  // if only 1 cell is formed from a packet
				{
					Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
					while(Temp_Cell1->pstru_NI_NextCells!=NULL)
					{
						Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
					}
					Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
				}
				else //More than one cell
				{
					//Add cell to last of cell list
					Temp_Cell->pstru_NI_NextCells=NULL;
					pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
				}

			}
		}//End of for(transversing through the cell list)
	}//End of AAL0

	if(nAAL_Type==1)//For AAL1
	{
		//getting the number of cell
		nNo_of_Cells=nPacket_Size/47; //AAL1 has payload of 47 byte so divided by 47
		i=0;

		//Calculating the padding value
		if(nPacket_Size%47!=0) //Last cell
		{
			nNo_of_Cells++;		//Increase the number of cell by 1
			nPadding=47-(nPacket_Size%47); // Padding = 47 - remaining bytes.
		}

		pstru_NP_ATM_DATA=pstru_NP_ATM_DATAHEAD;

		//Transverse through the cell list
		for(i=0;i<nNo_of_Cells;i++)
		{
			if(i!=nNo_of_Cells-1 && i!=0) //For intermediate cell
			{
				//Allocating memory for intermediate cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Set the cell sequence number
				Temp_Cell->n_NI_CellOverhead=1; // AAL1 add 1 byte header
				Temp_Cell->n_NI_CellPadd=0;		// Intermediate cell so no padding
				Temp_Cell->n_NI_CellPayload=47; // AAL1 cell payload = 47
				Temp_Cell->ld_CellSize = 53;	// ATM so cell size = 53
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=0;     // Intermediate cell so cell flag = 0
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Set the cell arrival time = packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL;

				// Adding new cell to last of the cell list
				Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
				while(Temp_Cell1->pstru_NI_NextCells!=NULL)
				{
					Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
				}
				Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
			}//End of if(Intermediate cell)
			if(i==0) //for first cell
			{
				//Allocating memory for first cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1;//Set the cell sequence number
				Temp_Cell->n_NI_CellOverhead=1; //AAL1 add 1 byte header
				Temp_Cell->n_NI_CellPadd=0;     // First cell so padd = 0
				Temp_Cell->n_NI_CellPayload=47; // AAL1 has payload = 47
				Temp_Cell->ld_CellSize = 53;    // ATM so cell size = 53
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=-1;     // First cell so cell flag = 0
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Set the arrival time = packet arrival time

				Temp_Cell->pstru_NI_NextCells=NULL;

				//Adding first cell to cell list
				pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
			}//End of if(First cell)

			if(i==nNo_of_Cells-1) //for last cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1;//Set the cell sequence number
				Temp_Cell->n_NI_CellOverhead=1;  // AAL1 add 1 byte header
				Temp_Cell->n_NI_CellPadd=nPadding; // Last cell so padding
				Temp_Cell->n_NI_CellPayload=47-nPadding;
				Temp_Cell->ld_CellSize = 53; // ATM so cell size = 53
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=1; // Last cell so cell flag =1
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime;
				Temp_Cell->pstru_NI_NextCells=NULL;

				//Add cell to the ATM cell list
				if(i!=0)  // if only 1 cell formed in a packet
				{
					Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
					while(Temp_Cell1->pstru_NI_NextCells!=NULL)
					{
						Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
					}
					Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
				}
				else //More than 1 cell
				{
					Temp_Cell->pstru_NI_NextCells=NULL;
					pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
				}

			}//End of if(last cell)
		}//End of for(Transversing through cell list)
	}//End of if(AAL1)

	if(nAAL_Type==2)//AAL2
	{
		nPacket_Size=nPacket_Size+3; //AAL2 add 3 byte header in each packet
		nNo_of_Cells=nPacket_Size/47; //AAL2 has payload of 47 byte
		i=0;

		//Calculate the padding
		if(nPacket_Size%47!=0) //Last cell
		{
			nNo_of_Cells++;
			nPadding=47-(nPacket_Size%47); //Padding = 47 - remaining bytes
		}

		pstru_NP_ATM_DATA=pstru_NP_ATM_DATAHEAD;

		//Transverse through the cell list
		for(i=0;i<nNo_of_Cells;i++)
		{

			if(i!=nNo_of_Cells-1 && i!=0)//Intermediate cell
			{
				//Allocating memory for cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellOverhead=1; // AAL2 add 1 byte header
				Temp_Cell->n_NI_CellPadd=0;     // Intermediate cell so no padding
				Temp_Cell->n_NI_CellPayload=47; // AAL2 has cell payload = 47
				Temp_Cell->ld_CellSize = 53;    // ATM cell size = 53
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=0;     // intermediate cell so cell flag = 0
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime;//Set the cell arrival time = packet arrival time
				Temp_Cell->n_NI_CellSequenceNo=i+1;//set the cell sequence number
				Temp_Cell->pstru_NI_NextCells=NULL;

				//Add cell into the cell list
				Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
				while(Temp_Cell1->pstru_NI_NextCells!=NULL)
				{
					Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
				}
				Temp_Cell1->pstru_NI_NextCells=Temp_Cell;

			}//End of if(Intermediate cell)
			if(i==0) //first cell
			{
				//Allocate memory for cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1;//Set the cell sequence number
				Temp_Cell->n_NI_CellOverhead=4; //1+3 due to cell overhead +packet overhead
				Temp_Cell->n_NI_CellPadd=0;//No padding for first cell
				Temp_Cell->n_NI_CellPayload=44;//Cell payload= 44 bytes for first cell
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->ld_CellSize = 53;//ATM cell size = 53 bytes
				Temp_Cell->n_NI_Cellflag=-1;//Set the cell flag = -1 for first cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime;//Set the cell arrival time = packet arrival time

				Temp_Cell->pstru_NI_NextCells=NULL;

				//Add the cell into ATM cell list
				pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
			}//end of if(First cell)

			if(i==nNo_of_Cells-1)//Last cell
			{
				//Allocate memory for last cell
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellOverhead=1; //Last cell have over head only 1 bytes
				Temp_Cell->n_NI_CellPadd=nPadding;	//Last cell have padding value
				Temp_Cell->n_NI_CellSequenceNo=i+1;//Set the cell sequence number
				Temp_Cell->n_NI_CellPayload=47-nPadding;//Last cell payload = remaining bytes = 47-padding
				Temp_Cell->ld_CellSize = 53;//ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=1;//Set the cell flag = 1 for last cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime;//Cell arrival time = Packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL;

				//Insert the cell into cell list
				if(i!=0) //For more than 1 cell formed from packet
				{
					//Insert the cell into end of the cell list
					Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
					while(Temp_Cell1->pstru_NI_NextCells!=NULL)
					{
						Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
					}
					Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
				}
				else //If only 1 packet formed from a packet
				{
					Temp_Cell->n_NI_CellOverhead=4;
					Temp_Cell->n_NI_CellPayload=44-nPadding;

					//Add the cell into cell list
					Temp_Cell->pstru_NI_NextCells=NULL;
					pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
				}//end of else

			}//end of if(Last cell)
		}//End of for(transverse through cell list)
	}//End of AAL2

	if(nAAL_Type==3 || nAAL_Type==4) // AAL3/4
	{
		nPacket_Size=nPacket_Size+8; //AAL3/4 add 4 byte header and 4 byte trailer in each packet
		nNo_of_Cells=nPacket_Size/44;//AAL3/4 has payload 44 byte
		i=0;
		if(nPacket_Size%44!=0) //Last cell
		{
			nNo_of_Cells++;	//Increase the number of cell by 1
			nPadding=44-(nPacket_Size%44); //Calculate the padding value for last cell
		}
		pstru_NP_ATM_DATA=pstru_NP_ATM_DATAHEAD;

		//Transverse through the cell list
		for(i=0;i<nNo_of_Cells;i++)
		{

			if(i!=nNo_of_Cells-1 && i!=0) //Intermediate cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1;
				Temp_Cell->n_NI_CellOverhead=4;//AAL3/4 Add 2 byte header and 2 byte trailer in each cell
				Temp_Cell->n_NI_CellPadd=0;	//Intermediate cell have no padding
				Temp_Cell->n_NI_CellPayload=44;	// AAL3/4 have cell payload = 44
				Temp_Cell->n_NI_Cellflag=0;	// Cell flag = 0 for intermediate cell
				Temp_Cell->ld_CellSize = 53;	//ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Set the cell arrival time = packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL; //Next cell = NULL;

				//Add to the cell list
				Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
				while(Temp_Cell1->pstru_NI_NextCells!=NULL)
				{
					Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
				}
				Temp_Cell1->pstru_NI_NextCells=Temp_Cell;

			}
			if(i==0) //First cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Cell number
				Temp_Cell->n_NI_CellOverhead=8;// 4 byte header in packet + AAL3/4 Add 2 byte header and 2 byte trailer in each cell
				Temp_Cell->n_NI_CellPadd=0; //First cell have no padding
				Temp_Cell->n_NI_CellPayload=40; //First cell of AAL3/4 have cell payload = 40 bytes.
				Temp_Cell->n_NI_Cellflag=-1; //Cell flag = -1 for first cell
				Temp_Cell->ld_CellSize = 53; //ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Cell arrival time = packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL; //Next cell = NULL

				//Adding to the cell list
				pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;

			}
			if(i==nNo_of_Cells-1)//Last cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Set the cell number
				Temp_Cell->n_NI_CellOverhead=8;// 4 byte trailer in packet + AAL3/4 Add 2 byte header and 2 byte trailer in each cell
				                       // so total overhead=4(only for last cell)+2+2=8
				Temp_Cell->n_NI_CellPadd=nPadding; //Set the padding value for last cell
				Temp_Cell->n_NI_CellPayload=40-nPadding; // Cell payload = 40 - padding.
				Temp_Cell->n_NI_Cellflag=1; //Cell flag = 1 for last cell
				Temp_Cell->ld_CellSize = 53; //ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Set the cell arrival time = packet arrival time
				Temp_Cell->pstru_NI_NextCells=NULL; //Next cell = NULL

				//Adding to the cell list
				if(i!=0) //If more than 1 cell
				{
					Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
					while(Temp_Cell1->pstru_NI_NextCells!=NULL)
					{
						Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
					}
					Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
				}
				else //Only one cell formed.
				{
					Temp_Cell->n_NI_CellOverhead=12; /* AAL3/4 add
													  * Packet header = 4 bytes.
													  * Packet tailer = 4 bytes
													  * Cell header = 2 bytes
													  * Cell tailer = 2 bytes.
													  * Total = 12 bytes.
													  */
					Temp_Cell->n_NI_CellPayload=36-nPadding;
					Temp_Cell->pstru_NI_NextCells=NULL;
					pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
				}

			}
		}

	}

	if(nAAL_Type==5)//AAL5
	{
		nPacket_Size=nPacket_Size+8;//AAL5 add 8 byte trailer in each packet
		nNo_of_Cells=nPacket_Size/48; //AAL5 has payload of 48 byte
		i=0;
		if(nPacket_Size%48!=0) //last cell
		{
			nNo_of_Cells++;
			//Calculate the padding value
			nPadding=48-(nPacket_Size%48);
		}
	pstru_NP_ATM_DATA=pstru_NP_ATM_DATAHEAD;

	//Transverse through the cell list
		for(i=0;i<nNo_of_Cells;i++)
		{
			if(i!=nNo_of_Cells-1 && i!=0) //Intermediate cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL)); //Allocate memory for the cell
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Cell numnber in a packet
				Temp_Cell->n_NI_CellOverhead=0;     //AAL5 has no cell overhead
				Temp_Cell->n_NI_CellPadd=0;		//No padding for intermediate cell
				Temp_Cell->n_NI_CellPayload=48; //AAL5 have cell payload = 48 bytes
				Temp_Cell->ld_CellSize = 53;	//ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=0; //Cell flag = 0 for intermediate cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Cell arrival time = packet arrival time

				//Add cell to the cell list
				Temp_Cell->pstru_NI_NextCells=NULL;
				Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
				while(Temp_Cell1->pstru_NI_NextCells!=NULL)
				{
					Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
				}
				Temp_Cell1->pstru_NI_NextCells=Temp_Cell;

			}
			if(i==0)//first cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL)); //Allocate memory for the cell
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Set the cell number in a packet
				Temp_Cell->n_NI_CellOverhead=0; //AAL5 cell don't have cell over head
				Temp_Cell->n_NI_CellPadd=0; //No padding for first cell
				Temp_Cell->n_NI_CellPayload=48; //AAL5 cell payload = 48 bytes
				Temp_Cell->ld_CellSize = 53; //ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=-1; //Cell flag = -1 for first cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //Cell arrival time = packet arrival time

				//Add cell to cell list
				Temp_Cell->pstru_NI_NextCells=NULL;
				pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;

			}
			if(i==nNo_of_Cells-1)//Last cell
			{
				Temp_Cell=(USERCELL*)calloc(1,sizeof(USERCELL));//Allocate memory for the cell
				Temp_Cell->n_NI_CellSequenceNo=i+1; //Set the cell number in apacket
				Temp_Cell->n_NI_CellOverhead=8; //8 byte trailer in packet, so overhead for last cell
				Temp_Cell->n_NI_CellPadd=nPadding; //LAst cell have padding
				Temp_Cell->ld_CellSize = 53; //ATM cell size = 53 bytes
				Temp_Cell->n_NI_CellPayload=40-nPadding; //Set payload for last cell
				Temp_Cell->n_NI_CellLossPriority=0;
				Temp_Cell->n_NI_Cellflag=1; //Cell flag = 1 for last cell
				Temp_Cell->ld_NI_CellArrivalTime=ldArrivalTime; //cell arrival time = packet arrival time

				if(i!=0) //More than one cell
				{
					Temp_Cell->pstru_NI_NextCells=NULL;
					Temp_Cell1=pstru_NP_ATM_DATA->pstruATMUserCell;
					while(Temp_Cell1->pstru_NI_NextCells!=NULL)
					{
						Temp_Cell1=Temp_Cell1->pstru_NI_NextCells;
					}
					Temp_Cell1->pstru_NI_NextCells=Temp_Cell;
				}
				else //Only one cell
				{
					Temp_Cell->pstru_NI_NextCells=NULL;
					pstru_NP_ATM_DATA->pstruATMUserCell=Temp_Cell;
				}

			}
		}

	}
	i=0;
	//Call ATM layer
	m_fnAtmLayer(nNo_of_Cells,pstru_NP_ATM_DATA);
	(*pstruATMData)->nNoOfCell = nNo_of_Cells; //Update the number of cell
	return 1;
}//End of function segmentation

 /*******************  int m_fnAtmLayer() **************************************
 	Purpose: Add 5 byte overhead to each ATM cell ( ATM Layer overhead)
 	Algorithm:
 		¢	Declare temp cell
 		¢	Temp cell = atm cell
 		¢	Add 5 byte overhead to each temp cell


   ***********************************************************************************/

 int m_fnAtmLayer(int nNo_of_Cells,USERDATA* pstru_NP_ATM_DATA)
 {
 	USERCELL *Temp_Cell;
 	Temp_Cell=pstru_NP_ATM_DATA->pstruATMUserCell;
 	while(Temp_Cell!=NULL)
 	{
 		//Add 5 byets overhead
 		Temp_Cell->n_NI_CellOverhead=Temp_Cell->n_NI_CellOverhead+5;
 		Temp_Cell=Temp_Cell->pstru_NI_NextCells;
 	}

 	return 0;
 }
