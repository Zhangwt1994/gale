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
** $Id: solHA.c 636 2006-09-08 03:07:06Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include "Analytic_solHA.h"

const Type Velic_solHA_Type = "Underworld_Velic_solHA";

void Velic_solHA_PressureFunction( void* analyticSolution, const double* coord, double* pressure ) {
	Velic_solHA* self = (Velic_solHA*) analyticSolution;
	
	_Velic_solHA( coord, self->sigma, self->eta, self->dx, self->dy, self->x0, self->y0, NULL, pressure, NULL, NULL );
}

void Velic_solHA_VelocityFunction( void* analyticSolution, const double* coord, double* velocity ) {
	Velic_solHA* self = (Velic_solHA*) analyticSolution;
	
	_Velic_solHA( coord, self->sigma, self->eta, self->dx, self->dy, self->x0, self->y0, velocity, NULL, NULL, NULL );
}

void Velic_solHA_StressFunction( void* analyticSolution, const double* coord, double* stress ) {
	Velic_solHA* self = (Velic_solHA*) analyticSolution;
	
	_Velic_solHA( coord, self->sigma, self->eta, self->dx, self->dy, self->x0, self->y0, NULL, NULL, stress, NULL );
}


void Velic_solHA_StrainRateFunction( void* analyticSolution, const double* coord, double* strainRate ) {
	Velic_solHA* self = (Velic_solHA*) analyticSolution;
	
	_Velic_solHA( coord, self->sigma, self->eta, self->dx, self->dy, self->x0, self->y0, NULL, NULL, NULL, strainRate );
}

void _Velic_solHA_Init( Velic_solHA* self, double sigma, double eta, double dx, double dy, double x0, double y0 ) {
	self->sigma = sigma;
	self->eta = eta;
	self->dx = dx;
	self->dy = dy;
	self->x0 = x0;
	self->y0 = y0;
}

void _Velic_solHA_Build( void* analyticSolution, void* data ) {
	Velic_solHA*          self  = (Velic_solHA*)analyticSolution;
	
	_FieldTest_Build( self, data );

	/* here we assign the memory and the func ptr for analytic sols */
	self->_analyticSolutionList = Memory_Alloc_Array_Unnamed( FieldTest_AnalyticSolutionFunc*, 4 );
	/* this order MUST be consistent with the xml file definition */
	self->_analyticSolutionList[0] = Velic_solHA_VelocityFunction;
	self->_analyticSolutionList[1] = Velic_solHA_PressureFunction;
	self->_analyticSolutionList[2] = Velic_solHA_StrainRateFunction;
	self->_analyticSolutionList[3] = Velic_solHA_StressFunction;
}

void _Velic_solHA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data ) {
	Velic_solHA* self = (Velic_solHA*) analyticSolution;
	double                   sigma, eta, dx, dy, x0, y0;
	double                   startX, endX, startY, endY;

	/* Construct Parent */
	_FieldTest_AssignFromXML( self, cf, data );

	sigma = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_sigma", 1.0  );
	eta = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_eta", 1.0  );

	startX = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_startX", 0.1  );
	endX   = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_endX", 0.7  );

	startY = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_startY", 0.2  );
	endY   = Stg_ComponentFactory_GetRootDictDouble( cf, (Dictionary_Entry_Key)"solHA_endY", 0.6 );

	dx = 0.5 * (startX + endX);
	dy = 0.5 * (startY + endY );
	x0 = endX - startX;
	y0 = endY - startY;

	_Velic_solHA_Init( self, sigma, eta, dx, dy, x0, y0 );

}

void* _Velic_solHA_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Velic_solHA);
	Type                                                      type = Velic_solHA_Type;
	Stg_Class_DeleteFunction*                              _delete = _FieldTest_Delete;
	Stg_Class_PrintFunction*                                _print = _FieldTest_Print;
	Stg_Class_CopyFunction*                                  _copy = _FieldTest_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _Velic_solHA_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Velic_solHA_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Velic_solHA_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FieldTest_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FieldTest_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FieldTest_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _FieldTest_New(  FIELDTEST_PASSARGS  );
}

Index Underworld_Velic_solHA_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, Velic_solHA_Type, (Name)"0", _Velic_solHA_DefaultNew  );
}


