//Dijkstra Algorithm

  

 

#include<stdio.h>

#include<stdlib.h>

#include<String.h>

#define MAXDevices 45

 

//this structure for maintaining the link state database

struct stru_NP_Router_OSPF_LinkStateDatabase

{

            int nCost[MAXDevices][MAXDevices]; // stores the cost

            char *szIPAddress[MAXDevices][MAXDevices]; //stores the IP address

            char *szConnectedDeviceIPAddress[MAXDevices][MAXDevices]; //stores the connected device ip address

      char *szSubnetMask[MAXDevices][MAXDevices];

            int nPortNumber[MAXDevices][MAXDevices]; //stores the port number

            double dReservableBandwidth[MAXDevices][MAXDevices];

            int nPruneFlag[MAXDevices][MAXDevices];

};

 

// This structure for maintaining the shortest path

typedef struct struShortestPath

{

            int nDeviceId;

            int nCost;

            struct struShortestPath *pstruNextentry;

}ROUTE;

 

 

int pcRowCost[100]; //for storing cost

 

int pcPath[100]; // for storing the Path(if there is path the value is Router Id else0)

 

int fn_NP_Router_ShortestPath(int,int,ROUTE**,ROUTE**);

 

_declspec(dllexport)int fnNPRouterOSPFDijkstraAlgorithm(int nIndex,int nCount,struct stru_NP_Router_OSPF_LinkStateDatabase *pstruLinkStateDataBase,ROUTE *pstruShortestpathFromRouter[])

{

            int nLoop=0;

            int nLoop1=0;

            int pcNodeVisited[100]; // for storing the node visited value

            int nMin=65355; // for storing minimum cost

            int nVertex =0; // for storing vertex value

            int nIsFinish =0; //for checking true or false condition

            // Create temporary object for ROUTE

            //ROUTE *pstruShortestpathFromRouter[50];

            // Create temporary reference object for ROUTE

            ROUTE *pstruReference[50];

            /*MAXROUTERS value is 15. MAXNODES value is 25.

             * Routers are numbered as 1,2,....15

             * CPEs (node) are numbered as MAXROUTERS+1 (i.e) 16,17.....45

             */

            // Set the temporary objects of ROUTE structure as NULL for all routers

            for(nLoop1=0;nLoop1<nCount;nLoop1++)

            {

                        pstruShortestpathFromRouter[nLoop1] = NULL;

                        pstruReference[nLoop1] = NULL;

            }

            // Call shortest path function

            fn_NP_Router_ShortestPath(0,nIndex,&pstruShortestpathFromRouter[0],&pstruReference[0]);

            for(nLoop=0;nLoop<nCount;nLoop++)

            {

                        if(nLoop == nIndex-1) // for the current router itself

                        {

                                    // Set cost as 0

                                    pcRowCost[nLoop] = 0;

                        }

                        else if((pstruLinkStateDataBase)->nCost[nIndex-1][nLoop] == 0) // for other devices if cost =0

                        {//This condition is used to check whether the cost of link is 0 or 1

                                    // Set cost as highest integer value

                                    pcRowCost[nLoop]=65355;

                        }

                        else // if cost >0

                        {//This is the alternate condition for the above two conditions,

 

                                    // Assign the cost of the link

                                                pcRowCost[nLoop]=(pstruLinkStateDataBase)->nCost[nIndex-1][nLoop];

                                                // Call the explicit route function

                                                fn_NP_Router_ShortestPath(nIndex,nLoop+1,&pstruShortestpathFromRouter[0],&pstruReference[0]);

                        }

                        //Initially make the NODE visited array variable to zero for all node

                        pcNodeVisited[nLoop]=0;

            }

            //Now make the Node visited array variable as 1 for the corresponding index of the node

            pcNodeVisited[nIndex-1]=1;

            pcRowCost[nIndex-1]=0;

            //Loop will execute till the Node visiting completes

            while(!nIsFinish)

            {

                        nMin = 0;

                        // this loop for finding the minimum cost node which is connected in the index

                        for(nLoop=0;nLoop<nCount;nLoop++)

                        {

                                    if(pcNodeVisited[nLoop] == 0) // check the node visited array

                                    {

                                                if(nMin == 0) // first time the nMin value is zero

                                                {

                                                            nMin = pcRowCost[nLoop]; //assign the cost value for corresponding node to nMim

                                                            nVertex = nLoop; // set the vertex value for corresponding node

 

                                                }

                                                else

                                                {

                                                            if((nMin >= pcRowCost[nLoop]) && (pcRowCost[nLoop] != 0)) //if the corresponding node's cost is not equal to zero

                                                            {

                                                                        nMin = pcRowCost[nLoop]; // assign that cost as nMin

                                                                        nVertex = nLoop; // set the vertex value for that node

                                                            }

                                                }

                                                //}

                                    }

                        }

                        pcNodeVisited[nVertex]=1; //set the node visited array as 1 for the nodes which have the minimum cost

                        //nVertex1 = nVertex;

                        // this is for finding shortest path between the index and all nodes which are not connected with index in the network

                        for(nLoop=0;nLoop<nCount;nLoop++)

                        {

                                    //for that nodes node visited array is zero because there is no connection between them

                                    if(pcNodeVisited[nLoop] == 0)

                                    {

                                                //for that nodes check the cost using the vertex value from the Link State Database

                                                if((pstruLinkStateDataBase)->nCost[nLoop][nVertex] > 0 )

                                                {

 

                                                            // this is for finding shortest path

                                                            if((pcRowCost[nVertex] + (pstruLinkStateDataBase)->nCost[nLoop][nVertex]) < pcRowCost[nLoop]  )

                                                            {

                                                                        //add the 2nodes cost(one is vertex node another one is the node which is connected from the vertex node

                                                                        pcRowCost[nLoop] = pcRowCost[nVertex] + (pstruLinkStateDataBase)->nCost[nLoop][nVertex];//change

                                                                        // Call the explicit route function

                                                                        fn_NP_Router_ShortestPath(nVertex+1,nLoop+1,&pstruShortestpathFromRouter[0],&pstruReference[0]);

                                                            }

                                                            else

                                                            {

 

                                                            }

 

                                                }

                                    }

                        }

                        //this for checking whether the index visit the all nodes or not

                        for(nLoop=0;nLoop<nCount;nLoop++)

                        {

                                    nIsFinish=1;

                                    if(pcNodeVisited[nLoop] == 0) // if not means set nIsFinish 0 and continue while loop

                                    {

                                                nIsFinish=0;

                                                break;

                                    }

 

                        }//if the index visits the all nodes(i.e node visiting completes) in the network then terminate the while loop

 

            }

            return 0;

}

/* this function is used to find the route for all device from the index*/

int fn_NP_Router_ShortestPath(int nVertex,int nDeviceId,ROUTE **pstruTempRouteFromRouter,ROUTE **pstruTempReference)

{

            // Create temporary objects for explicit route structure

            struct struShortestPath *TempShortestPath;

            struct struShortestPath *TempShortestPath1;

 

            if(nVertex == 0)

            { // i.e for index

                        // Allocate memory

                        TempShortestPath1 = (struct struShortestPath*)malloc(sizeof(struct struShortestPath));

                        // Assign the device id

                        TempShortestPath1->nDeviceId = nDeviceId;

                        TempShortestPath1->nCost = pcRowCost[nDeviceId-1];

                        TempShortestPath1->pstruNextentry = NULL;

                        // Assign the temporary object in the explicit route of the device

                        pstruTempRouteFromRouter[nDeviceId-1] = TempShortestPath1;

            }

            else

            {

                        // Set the explicit route of device as NULL

                        pstruTempRouteFromRouter[nDeviceId-1] = NULL;

                        pstruTempReference[nDeviceId-1] = NULL;

                        // Get the explicit route of vertex

                        TempShortestPath = pstruTempRouteFromRouter[nVertex-1];

                        // This loop for assign the explicit route of vertex in the explicit route of device

                        while(TempShortestPath != NULL)

                        {

                                    // Allocate memory for temporary object

                                    TempShortestPath1 = (struct struShortestPath*)malloc(sizeof(struct struShortestPath));

                                    //Assign the device id from the explicit route of vertex

                                    TempShortestPath1->nDeviceId = TempShortestPath->nDeviceId;

                                    TempShortestPath1->nCost = pcRowCost[nDeviceId-1];

                                    TempShortestPath1->pstruNextentry = NULL;

                                    // Assign the temporary object in the explicit route of device

                                    if(pstruTempRouteFromRouter[nDeviceId-1] == NULL)

                                    {

                                                pstruTempRouteFromRouter[nDeviceId-1] = TempShortestPath1;

                                                pstruTempReference[nDeviceId-1] = TempShortestPath1;

                                    }

                                    else

                                    {

                                                (pstruTempReference[nDeviceId-1])->pstruNextentry = TempShortestPath1;

                                                pstruTempReference[nDeviceId-1] = TempShortestPath1;

                                    }

                                    TempShortestPath = TempShortestPath->pstruNextentry;

                        }

                        /* this loop for assign the device id in the explicit route of device

 

                         */

                        if(TempShortestPath == NULL)

                        {

                                    // Allocate memory for temporary object

                                    TempShortestPath1 = (struct struShortestPath*)malloc(sizeof(struct struShortestPath));

                                    //Assign the device id

                                    TempShortestPath1->nDeviceId = nDeviceId;

                                    TempShortestPath1->nCost = pcRowCost[nDeviceId-1];

                                    TempShortestPath1->pstruNextentry = NULL;

                                    // Assign the temporary object in the explicit route of device

                                    if(pstruTempRouteFromRouter[nDeviceId-1] == NULL)

                                    {

                                                pstruTempRouteFromRouter[nDeviceId-1] = TempShortestPath1;

                                                pstruTempReference[nDeviceId-1] = TempShortestPath1;

                                    }

                                    else

                                    {

                                                (pstruTempReference[nDeviceId-1])->pstruNextentry = TempShortestPath1;

                                                pstruTempReference[nDeviceId-1] = TempShortestPath1;

                                    }

                        }

 

            }

            return 0;

}

 
