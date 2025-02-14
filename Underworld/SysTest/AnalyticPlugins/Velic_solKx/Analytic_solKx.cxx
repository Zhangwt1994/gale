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
*+		Mirko Velic
*+		Julian Giordani
*+		Robert Turnbull
*+		Vincent Lemiale
*+		Louis Moresi
*+		David May
*+		David Stegman
*+		Patrick Sunter
** $Id: solKx.c 636 2006-09-08 03:07:06Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include "Analytic_solKx.h"

const Type Velic_solKx_Type = "Underworld_Velic_solKx";

void Velic_solKx_ViscosityFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* viscosity ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	
	_Velic_solKx( coord, self->sigma, self->_m, self->n, self->B, NULL, NULL, NULL, NULL, viscosity );
}

void Velic_solKx_PressureFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* pressure ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	
	_Velic_solKx( coord, self->sigma, self->_m, self->n, self->B, NULL, pressure, NULL, NULL, NULL );
}

void Velic_solKx_VelocityFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* velocity ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	
	_Velic_solKx( coord, self->sigma, self->_m, self->n, self->B, velocity, NULL, NULL, NULL, NULL );
}

void Velic_solKx_StressFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* stress ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	
	_Velic_solKx( coord, self->sigma, self->_m, self->n, self->B, NULL, NULL, stress, NULL, NULL );
}


void Velic_solKx_StrainRateFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* strainRate ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	
	_Velic_solKx( coord, self->sigma, self->_m, self->n, self->B, NULL, NULL, NULL, strainRate, NULL );
}
	
void _Velic_solKx_Init( Velic_solKx* self, double sigma, double _m, double B, int n ) {
	// TODO	Bool                     correctInput = True;
	// Set the general AnalyticSolution (parent class) field function pointers to the specific analytic sols to use
        self->_getAnalyticVelocity    = Velic_solKx_VelocityFunction;
	self->_getAnalyticPressure    = Velic_solKx_PressureFunction;
	self->_getAnalyticTotalStress = Velic_solKx_StressFunction;
	self->_getAnalyticStrainRate  = Velic_solKx_StrainRateFunction;

	self->sigma = sigma;
	self->_m = _m;
	self->B = B;
	self->n = n;
}

void _Velic_solKx_Build( void* analyticSolution, void* data ) {
	Velic_solKx*          self  = (Velic_solKx*)analyticSolution;

	_AnalyticSolution_Build( self, data );
}

void _Velic_solKx_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data ) {
	Velic_solKx* self = (Velic_solKx*) analyticSolution;
	FeVariable*              velocityField;
	FeVariable*              pressureField;
	FeVariable*              stressField;
	FeVariable*              strainRateField;
	FeVariable*              recoveredStrainRateField;
	FeVariable*              viscosityField;
	FeVariable*              recoveredStressField;
	FeVariable*              recoveredPressureField;
	double                   sigma, _m, B, twiceB;
	int                      n;

	/* Construct Parent */
	_AnalyticSolution_AssignFromXML( self, cf, data );

	/* Create Analytic Fields */
	velocityField = Stg_ComponentFactory_ConstructByName( cf, (Name)"VelocityField", FeVariable, True, data  );
	AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, velocityField, Velic_solKx_VelocityFunction );

	pressureField = Stg_ComponentFactory_ConstructByName( cf, (Name)"PressureField", FeVariable, True, data  );
	AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, pressureField, Velic_solKx_PressureFunction );

	viscosityField = Stg_ComponentFactory_ConstructByName( cf, (Name)"ViscosityField", FeVariable, False, data );
	if ( viscosityField  ) 
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, viscosityField, Velic_solKx_ViscosityFunction );

	stressField = Stg_ComponentFactory_ConstructByName( cf, (Name)"StressField", FeVariable, False, data );
	if ( stressField  ) 
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, stressField, Velic_solKx_StressFunction );

	strainRateField = Stg_ComponentFactory_ConstructByName( cf, (Name)"StrainRateField", FeVariable, False, data );
	if ( strainRateField   )
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, strainRateField, Velic_solKx_StrainRateFunction );

	recoveredStrainRateField = Stg_ComponentFactory_ConstructByName( cf, (Name)"recoveredStrainRateField", FeVariable, False, data );
	if ( recoveredStrainRateField  )
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, recoveredStrainRateField, Velic_solKx_StrainRateFunction );

	recoveredStressField = Stg_ComponentFactory_ConstructByName( cf, (Name)"recoveredStressField", FeVariable, False, data );
	if ( recoveredStressField  )
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, recoveredStressField, Velic_solKx_StressFunction );

	recoveredPressureField = Stg_ComponentFactory_ConstructByName( cf, (Name)"recoveredPressureField", FeVariable, False, data );
	if ( recoveredPressureField  )
		AnalyticSolution_RegisterFeVariableWithAnalyticFunction( self, recoveredPressureField, Velic_solKx_PressureFunction );


	sigma = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKx_sigma", 1.0  );
	_m = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"wavenumberY", 1  );
	n = Stg_ComponentFactory_GetRootDictInt( cf, "wavenumberX", 1 );
	twiceB = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKx_twiceB", 2.0  );
	B = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKx_B", 0.5 * twiceB  );

	_Velic_solKx_Init( self, sigma, _m, B, n );

}

void* _Velic_solKx_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Velic_solKx);
	Type                                                      type = Velic_solKx_Type;
	Stg_Class_DeleteFunction*                              _delete = _AnalyticSolution_Delete;
	Stg_Class_PrintFunction*                                _print = _AnalyticSolution_Print;
	Stg_Class_CopyFunction*                                  _copy = _AnalyticSolution_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _Velic_solKx_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Velic_solKx_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Velic_solKx_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _AnalyticSolution_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _AnalyticSolution_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _AnalyticSolution_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _AnalyticSolution_New(  ANALYTICSOLUTION_PASSARGS  );
}

Index Underworld_Velic_solKx_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, Velic_solKx_Type, (Name)"0", _Velic_solKx_DefaultNew  );
}


