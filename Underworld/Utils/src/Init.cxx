/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2005, Monash Cluster Computing 
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 		* Redistributions of source code must retain the above copyright notice, 
** 			this list of conditions and the following disclaimer.
** 		* Redistributions in binary form must reproduce the above copyright 
**			notice, this list of conditions and the following disclaimer in the 
**			documentation and/or other materials provided with the distribution.
** 		* Neither the name of the Monash University nor the names of its contributors 
**			may be used to endorse or promote products derived from this software 
**			without specific prior written permission.
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
*%		Louis Moresi - Louis.Moresi@sci.monash.edu.au
*%
** Contributors:
*+		Robert Turnbull
*+		Vincent Lemiale
*+		Louis Moresi
*+		David May
*+		David Stegman
*+		Mirko Velic
*+		Patrick Sunter
*+		Julian Giordani
*+
** $Id: Init.c 765 2008-07-24 05:40:02Z LukeHodkinson $
** 
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include <PICellerator/PICellerator.h>
#include <PICellerator/Weights/Weights.h>
#include <PICellerator/Weights/WeightsCalculator.h>

#include <Underworld/Rheology/Rheology.h>

#include "Utils.h"

Stg_ObjectList* repRequiredRawFields_Reg = NULL;

#include <stdio.h>

Bool Underworld_Utils_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

   /* A register to keep track of fields needed to be initialise before the REP_Algorithm */
   repRequiredRawFields_Reg = Stg_ObjectList_New();

	Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */
	
	Stg_ComponentRegister_Add( componentRegister, UnderworldContext_Type, (Name)"0", _UnderworldContext_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, TracerOutput_Type, (Name)"0", _TracerOutput_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, Underworld_SwarmOutput_Type, (Name)"0", _Underworld_SwarmOutput_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, RadiogenicHeatingTerm_Type, (Name)"0", _RadiogenicHeatingTerm_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, StressField_Type , (Name)"0", _StressField_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, NodalPressureField_Type , (Name)"0", _NodalPressureField_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, SmoothVelGradField_Type , (Name)"0", _SmoothVelGradField_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, ViscosityField_Type , (Name)"0", _ViscosityField_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, DensityField_Type , (Name)"0", _DensityField_DefaultNew  );
	Stg_ComponentRegister_Add( componentRegister, DivergenceForce_Type,
                                   (Name)"0", _DivergenceForce_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, MixedStabiliserTerm_Type,
                                   (Name)"0", _MixedStabiliserTerm_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, StressBC_Type,
                                   (Name)"0", _StressBC_DefaultNew );

	RegisterParent( UnderworldContext_Type,       	    PICelleratorContext_Type );
	RegisterParent( TracerOutput_Type,     SwarmOutput_Type );
	RegisterParent( Underworld_SwarmOutput_Type,        SwarmOutput_Type );
	RegisterParent( RadiogenicHeatingTerm_Type,         ForceTerm_Type );
	RegisterParent( StressField_Type,                   ParticleFeVariable_Type );
	RegisterParent( NodalPressureField_Type,            ParticleFeVariable_Type );
	RegisterParent( SmoothVelGradField_Type,            ParticleFeVariable_Type );
	RegisterParent( ViscosityField_Type,                ParticleFeVariable_Type );
	RegisterParent( DensityField_Type,                  ParticleFeVariable_Type );
	RegisterParent( DivergenceForce_Type,                  ForceTerm_Type );
	RegisterParent( MixedStabiliserTerm_Type, StiffnessMatrixTerm_Type );
	RegisterParent( StressBC_Type, ForceTerm_Type );

   /* Register the new BaseClass for recovery fields */
   Stg_ComponentRegister_Add( componentRegister, BaseRecoveryFeVar_Type, (Name)"0", _BaseRecoveryFeVar_DefaultNew  );
   RegisterParent( BaseRecoveryFeVar_Type, FeVariable_Type );

   /* Register the SPR component */
   Stg_ComponentRegister_Add( componentRegister, SPR_StrainRate_Type, (Name)"0", _SPR_StrainRate_DefaultNew  );
   RegisterParent( SPR_StrainRate_Type, BaseRecoveryFeVar_Type );

   Stg_ComponentRegister_Add( componentRegister, REP_Algorithm_Type, (Name)"0", _REP_Algorithm_DefaultNew  );
   RegisterParent( REP_Algorithm_Type,           Stg_Component_Type );

   Stg_ComponentRegister_Add( componentRegister, RecoveredFeVariable_Type, (Name)"0", _RecoveredFeVariable_DefaultNew  );
   RegisterParent( RecoveredFeVariable_Type,     FeVariable_Type );

	return True;
}


