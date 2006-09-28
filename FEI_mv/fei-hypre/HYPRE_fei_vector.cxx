/*BHEADER**********************************************************************
 * Copyright (c) 2006   The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by the HYPRE team <hypre-users@llnl.gov>, UCRL-CODE-222953.
 * All rights reserved.
 *
 * This file is part of HYPRE (see http://www.llnl.gov/CASC/hypre/).
 * Please see the COPYRIGHT_and_LICENSE file for the copyright notice, 
 * disclaimer and the GNU Lesser General Public License.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * $Revision$
 ***********************************************************************EHEADER*/

/******************************************************************************
 *
 * HYPRE_fei_vector functions
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef HAVE_FEI
#include "FEI_Implementation.h"
#endif
#include "LLNL_FEI_Impl.h"
#include "fei_mv.h"
#include "Data.h"
#include "IJ_mv/HYPRE_IJ_mv.h"
#include "parcsr_mv/HYPRE_parcsr_mv.h"

/*****************************************************************************/
/* HYPRE_FEVectorCreate function                                             */
/*---------------------------------------------------------------------------*/

extern "C" int
HYPRE_FEVectorCreate(MPI_Comm comm, HYPRE_FEMesh mesh, HYPRE_FEVector *vector)
{
   HYPRE_FEVector myVector;
   myVector = (HYPRE_FEVector) malloc(sizeof(HYPRE_FEVector));
   myVector->mesh_ = mesh;
   myVector->comm_ = comm;
   (*vector) = myVector;
   return 0;
}

/*****************************************************************************/
/* HYPRE_FEVectorDestroy - Destroy a FEVector object.                        */
/*---------------------------------------------------------------------------*/

extern "C" int
HYPRE_FEVectorDestroy(HYPRE_FEVector vector)
{
   if (vector)
   {
      free(vector);
   }
   return 0;
}

/*****************************************************************************/
/* HYPRE_FEVectorGetRHS                                                      */
/*---------------------------------------------------------------------------*/

extern "C" int
HYPRE_FEVectorGetRHS(HYPRE_FEVector vector, void **object)
{
   int               ierr=0;
   HYPRE_FEMesh      mesh;
   LinearSystemCore* lsc;
   Data              dataObj;
   HYPRE_IJVector    X;
   HYPRE_ParVector   XCSR;

   if (vector == NULL)
      ierr = 1;
   else
   {
      mesh = vector->mesh_;
      if (mesh == NULL)
         ierr = 1;
      else
      {
         lsc = (LinearSystemCore *) mesh->linSys_;
         if (lsc != NULL)
         {
            lsc->copyOutRHSVector(1.0e0, dataObj); 
            X = (HYPRE_IJVector) dataObj.getDataPtr();
            HYPRE_IJVectorGetObject(X, (void **) &XCSR);
            (*object) = (void *) XCSR;
         }
         else
         {
            (*object) = NULL;
            ierr = 1;
         }
      }
   }
   return ierr;
}

/*****************************************************************************/
/* HYPRE_FEVectorSetSol                                                      */
/*---------------------------------------------------------------------------*/

extern "C" int
HYPRE_FEVectorSetSol(HYPRE_FEVector vector, void *object)
{
   int                ierr=0;
   HYPRE_FEMesh       mesh;
   LinearSystemCore   *lsc;
   Data               dataObj;
   LLNL_FEI_Impl      *fei1;
#ifdef HAVE_FEI
   FEI_Implementation *fei2;
#endif

   if (vector == NULL)
      ierr = 1;
   else
   {
      mesh = vector->mesh_;
      if (mesh == NULL)
         ierr = 1;
      else
      {
         lsc = (LinearSystemCore *) mesh->linSys_;
         if (lsc != NULL)
         {
            dataObj.setTypeName("Sol_Vector");
            dataObj.setDataPtr((void*) object);
            lsc->copyInRHSVector(1.0e0, dataObj); 
            if (mesh->feiPtr_ != NULL)
            {
#ifdef HAVE_FEI
               if (mesh->objectType_ == 1)
               {
                  fei1 = (LLNL_FEI_Impl *) mesh->feiPtr_;
                  ierr = fei1->solve(&ierr);
               }
               if (mesh->objectType_ == 2)
               {
                  fei2 = (FEI_Implementation *) mesh->feiPtr_;
                  ierr = fei2->solve(&ierr);
               }
#else
               fei1 = (LLNL_FEI_Impl *) mesh->feiPtr_;
               ierr = fei1->solve(&ierr);
#endif
            }
         }
         else
         {
            ierr = 1;
         }
      }
   }
   return ierr;
}

