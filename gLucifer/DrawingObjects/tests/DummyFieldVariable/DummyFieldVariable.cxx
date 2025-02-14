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
*%		Cecile Duboz - Cecile.Duboz@sci.monash.edu.au
*%
** Contributors:
*+		Cecile Duboz
*+		Robert Turnbull
*+		Alan Lo
*+		Louis Moresi
*+		David Stegman
*+		David May
*+		Stevan Quenette
*+		Patrick Sunter
*+		Greg Watson
*+
** $Id: Arrhenius.c 78 2005-11-29 11:58:21Z RobertTurnbull $
** 
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifdef HAVE_PYTHON
#include <Python.h>
#endif

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <glucifer/Base/Base.h>
#include <glucifer/Windowing/Windowing.h>
#include <glucifer/RenderingEngines/RenderingEngines.h>
#include <glucifer/DrawingObjects/DrawingObjects.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const Type DummyFieldVariable_Type = "DummyFieldVariable";
			
InterpolationResult DummyFieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value ) {
	FieldVariable* self = (FieldVariable*) fieldVariable;
	Dof_Index      component_I;
	
	for ( component_I =0; component_I  < self->fieldComponentCount ; component_I++ ){
		value[ component_I ] = coord[0] * coord[0] + coord[1] *coord[1];
		if ( self->dim == 3 )
			value[ component_I ] += coord[2] *coord[2];
	}

	return 1;
}

double DummyFieldVariable_GetMinGlobalFieldMagnitude( void* fieldVariable ) {
	return  0.0;
}
double DummyFieldVariable_GetMaxGlobalFieldMagnitude( void* fieldVariable ) {
	FieldVariable* self = (FieldVariable*) fieldVariable;
	return  12.0 * sqrt( self->fieldComponentCount );
}
void DummyFieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) {
	FieldVariable* self = (FieldVariable*) fieldVariable; 
	int            rank; 
	int            numProcessors;
	double         dx;

	MPI_Comm_size( self->communicator, &numProcessors );
	MPI_Comm_rank( self->communicator, &rank );

	dx = 2.0/(double)numProcessors;

	min[0] = dx * (double) rank;
	min[1] = 0.0;
	min[2] = 0.0;
	
	max[0] = dx * (double) (rank + 1);
	max[1] = 2.0;
	max[2] = 2.0;
}
void DummyFieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) {
	min[0] = 0.0;
	min[1] = 0.0;
	min[2] = 0.0;
	
	max[0] = 2.0;
	max[1] = 2.0;
	max[2] = 2.0;
}

void _DummyFieldVariable_AssignFromXML( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) {
	FieldVariable* self = (FieldVariable*) fieldVariable;

	self->fieldComponentCount = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"fieldComponentCount", 1  );
	self->dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0 );
	self->communicator = MPI_COMM_WORLD;
}

void* _DummyFieldVariable_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                      _sizeOfSelf = sizeof(FieldVariable);
	Type                                                              type = DummyFieldVariable_Type;
	Stg_Class_DeleteFunction*                                      _delete = _FieldVariable_Delete;
	Stg_Class_PrintFunction*                                        _print = _FieldVariable_Print;
	Stg_Class_CopyFunction*                                          _copy = _FieldVariable_Copy;
	Stg_Component_DefaultConstructorFunction*          _defaultConstructor = _DummyFieldVariable_DefaultNew;
	Stg_Component_ConstructFunction*                            _construct = _DummyFieldVariable_AssignFromXML;
	Stg_Component_BuildFunction*                                    _build = _FieldVariable_Build;
	Stg_Component_InitialiseFunction*                          _initialise = _FieldVariable_Initialise;
	Stg_Component_ExecuteFunction*                                _execute = _FieldVariable_Execute;
	Stg_Component_DestroyFunction*                                _destroy = _FieldVariable_Destroy;
	AllocationType                                      nameAllocationType = False;
	FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt = DummyFieldVariable_InterpolateValueAt;
	FieldVariable_GetValueFunction*            _getMinGlobalFieldMagnitude = DummyFieldVariable_GetMinGlobalFieldMagnitude;
	FieldVariable_GetValueFunction*            _getMaxGlobalFieldMagnitude = DummyFieldVariable_GetMaxGlobalFieldMagnitude;
	FieldVariable_GetCoordFunction*               _getMinAndMaxLocalCoords = DummyFieldVariable_GetMinAndMaxLocalCoords;
	FieldVariable_GetCoordFunction*              _getMinAndMaxGlobalCoords = DummyFieldVariable_GetMinAndMaxGlobalCoords;

	return _FieldVariable_New(  FIELDVARIABLE_PASSARGS  );
}

Index DummyFieldVariable_Register( PluginsManager* pluginsManager  ) {
	RegisterParent( DummyFieldVariable_Type, FieldVariable_Type );
	return PluginsManager_Submit( pluginsManager, DummyFieldVariable_Type, (Name)"0", _DummyFieldVariable_DefaultNew  );
}


