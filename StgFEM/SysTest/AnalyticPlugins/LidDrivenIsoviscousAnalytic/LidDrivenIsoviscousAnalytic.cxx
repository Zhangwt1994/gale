/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003-2006, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street,
**	Melbourne, 3053, Australia.
**
** Primary Contributing Organisations:
**	Victorian Partnership for Advanced Computing Ltd, Computational Software Development - http://csd.vpac.org
**	Australian Computational Earth Systems Simulator - http://www.access.edu.au
**	Monash Cluster Computing - http://www.mcc.monash.edu.au
**	Computational Infrastructure for Geodynamics - http://www.geodynamics.org
**
** Contributors:
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Robert Turnbull, Research Assistant, Monash University. (robert.turnbull@sci.monash.edu.au)
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	David May, PhD Student, Monash University (david.may@sci.monash.edu.au)
**	Louis Moresi, Associate Professor, Monash University. (louis.moresi@sci.monash.edu.au)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**	Julian Giordani, Research Assistant, Monash University. (julian.giordani@sci.monash.edu.au)
**	Vincent Lemiale, Postdoctoral Fellow, Monash University. (vincent.lemiale@sci.monash.edu.au)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: LidDrivenIsoviscousAnalytic.c 964 2007-10-11 08:03:06Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

/* This is taken from Mirko Velic's Analytic Stokes Flow solution */

const Type LidDrivenIsoviscousAnalytic_Type = "LidDrivenIsoviscousAnalytic";

typedef struct { 
	__FieldTest 
	unsigned int wavenumber;
	double A, B, C, D;
	FeVariable* velocityField;
	FeVariable* pressureField;
} LidDrivenIsoviscousAnalytic;


void LidDrivenIsoviscousAnalytic_CalculateConstants( LidDrivenIsoviscousAnalytic *self ) {
	double                  E;
	double                  e_nPI;
	double                  e_2nPI;
	double                  e_4nPI;
	double                  n;

	n = (double) self->wavenumber;

	e_nPI = exp( n * M_PI );
	e_2nPI = e_nPI * e_nPI;
	e_4nPI = e_2nPI * e_2nPI;

	E = (4.0 * n * n * M_PI * M_PI + 2.0 ) * e_2nPI - e_4nPI - 1.0;

	self->A = ( e_2nPI - 1.0 )* e_nPI / E;
	self->B = - self->A;

	self->C =   ( 2.0 * n * M_PI - e_2nPI + 1.0 ) * e_nPI / E;
	self->D = - ( 2.0 * n * M_PI * e_2nPI - e_2nPI + 1.0 ) * e_nPI / E;
}

void LidDrivenIsoviscousAnalytic_VelocityFunction( void* analyticSolution, const double* coord, double* velocity ) {
	LidDrivenIsoviscousAnalytic *self = (LidDrivenIsoviscousAnalytic*)analyticSolution;
	double x,y;
	double n;
	double A, B, C, D;

	/* Get local copy of constants */
	n = (double) self->wavenumber;
	A = self->A;
	B = self->B;
	C = self->C;
	D = self->D;

	/* get copy of coords */
	x = coord[I_AXIS];
	y = coord[J_AXIS];
	
	velocity[ I_AXIS ] = sin( n * M_PI * x ) * 
		( ( A * n * M_PI + C + C * n * M_PI * y) *exp( n * M_PI * y ) 
		- ( B * n * M_PI - D + D * n * M_PI * y ) * exp( - n * M_PI * y ) );
	velocity[ J_AXIS ] = - n * M_PI * cos( n * M_PI * x ) * 
		( ( A + C * y ) * exp( n * M_PI * y ) 
		+ ( B + D * y ) * exp( - n * M_PI * y ) );
}


void LidDrivenIsoviscousAnalytic_PressureFunction( void* analyticSolution, const double* coord, double* pressure ) {
	LidDrivenIsoviscousAnalytic *self = (LidDrivenIsoviscousAnalytic*)analyticSolution;
	double x,y;
	double n;
	double C, D;

	/* Get local copy of constants */
	n = (double) self->wavenumber;
	C = self->C;
	D = self->D;

	/* get copy of coords */
	x = coord[I_AXIS];
	y = coord[J_AXIS];
	
	*pressure = - 2.0 * n * M_PI * cos( n * M_PI * x ) * ( C * exp( n * M_PI * y ) + D * exp( - n * M_PI * y ) );
}

void _LidDrivenIsoviscousAnalytic_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data ) {
	LidDrivenIsoviscousAnalytic *self = (LidDrivenIsoviscousAnalytic*)analyticSolution;

	_FieldTest_AssignFromXML( self, cf, data );

	/* Set constants */
	self->wavenumber = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"sinusoidalLidWavenumber", 1 );
	LidDrivenIsoviscousAnalytic_CalculateConstants( self  );
}

void _LidDrivenIsoviscousAnalytic_Build( void* analyticSolution, void* data ) {
	LidDrivenIsoviscousAnalytic *self = (LidDrivenIsoviscousAnalytic*)analyticSolution;

	_FieldTest_Build( self, data );

	/* here we assign the memory and the func ptr for analytic sols */
	self->_analyticSolutionList = Memory_Alloc_Array_Unnamed( FieldTest_AnalyticSolutionFunc*, 2 );
	/* this order MUST be consistent with the xml file definition */
	self->_analyticSolutionList[0] = LidDrivenIsoviscousAnalytic_VelocityFunction;
	self->_analyticSolutionList[1] = LidDrivenIsoviscousAnalytic_PressureFunction;
}

void* _LidDrivenIsoviscousAnalytic_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(LidDrivenIsoviscousAnalytic);
	Type                                                      type = LidDrivenIsoviscousAnalytic_Type;
	Stg_Class_DeleteFunction*                              _delete = _FieldTest_Delete;
	Stg_Class_PrintFunction*                                _print = _FieldTest_Print;
	Stg_Class_CopyFunction*                                  _copy = _FieldTest_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _LidDrivenIsoviscousAnalytic_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _LidDrivenIsoviscousAnalytic_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _LidDrivenIsoviscousAnalytic_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FieldTest_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FieldTest_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FieldTest_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _FieldTest_New(  FIELDTEST_PASSARGS  );
}

/* This function is automatically run by StGermain when this plugin is loaded. The name must be "<plugin-name>_Register". */
Index StgFEM_LidDrivenIsoviscousAnalytic_Register( PluginsManager* pluginsManager ) {
	/* A plugin is only properly registered once it returns the handle provided when submitting a codelet to StGermain. */
	return PluginsManager_Submit( pluginsManager, LidDrivenIsoviscousAnalytic_Type, (Name)"0", _LidDrivenIsoviscousAnalytic_DefaultNew  );
}


