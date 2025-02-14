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
** $Id: solKz.c 636 2006-09-08 03:07:06Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include "Analytic_solKz.h"

const Type Velic_solKz_Type = "Underworld_Velic_solKz";

void Velic_solKz_PressureFunction( void* analyticSolution, const double *coord, double* pressure ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	
	_Velic_solKz( coord, self->sigma, self->km, self->n, self->B, NULL, pressure, NULL, NULL );
}

void Velic_solKz_VelocityFunction( void* analyticSolution, const double *coord, double* velocity ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	
	_Velic_solKz( coord, self->sigma, self->km, self->n, self->B, velocity, NULL, NULL, NULL );
}

void Velic_solKz_StressFunction( void* analyticSolution, const double *coord, double* stress ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	
	_Velic_solKz( coord, self->sigma, self->km, self->n, self->B, NULL, NULL, stress, NULL );
}


void Velic_solKz_StrainRateFunction( void* analyticSolution, const double *coord, double* strainRate ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	
	_Velic_solKz( coord, self->sigma, self->km, self->n, self->B, NULL, NULL, NULL, strainRate );
}

void Velic_solKz_ViscosityFunction( void* analyticSolution, const double *coord, double* viscosity ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	
	*viscosity = exp( 2.0 * self->B * coord[ J_AXIS ] );
}

	
void _Velic_solKz_Init( Velic_solKz* self, double sigma, double km, double B, int n ) {
	self->sigma = sigma;
	self->km = km;
	self->B = B;
	self->n = n;
}

void _Velic_solKz_Build( void* analyticSolution, void* data ) {
	Velic_solKz*          self  = (Velic_solKz*)analyticSolution;

	_FieldTest_Build( self, data );

	/* here we assign the memory and the func ptr for analytic sols */
	self->_analyticSolutionList = Memory_Alloc_Array_Unnamed( FieldTest_AnalyticSolutionFunc*, 4 );
	/* this order MUST be consistent with the xml file definition */
	self->_analyticSolutionList[0] = Velic_solKz_VelocityFunction;
	self->_analyticSolutionList[1] = Velic_solKz_PressureFunction;
	self->_analyticSolutionList[2] = Velic_solKz_StrainRateFunction;
	self->_analyticSolutionList[3] = Velic_solKz_StressFunction;
}

void _Velic_solKz_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data ) {
	Velic_solKz* self = (Velic_solKz*) analyticSolution;
	double                   sigma, m, B, twiceB, km;
	int                      n;

	/* Construct Parent */
	_FieldTest_AssignFromXML( self, cf, data );

	sigma = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKz_sigma", 1.0  );
	twiceB = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKz_twiceB", 2.0  );
	B = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKz_B", 0.5 * twiceB  );
	m = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solKz_m", 1  );
	n = Stg_ComponentFactory_GetRootDictInt( cf, "solKz_n", 1 );

   km = M_PI * m;
	
  /* Note: in the _Velic_solKz function km and n must be supplied */
	_Velic_solKz_Init( self, sigma, km, B, n );
}

void* _Velic_solKz_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Velic_solKz);
	Type                                                      type = Velic_solKz_Type;
	Stg_Class_DeleteFunction*                              _delete = _FieldTest_Delete;
	Stg_Class_PrintFunction*                                _print = _FieldTest_Print;
	Stg_Class_CopyFunction*                                  _copy = _FieldTest_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _Velic_solKz_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Velic_solKz_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Velic_solKz_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FieldTest_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FieldTest_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FieldTest_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _FieldTest_New(  FIELDTEST_PASSARGS  );
}

Index Underworld_Velic_solKz_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, Velic_solKz_Type, (Name)"0", _Velic_solKz_DefaultNew  );
}


