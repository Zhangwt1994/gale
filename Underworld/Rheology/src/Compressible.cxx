/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2005, Monash Cluster Computing
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
**       * Redistributions of source code must retain the above copyright notice,
**          this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**       * Neither the name of the Monash University nor the names of its contributors
**       may be used to endorse or promote products derived from this software
**       without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%    Louis Moresi - Louis.Moresi@sci.monash.edu.au
*%
** Contributors:
*+    Robert Turnbull
*+    Vincent Lemiale
*+    Louis Moresi
*+    David May
*+    David Stegman
*+    Mirko Velic
*+    Patrick Sunter
*+    Julian Giordani
*+
** $Id: Compressible.c 610 2007-10-11 08:09:29Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include "types.h"
#include "Compressible.h"
#include "RheologyMaterial.h"

#include <assert.h>

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type Compressible_Type = "Compressible";

/* Public Constructor */

Compressible* Compressible_New(
      Name                   name,
      FiniteElementContext*  context,
		StiffnessMatrix*       stiffnessMatrix,
		Swarm*                 swarm,
		Stg_Component*         extraInfo,
      FeMesh*                geometryMesh,
      Materials_Register*    materials_Register,
      double                 oneOnLambda )
{
   Compressible* self = (Compressible*) _Compressible_DefaultNew( name );
   _StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, swarm, extraInfo );
   _Compressible_Init( self, geometryMesh, materials_Register, oneOnLambda );
   self->isConstructed = True;
   
   return self;
}

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
Compressible* _Compressible_New(  COMPRESSIBLE_DEFARGS  )
{
   Compressible*              self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(Compressible) );
   self = (Compressible*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

   return self;
}

void _Compressible_Init(
      Compressible*        self,
      FeMesh*              geometryMesh,
      Materials_Register*  materials_Register,
      double               oneOnLambda )
{

   self->oneOnLambda            = oneOnLambda;
   self->geometryMesh           = geometryMesh;
   self->materials_Register     = materials_Register;
}


void _Compressible_Destroy( void* compressible, void* data ) {
   Compressible*    self = (Compressible*)compressible;

   Stg_Component_Destroy( self->geometryMesh, data, False );
   _StiffnessMatrixTerm_Destroy( self, data );
}

void _Compressible_Print( void* compressible, Stream* stream ) {
   Compressible*    self = (Compressible*)compressible;

   _StiffnessMatrixTerm_Print( self, stream );

   Journal_PrintValue( stream, self->oneOnLambda );
}

void* _Compressible_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(Compressible);
	Type                                                         type = Compressible_Type;
	Stg_Class_DeleteFunction*                                 _delete = _StiffnessMatrixTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _Compressible_Print;
	Stg_Class_CopyFunction*                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _Compressible_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _Compressible_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _Compressible_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _Compressible_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _Compressible_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _Compressible_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _Compressible_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _Compressible_New(  COMPRESSIBLE_PASSARGS  );
}


void _Compressible_AssignFromXML( void* compressible, Stg_ComponentFactory* cf, void* data ){
   Compressible*     self = (Compressible*)compressible;
   FeMesh*        geometryMesh;
   Materials_Register*  materials_Register;
   PICelleratorContext* context;

   _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

   geometryMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"GeometryMesh", FeMesh, True, data );

   context = (PICelleratorContext* )self->context;
   assert( Stg_CheckType( context, PICelleratorContext ) );
   materials_Register = context->materials_Register;
   assert( materials_Register );

   _Compressible_Init(
         self,
         geometryMesh,
         materials_Register,
         Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"oneOnLambda", 10.0 )  );

   /* Make sure that we are using the correct type of swarm
    * SHOULDN'T THIS BEEN DONE AT THE STIFFNESSMATRIXTERM LEVEL? */
   Stg_CheckType( self->integrationSwarm, IntegrationPointsSwarm );
}

void _Compressible_Build( void* compressible, void* data ){
   _StiffnessMatrixTerm_Build( compressible, data );
}
void _Compressible_Initialise( void* compressible, void* data ){
   _StiffnessMatrixTerm_Initialise( compressible, data );
}
void _Compressible_Execute( void* compressible, void* data ){
   _StiffnessMatrixTerm_Execute( compressible, data );
}

void _Compressible_AssembleElement(void* compressible,
                                   StiffnessMatrix* stiffnessMatrix,
                                   Element_LocalIndex lElement_I,
                                   SystemLinearEquations* sle,
                                   FiniteElementContext* context,
                                   double** elStiffMat)
{
  Compressible* self=(Compressible*)compressible;
  IntegrationPointsSwarm* swarm=(IntegrationPointsSwarm*)self->integrationSwarm;
  RheologyMaterial*         material;
  FeVariable*               variable1=stiffnessMatrix->rowVariable;
  Dimension_Index           dim=stiffnessMatrix->dim;
  IntegrationPoint*         particle;
  Particle_InCellIndex      cParticle_I;
  Particle_InCellIndex      cellParticleCount;
  Element_NodeIndex         elementNodeCount;
  Index                     row_I;
  Index                     col_I;
  double                    detJac;
  Cell_Index                cell_I;
  ElementType*              elementType;
  Dof_Index                 dofCount;
  FeMesh* mesh=variable1->feMesh;
  double                    Ni[27];
  double*                   xi;
  double                    factor;
  FeMesh*  		  geometryMesh        = self->geometryMesh;
  ElementType*              geometryElementType;
  double oneOnLambda(0.0);

  /* Set the element type */
  elementType         = FeMesh_GetElementType( mesh, lElement_I );
  geometryElementType = FeMesh_GetElementType( geometryMesh, lElement_I );
  elementNodeCount    = elementType->nodeCount;
  dofCount            = elementNodeCount;

  /* Get number of particles per element */
  cell_I=CellLayout_MapElementIdToCellId(swarm->cellLayout,lElement_I);
  cellParticleCount=swarm->cellParticleCountTbl[cell_I];

  /* Use an average compressibility for OneToMany mapper */
  bool one_to_many=Stg_Class_IsInstance(swarm->mapper,OneToManyMapper_Type);
  if(one_to_many)
    {
      OneToManyMapper *mapper=(OneToManyMapper*)(swarm->mapper);
      IntegrationPointsSwarm* OneToMany_swarm=mapper->swarm;
      int OneToMany_cell=
        CellLayout_MapElementIdToCellId(OneToMany_swarm->cellLayout,
                                        lElement_I);
      int num_particles=OneToMany_swarm->cellParticleCountTbl[OneToMany_cell];

      double weight(0);
      for(int ii=0;ii<num_particles;++ii)
        {
          IntegrationPoint *OneToMany_particle=
            (IntegrationPoint*)Swarm_ParticleInCellAt(OneToMany_swarm,
                                                      OneToMany_cell,ii);
          weight+=OneToMany_particle->weight;
          material = (RheologyMaterial*)
            IntegrationPointsSwarm_GetMaterialOn( swarm, OneToMany_particle );

          /* Only make contribution to the compressibility matrix if
             this material is compressible */
          if(!material->compressible) 
            continue;

          oneOnLambda = material->compressible->oneOnLambda;
        }
      if(oneOnLambda==0.0)
        return;
      oneOnLambda/=weight;
    }

  /* Loop over points to build Stiffness Matrix */
  for(cParticle_I=0; cParticle_I<cellParticleCount; cParticle_I++)
    {
      particle=(IntegrationPoint*)Swarm_ParticleInCellAt(swarm,cell_I,
                                                         cParticle_I);
      if(!one_to_many)
        {
          IntegrationPointsSwarm* NNswarm(swarm);
          IntegrationPoint* NNparticle(particle);
          NearestNeighbor_Replace(&NNswarm,&NNparticle,lElement_I,dim);

          material = (RheologyMaterial*)
            IntegrationPointsSwarm_GetMaterialOn( swarm, NNparticle );

          /* Only make contribution to the compressibility matrix if
             this material is compressible */
          if ( !material->compressible ) 
            continue;

          oneOnLambda = material->compressible->oneOnLambda;
        }

      /* Calculate Determinant of Jacobian and Shape Functions */
      xi = particle->xi;
      detJac=ElementType_JacobianDeterminant(geometryElementType,geometryMesh,
                                             lElement_I,xi,dim);
      ElementType_EvaluateShapeFunctionsAt( elementType, xi, Ni );
      factor=detJac*particle->weight*oneOnLambda;

      for(row_I=0; row_I<dofCount; row_I++)
        {
          for(col_I=0; col_I<dofCount; col_I++)
            {
              elStiffMat[row_I][col_I]-=factor*Ni[row_I]*Ni[col_I];
            }
        }
    }
}
