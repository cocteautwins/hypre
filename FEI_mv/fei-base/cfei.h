#ifndef _cfei_H_
#define _cfei_H_

/*------------------------------------------------------------------------------
   This is the header for the prototypes of the procedural ("C") version
   of the finite element interface.

   For explanations of parameters and semantics, see the C++ header, or
   doxygen output created there-from. With the exception of create/destroy
   functions, all FEI functions in this header mirror those in the C++ header
   but have 'FEI_' pre-pended to the name, and have 'CFEI* cfei' as the first
   argument, and arguments which are references in C++ (e.g., an output int)
   are pointers in this C interface.

   NOTE: ALL functions return an error code which is 0 if successful,
         non-zero if un-successful.

   Noteworthy special case: the solve function may return non-zero
   if the solver failed to converge. This is, of course, a non-fatal 
   situation, and the caller should then check the 'status' argument for
   possible further information (solver-specific/solver-dependent).
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
   First, we define a "Linear System Core" struct. This is the beast that
   handles all solver-library-specific functionality like sumIntoMatrix,
   launchSolver, etc., etc. The pointer 'lsc_' needs to hold an instance
   of an object which implements the C++ interface defined in
   ../base/LinearSystemCore.h. Naturally, an implementation-specific 
   function will be required to create one of these. 

   e.g., ISIS_LinSysCore_create(LinSysCore** lsc,
                                MPI_Comm comm);

   This function would be found in ../support-ISIS/cfei_isis.h, in the case
   of an ISIS++ FEI implementation. Each other FEI implementation will also
   need an equivalent function.
------------------------------------------------------------------------------*/

struct LinSysCore_struct {
   void* lsc_;
};
typedef struct LinSysCore_struct LinSysCore;

/*------------------------------------------------------------------------------
   The LinSysCore struct is going to obsolete, as soon as ESI objects become
   widely implemented. We will replace LinSysCore with a close cousin, which
   we'll call the "Linear System Manager". It will be more like a broker,
   handing out interfaces to individual objects such as matrices, vectors, etc.,
   on demand. For now, it will just be a wrapper around LinSysCore so that we
   don't lose our backwards compatibility.
------------------------------------------------------------------------------*/

struct LinSysMgr_struct {
   void* lsm_;
};
typedef struct LinSysMgr_struct LinSysMgr;


/*------------------------------------------------------------------------------
   Next, define an opaque CFEI thingy which will be an FEI context, and will
   be the first argument to all of the C FEI functions which follow in this
   header.
------------------------------------------------------------------------------*/

struct CFEI_struct {
   void* cfei_;
};
typedef struct CFEI_struct CFEI;

/*------------------------------------------------------------------------------
   And now, the function prototypes...
------------------------------------------------------------------------------*/

/* include fei_defs.h for the #defines of parameters such as FEI_LOCAL_TIMES,
  FEI_NODE_MAJOR, etc. */
#include <fei_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
   Initialization function. Creates an FEI instance, wrapped in a CFEI pointer.
*/
int FEI_create(CFEI** cfei,
               LinSysMgr* lsm,
               MPI_Comm FEI_COMM_WORLD, 
               int masterRank);

/* Function to create a LinSysMgr struct from a LinSysCore */
int LinSysMgr_create(LinSysMgr** lsm,
                     LinSysCore* lsc);

/* A function to destroy allocated memory. */
int FEI_destroy(CFEI** cfei);

/* A function to destroy those LinSysCore things. */
int LinSysCore_destroy(LinSysCore** lsc);

/* A function to destroy those LinSysMgr things. */
int LinSysMgr_destroy(LinSysMgr** lsm);

/*                                     */
/* And now all of the FEI functions... */
/*                                     */

int FEI_parameters(CFEI* cfei, 
                   int numParams, 
                   char **paramStrings);

int FEI_setIDLists(CFEI* cfei,
                   int numMatrices,
                   const int* matrixIDs,
                   int numRHSs,
                   const int* rhsIDs);

int FEI_setSolveType(CFEI* cfei, 
                     int solveType);

int FEI_initFields(CFEI* cfei, 
                   int numFields, 
                   int *fieldSizes, 
                   int *fieldIDs); 

int FEI_initElemBlock(CFEI* cfei, 
                      GlobalID elemBlockID, 
                      int numElements, 
                      int numNodesPerElement, 
                      int* numFieldsPerNode,
                      int** nodalFieldIDs,
                      int numElemDofFieldsPerElement,
                      int* elemDOFFieldIDs,
                      int interleaveStrategy); 

int FEI_initElem(CFEI* cfei, 
                 GlobalID elemBlockID, 
                 GlobalID elemID, 
                 GlobalID *elemConn);

int FEI_initSharedNodes(CFEI* cfei, 
                        int numSharedNodes, 
                        GlobalID *sharedNodeIDs,
                        int* numProcsPerNode,
                        int** sharingProcIDs);

int FEI_initCRMult(CFEI* cfei,
                   int numCRNodes,
                   GlobalID* CRNodes,
                   int *CRFields,
                   int* CRID);

int FEI_initCRPen(CFEI* cfei, 
                  int numCRNodes,
                  GlobalID* CRNodes, 
                  int *CRFields,
                  int* CRID); 

int FEI_initCoefAccessPattern( CFEI* cfei,
                               int patternID,
                               int numRowIDs,
                               int* numFieldsPerRow,
                               int** rowFieldIDs,
                               int numColIDsPerRow,
                               int* numFieldsPerCol,
                               int** colFieldIDs,
                               int interleaveStrategy );

int FEI_initCoefAccess( CFEI* cfei,
                        int patternID,
			int* rowIDTypes,
                        GlobalID* rowIDs,
			int* colIDTypes,
                        GlobalID* colIDs );

int FEI_initComplete(CFEI* cfei);

int FEI_resetSystem(CFEI* cfei, double s);
int FEI_resetMatrix(CFEI* cfei, double s);
int FEI_resetRHSVector(CFEI* cfei, double s);

int FEI_setCurrentMatrix(CFEI* cfei, int matID);
int FEI_setCurrentRHS(CFEI* cfei, int rhsID);

int FEI_loadNodeBCs(CFEI* cfei,
                    int numNodes,
                    GlobalID *BCNodes,
                    int fieldID,
                    double **alpha,
                    double **beta,
                    double **gamma);

int FEI_loadElemBCs( CFEI* cfei,
                     int numElems,
                     GlobalID *elemIDs,
                     int fieldID,
                     double **alpha,  
                     double **beta,  
                     double **gamma );

int FEI_sumInElem(CFEI* cfei, 
                  GlobalID elemBlockID, 
                  GlobalID elemID,
                  GlobalID* elemConn,
                  double **elemStiffness,
                  double *elemLoad,
                  int elemFormat);

int FEI_sumInElemMatrix(CFEI* cfei, 
                        GlobalID elemBlockID, 
                        GlobalID elemID,
                        GlobalID* elemConn,
                        double **elemStiffness,
                        int elemFormat);

int FEI_sumInElemRHS(CFEI* cfei, 
                     GlobalID elemBlockID, 
                     GlobalID elemID,
                     GlobalID* elemConn,
                     double *elemLoad);

int FEI_loadElemTransfer(CFEI* cfei,
                         GlobalID elemBlockID,
                         GlobalID elemID,
                         GlobalID* coarseNodeList,
                         int fineNodesPerCoarseElem,
                         GlobalID* fineNodeList,
                         double** elemProlong,
                         double** elemRestrict);

int FEI_loadCRMult(CFEI* cfei, 
                   int CRID, 
                   int numCRNodes,
                   GlobalID *CRNodes,  
                   int *CRFields,
                   double *CRWeights,
                   double CRValue);

int FEI_loadCRPen(CFEI* cfei, 
                  int CRID,
                  int numCRNodes, 
                  GlobalID *CRNodes,
                  int *CRFields,
                  double *CRWeights,  
                  double CRValue,
                  double penValue);

int FEI_sumIntoMatrix(CFEI* cfei,
		      int patternID,
		      int* rowIDTypes,
		      GlobalID* rowIDs,
		      int* colIDTypes,
		      GlobalID* colIDs,
		      double** matrixEntries);

int FEI_getFromMatrix(CFEI* cfei,
		      int patternID,
		      int* rowIDTypes,
		      GlobalID* rowIDs,
		      int* colIDTypes,
		      GlobalID* colIDs,
		      double** matrixEntries);

int FEI_putIntoMatrix(CFEI* cfei, int patternID,
		      int* rowIDTypes,
		      GlobalID* rowIDs,
		      int* colIDTypes,
		      GlobalID* colIDs,
		      double* * matrixEntries);

int FEI_sumIntoRHS(CFEI* cfei, int patternID,
		   int* IDTypes,
		   GlobalID* IDs,
		   double* vectorEntries);

int FEI_getFromRHS(CFEI* cfei, int patternID,
		   int* IDTypes,
		   GlobalID* IDs,
		   double* vectorEntries);

int FEI_putIntoRHS(CFEI* cfei, int patternID,
		   int* IDTypes,
		   GlobalID* IDs,
		   double* vectorEntries);

int FEI_setMatScalars(CFEI* cfei,
                      int numScalars,
                      int* IDs,
                      double* scalars);

int FEI_setRHSScalars(CFEI* cfei,
                      int numScalars,
                      int* IDs,
                      double* scalars);

int FEI_residualNorm(CFEI* cfei,
                      int whichNorm,
                     int numFields,
                     int* fieldIDs,
                     double* norms);

int FEI_solve(CFEI* cfei, int* status);

int FEI_iterations(CFEI* cfei, int* itersTaken);

int FEI_version(CFEI* cfei, char** versionStringPtr);

int FEI_cumulative_MPI_Wtimes(CFEI* cfei,
                              double* initTime,
                              double* loadTime,
                              double* solveTime,
                              double* solnReturnTime,
                              int timingMode);

int FEI_allocatedSize(CFEI* cfei,
                      int* bytes);

int FEI_getBlockNodeSolution(CFEI* cfei, 
                             GlobalID elemBlockID,
                             int numNodes, 
                             GlobalID* nodeIDs, 
                             int *offsets,
                             double *results);

int FEI_getBlockFieldNodeSolution(CFEI* cfei, 
                                  GlobalID elemBlockID,
                                  int fieldID,
                                  int numNodes, 
                                  GlobalID* nodeIDs, 
                                  double *results);

int FEI_getBlockElemSolution(CFEI* cfei, 
                             GlobalID elemBlockID,
                             int numElems, 
                             GlobalID *elemIDs, 
                             int* numElemDOFPerElement,
                             double *results);

int FEI_getNumCRMultipliers(CFEI* cfei, 
                            int* numMultCRs);

int FEI_getCRMultIDList(CFEI* cfei,
                        int numMultCRs,
                        int* multIDs);

int FEI_getCRMultipliers(CFEI* cfei,
                         int numMultCRs,
                         int* CRIDs,
                         double* multipliers);

int FEI_putBlockNodeSolution(CFEI* cfei, 
                             GlobalID elemBlockID,  
                             int numNodes, 
                             GlobalID *nodeIDs, 
                             int *offsets,  
                             double *estimates);

int FEI_putBlockFieldNodeSolution(CFEI* cfei, 
                                  GlobalID elemBlockID,  
                                  int fieldID, 
                                  int numNodes, 
                                  GlobalID *nodeIDs, 
                                  double *estimates);
         
int FEI_putBlockElemSolution(CFEI* cfei, 
                             GlobalID elemBlockID,  
                             int numElems, 
                             GlobalID *elemIDs, 
                             int dofPerElem,
                             double *estimates);
 
int FEI_putCRMultipliers(CFEI* cfei, 
                         int numMultCRs, 
                         int* CRIDs,
                         double *multEstimates);
 
int FEI_getBlockNodeIDList(CFEI* cfei, 
                           GlobalID elemBlockID,
                           int numNodes, 
                           GlobalID* nodeIDs);

int FEI_getBlockElemIDList(CFEI* cfei, 
                           GlobalID elemBlockID,
                           int numElems,
                           GlobalID* elemIDs);

int FEI_getNumSolnParams(CFEI* cfei,
                         GlobalID nodeID,
                         int* numSolnParams);

int FEI_getNumElemBlocks(CFEI* cfei, int* numElemBlocks);

int FEI_getNumBlockActNodes(CFEI* cfei,
                            GlobalID blockID,
                            int* numNodes);

int FEI_getNumBlockActEqns(CFEI* cfei,
                           GlobalID blockID,
                           int* numEqns);

int FEI_getNumNodesPerElement(CFEI* cfei,
                              GlobalID blockID,
                              int* nodesPerElem);

int FEI_getNumEqnsPerElement(CFEI* cfei,
                             GlobalID blockID,
                             int* numEqns);
 
int FEI_getNumBlockElements(CFEI* cfei,
                            GlobalID blockID,
                            int* numElems);

int FEI_getNumBlockElemDOF(CFEI* cfei,
                           GlobalID blockID,
                           int* DOFPerElem);

int FEI_initSubstructure( CFEI* cfei, int substructureID,
                                 int numIDs, int* IDTypes,
                                 GlobalID* IDs);

int FEI_getSubstructureSize( CFEI* cfei, int substructureID,
                                        int* numIDs );

int FEI_getSubstructureIDList(CFEI* cfei, int substructureID,
			      int numIDs, int* IDTypes,
                                         GlobalID* IDs );

int FEI_getSubstructureFieldSolution(CFEI* cfei, int substructureID,
                                         int fieldID,
                                         int numIDs, int* IDTypes, 
                                         GlobalID *IDs, 
                                         double *results);

int FEI_putSubstructureFieldSolution(CFEI* cfei, int substructureID, 
                                         int fieldID, 
                                         int numIDs, int* IDTypes, 
                                         GlobalID *IDs, 
                                         double *estimates);

int FEI_putSubstructureFieldData(CFEI* cfei, int substructureID, 
                                        int fieldID, 
                                        int numIDs, int* IDTypes,
                                        GlobalID *IDs, 
                                        double *data);

#ifdef __cplusplus
}
#endif

#endif
