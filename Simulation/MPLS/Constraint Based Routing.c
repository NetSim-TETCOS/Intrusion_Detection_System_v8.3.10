//Constraint Based Routing


 

#include<stdio.h>

#include<stdlib.h>

# define MAXNODES 25

# define MAXROUTERS 20

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

 

/* this structure is for the explicit route

 * This is used only for CR-LDP

 */

typedef struct struExplicitRoute

{

            int nDeviceId;

            struct struExplicitRoute *pstruNextentry;

}ROUTE;

 

int fn_NP_MPLS_AddExplicitRoute(int,int,ROUTE**,ROUTE**);

int pcRowCost[100]; //for storing cost

int pcPath[100]; // for storing the Path(if there is path the value is Router Id else0)

 

/* This function is used to find explicit route from ingress router to egress rotuer

 * Using the explicit route only, router will forward Label request message

 * According to the user constraint, explicit route will be found using constraint based algorithm

 * To find constraint path (Explicit route from ingress to egress router),

 * Prune (cut) the links which are not meet the constraint

 * Apply the dijkstra algorithm

 */

_declspec(dllexport)ROUTE *fnNPMPLSConstraintBasedRouting(double dUserBandwidth,int nIndex,struct stru_NP_Router_OSPF_LinkStateDatabase *pstruOSPFDataBase,int nDestinationNode)

{

            int nLoop=0;

            int nLoop1=0;

            int nCount;

            int pcNodeVisited[MAXDevices]; // for storing the node visited value

            int nMin=65355; // for storing minimum cost

            int nVertex =0; // for storing vertex value

            int nIsFinish =0; //for checking true or false condition

            // Create temporary object for ROUTE

            ROUTE *pstruRouteFromIngress[MAXDevices];

            // Create temporary reference object for ROUTE

            ROUTE *pstruReference[MAXDevices];

            /*MAXROUTERS value is 15. MAXNODES value is 25.

             * Routers are numbered as 1,2,....15

             * CPEs (node) are numbered as MAXROUTERS+1 (i.e) 16,17.....45

             */

            nCount = MAXROUTERS+MAXNODES; //Number of total device

            // This loop for set prune flag for all link

            while(nLoop<MAXROUTERS)

            {

                        for(nLoop1=0;nLoop1<MAXROUTERS;nLoop1++)

                        {

                                    // Set the prune flag as 0

                                    (pstruOSPFDataBase)->nPruneFlag[nLoop][nLoop1] = 0;

                                    // If the link bandwidth is less than the user bandwidth

                                    if((pstruOSPFDataBase)->dReservableBandwidth[nLoop][nLoop1] < dUserBandwidth)

                                                // Set prune flag as 1

                                                (pstruOSPFDataBase)->nPruneFlag[nLoop][nLoop1] = 1;

                        }

                        nLoop++;

                        nLoop1=0;

            }

            // Set the temporary objects of ROUTE structure as NULL for all routers

            for(nLoop1=0;nLoop1<nCount;nLoop1++)

            {

                        pstruRouteFromIngress[nLoop1] = NULL;

                        pstruReference[nLoop1] = NULL;

            }

            // Call Explicit route function

            fn_NP_MPLS_AddExplicitRoute(0,nIndex,&pstruRouteFromIngress[0],&pstruReference[0]);

            for(nLoop=0;nLoop<nCount;nLoop++)

            {

                        if(nLoop == nIndex-1) // for the current router itself

                        {

                                    // Set cost as 0

                                    pcRowCost[nLoop] = 0;

                                    // Add the loop value in the path

                                    pcPath[nLoop]= nLoop+1;

                        }

                        else if((pstruOSPFDataBase)->nCost[nIndex-1][nLoop] == 0) // for other devices if cost =0

                        {//This condition is used to check whether the cost of link is 0 or 1

                                    // Set the path loop as o

                                    pcPath[nLoop] = 0;

                                    // Set cost as highest integer value

                                    pcRowCost[nLoop]=65355;

                        }

                        else // if cost >0

                        {//This is the alternate condition for the above two conditions,

 

                                    if((pstruOSPFDataBase)->nPruneFlag[nIndex-1][nLoop] != 1) //if the prune flag of link is not equal to one.

                                    {

                                                // Assign the cost of the link

                                                pcRowCost[nLoop]=(pstruOSPFDataBase)->nCost[nIndex-1][nLoop];

                                                // Assign the index (current router) id in the path loop

                                                pcPath[nLoop] = nIndex;

 

                                    }

                                    else

                                    { // There is path from the index (current router). But the cost of the path is less than the user bandwidth

                                                // Set path loop as 0

                                                pcPath[nLoop] = 0;

                                                // Set cost as highest integer value

                                                pcRowCost[nLoop]=65355;

                                    }

                                    // Call the explicit route function

                                    fn_NP_MPLS_AddExplicitRoute(nIndex,nLoop+1,&pstruRouteFromIngress[0],&pstruReference[0]);

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

                        //set the path for the corresponding index as 0

                        pcPath[nIndex-1]=0;

                        nMin = 0;

                        // this loop for finding the minimum cost node which is connected in the index

                        for(nLoop=0;nLoop<nCount;nLoop++)

                        {

                                    if(pcNodeVisited[nLoop] == 0) // check the node visited array

                                    {

                                                //if(pstru_NP_Router_OSPF_LSAARRAY->dReservableBandwidth[nIndex-1][nLoop] > dUserBandwidth || (nLoop > 14 && pcRowCost[nLoop]==1))

                                                //{

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

                                                // If the prune flag of the link is 0

                                                if((pstruOSPFDataBase)->nPruneFlag[nVertex][nLoop] != 1 || (nLoop > MAXROUTERS && pcRowCost[nLoop]>1))

                                                {

                                                            //for that nodes check the cost using the vertex value from the Link State Database

                                                            if((pstruOSPFDataBase)->nCost[nLoop][nVertex] > 0 )

                                                            {

                                                                        // this is for finding shortest path

                                                                        if((pcRowCost[nVertex] + (pstruOSPFDataBase)->nCost[nLoop][nVertex]) < pcRowCost[nLoop]  )

                                                                        {

                                                                                    //add the 2nodes cost(one is vertex node another one is the node which is connected from the vertex node

                                                                                    pcRowCost[nLoop] = pcRowCost[nVertex] + (pstruOSPFDataBase)->nCost[nLoop][nVertex];//change

                                                                                    pcPath[nLoop] = nVertex; //set path value for that node by using vertex

                                                                                    // Call the explicit route function

                                                                                    fn_NP_MPLS_AddExplicitRoute(nVertex+1,nLoop+1,&pstruRouteFromIngress[0],&pstruReference[0]);

                                                                        }

                                                                        else

                                                                        {

 

                                                                        }

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

            // return the explicit route

            return pstruRouteFromIngress[(nDestinationNode+MAXROUTERS)-1];

}

/* this function is used to find the route for all device from the index*/

int fn_NP_MPLS_AddExplicitRoute(int nVertex,int nDeviceId,ROUTE **pstruTempRouteFromIngress,ROUTE **pstruTempReference)

{

            // Create temporary objects for explicit rotue structure

            struct struExplicitRoute *TempExplicitRoute;

            struct struExplicitRoute *TempExplicitRoute1;

 

            if(nVertex == 0)

            { // i.e for index

                        // Allocate memory

                        TempExplicitRoute1 = (struct struExplicitRoute*)malloc(sizeof(struct struExplicitRoute));

                        // Assign the device id

                        TempExplicitRoute1->nDeviceId = nDeviceId;

                        TempExplicitRoute1->pstruNextentry = NULL;

                        // Assign the temporary object in the explicit route of the device

                        pstruTempRouteFromIngress[nDeviceId-1] = TempExplicitRoute1;

            }

            else

            {

                        // Set the explicit route of device as NULL

                        pstruTempRouteFromIngress[nDeviceId-1] = NULL;

                        pstruTempReference[nDeviceId-1] = NULL;

                        // Get the explicit route of vertex

                        TempExplicitRoute = pstruTempRouteFromIngress[nVertex-1];

                        // This loop for assign the explicit route of vertex in the explicit route of device

                        while(TempExplicitRoute != NULL)

                        {

                                    // Allocate memory for temporary object

                                    TempExplicitRoute1 = (struct struExplicitRoute*)malloc(sizeof(struct struExplicitRoute));

                                    //Assign the device id from the explicit route of vertex

                                    TempExplicitRoute1->nDeviceId = TempExplicitRoute->nDeviceId;

                                    TempExplicitRoute1->pstruNextentry = NULL;

                                    // Assign the temporary object in the explicit route of device

                                    if(pstruTempRouteFromIngress[nDeviceId-1] == NULL)

                                    {

                                                pstruTempRouteFromIngress[nDeviceId-1] = TempExplicitRoute1;

                                                pstruTempReference[nDeviceId-1] = TempExplicitRoute1;

                                    }

                                    else

                                    {

                                                (pstruTempReference[nDeviceId-1])->pstruNextentry = TempExplicitRoute1;

                                                pstruTempReference[nDeviceId-1] = TempExplicitRoute1;

                                    }

                                    TempExplicitRoute = TempExplicitRoute->pstruNextentry;

                        }

                        /* this loop for assign the device id in the explicit route of device

                         * for CPE's there is no need of assign device id

                         * Thats why the condition nDeviceId < MAXROUTERS condition is checked

                         */

                        if(nDeviceId < MAXROUTERS+1)

                        {

                                    if(TempExplicitRoute == NULL)

                                    {

                                                // Allocate memory for temporary object

                                                TempExplicitRoute1 = (struct struExplicitRoute*)malloc(sizeof(struct struExplicitRoute));

                                                //Assign the device id

                                                TempExplicitRoute1->nDeviceId = nDeviceId;

                                                TempExplicitRoute1->pstruNextentry = NULL;

                                                // Assign the temporary object in the explicit route of device

                                                if(pstruTempRouteFromIngress[nDeviceId-1] == NULL)

                                                {

                                                            pstruTempRouteFromIngress[nDeviceId-1] = TempExplicitRoute1;

                                                            pstruTempReference[nDeviceId-1] = TempExplicitRoute1;

                                                }

                                                else

                                                {

                                                            (pstruTempReference[nDeviceId-1])->pstruNextentry = TempExplicitRoute1;

                                                            pstruTempReference[nDeviceId-1] = TempExplicitRoute1;

                                                }

                                    }

                        }

            }

            return 0;

}

 
