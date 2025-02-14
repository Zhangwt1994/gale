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
** $Id: TrilinearInnerElType.c 832 2007-05-16 01:11:18Z DaveLee $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "ElementType.h"
#include "TrilinearInnerElType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type TrilinearInnerElType_Type = "TrilinearInnerElType";

#define _TrilinearInnerElType_NodeCount 4

void* _TrilinearInnerElType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(TrilinearInnerElType);
	Type                                                                                    type = TrilinearInnerElType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _TrilinearInnerElType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _TrilinearInnerElType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _TrilinearInnerElType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _TrilinearInnerElType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _TrilinearInnerElType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _TrilinearInnerElType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _TrilinearInnerElType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _TrilinearInnerElType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _TrilinearInnerElType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _TrilinearInnerElType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_JacobianDeterminantSurfaceFunction*                  _jacobianDeterminantSurface = _ElementType_JacobianDeterminantSurface;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = _TrilinearInnerElType_SurfaceNormal;

	return _TrilinearInnerElType_New(  TRILINEARINNERELTYPE_PASSARGS  );
}

TrilinearInnerElType* TrilinearInnerElType_New( Name name ) {
  TrilinearInnerElType* self = (TrilinearInnerElType*)_TrilinearInnerElType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _TrilinearInnerElType_NodeCount );
	_TrilinearInnerElType_Init( self );

	return self;
}

TrilinearInnerElType* _TrilinearInnerElType_New(  TRILINEARINNERELTYPE_DEFARGS  ) {
	TrilinearInnerElType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(TrilinearInnerElType) );
	self = (TrilinearInnerElType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* TrilinearInnerElType info */
	
	return self;
}

void _TrilinearInnerElType_Init( TrilinearInnerElType* self ) {
	Dimension_Index dim_I=0;

	/* General and Virtual info should already be set */
	
	self->dim = 3;
	/* TriInnerEllementType info */
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] = 1;
		self->elLocalLength[dim_I] = self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}

	/* Set up the tetrahedral indices. */
	self->tetInds = Memory_Alloc_2DArray( unsigned, 10, 4, (Name)"Mesh_HexType::tetInds" );
	self->tetInds[0][0] = 0; self->tetInds[0][1] = 1; self->tetInds[0][2] = 2; self->tetInds[0][3] = 4;
	self->tetInds[1][0] = 1; self->tetInds[1][1] = 2; self->tetInds[1][2] = 3; self->tetInds[1][3] = 7;
	self->tetInds[2][0] = 1; self->tetInds[2][1] = 4; self->tetInds[2][2] = 5; self->tetInds[2][3] = 7;
	self->tetInds[3][0] = 2; self->tetInds[3][1] = 4; self->tetInds[3][2] = 6; self->tetInds[3][3] = 7;
	self->tetInds[4][0] = 1; self->tetInds[4][1] = 2; self->tetInds[4][2] = 4; self->tetInds[4][3] = 7;
	self->tetInds[5][0] = 0; self->tetInds[5][1] = 1; self->tetInds[5][2] = 3; self->tetInds[5][3] = 5;
	self->tetInds[6][0] = 0; self->tetInds[6][1] = 4; self->tetInds[6][2] = 5; self->tetInds[6][3] = 6;
	self->tetInds[7][0] = 0; self->tetInds[7][1] = 2; self->tetInds[7][2] = 3; self->tetInds[7][3] = 6;
	self->tetInds[8][0] = 3; self->tetInds[8][1] = 5; self->tetInds[8][2] = 6; self->tetInds[8][3] = 7;
	self->tetInds[9][0] = 0; self->tetInds[9][1] = 3; self->tetInds[9][2] = 5; self->tetInds[9][3] = 6;
}

void _TrilinearInnerElType_Delete( void* elementType ) {
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self  );
}

void _TrilinearInnerElType_Print( void* elementType, Stream* stream ) {
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* trilinearInnerElTypeStream = stream;
	
	/* General info */
	Journal_Printf( trilinearInnerElTypeStream, "TrilinearInnerElType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, trilinearInnerElTypeStream );
	
	/* Virtual info */
	
	/* TrilinearInnerElType info */
}

void _TrilinearInnerElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _TrilinearInnerElType_Initialise( void* elementType, void *data ){
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;
	
	self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
	self->GNi = Memory_Alloc_2DArray( double, self->dim, self->nodeCount, (Name)"localShapeFuncDerivitives"  );
}
	
void _TrilinearInnerElType_Execute( void* elementType, void *data ){
	
}
	
void _TrilinearInnerElType_Destroy( void* elementType, void *data ){
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;
	
	Memory_Free( self->evaluatedShapeFunc );
	Memory_Free( self->GNi );

	FreeArray( self->tetInds );

	_ElementType_Destroy( self, data );
}

void _TrilinearInnerElType_Build( void* elementType, void *data ) {
	
}

#if 0
void _TrilinearInnerElType_ConvertGlobalCoordToElementLocal( void* elementType, Element_DomainIndex element,const Coord globalCoord, Coord localCoord ) 
{
	TrilinearInnerElType*	self = (TrilinearInnerElType*)elementType;
	Dimension_Index		dim_I;

	for ( dim_I=0; dim_I < 3; dim_I++ ) {
	}
}
#endif


/*

 - Shape function definitions
 - Local node numbering convention for billinear, trilinear element (xi, eta, zeta)
 - Local coordinate domain spans  -1 <= xi,eta,zeta <= 1

    eta
     |
     |____ xi
    /
   /
 zeta


  eta
   |
3-----2
|  |__|___xi
|     |
0-----1
(zeta = -1 plane)


  eta
   |
7-----6
|  |__|___xi
|     |
4-----5
(zeta = +1 plane)


*/
void _TrilinearInnerElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	double xi, eta, zeta;
	
	xi   = localCoord[0];
	eta  = localCoord[1];
	zeta = localCoord[2];	
	
	evaluatedValues[0] = -xi - eta/2 - zeta/4 + 0.125;
	evaluatedValues[1] =  xi - eta/2 - zeta/4 + 0.125;
	evaluatedValues[2] =       eta   - zeta/2 + 0.25;
	evaluatedValues[3] =               zeta   + 0.5;
}



void _TrilinearInnerElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives )
{		
	evaluatedDerivatives[0][0] = -1;
	evaluatedDerivatives[0][1] =  1;
	evaluatedDerivatives[0][2] =  0;
	evaluatedDerivatives[0][3] =  0;
	                             
	evaluatedDerivatives[1][0] = -0.5;
	evaluatedDerivatives[1][1] = -0.5;
	evaluatedDerivatives[1][2] = 1;
	evaluatedDerivatives[1][3] = 0;
	                            
	evaluatedDerivatives[2][0] = -0.25;
	evaluatedDerivatives[2][1] = -0.25;
        evaluatedDerivatives[2][2] = -0.5;
	evaluatedDerivatives[2][3] = 1;
}

int _TrilinearInnerElType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal ) {
	Stream* errStream = Journal_Register( ErrorStream_Type, (Name)ElementType_Type  );

	Journal_Printf( errStream, "Surface normal function not yet implemented for this element type.\n" );
	assert( 0 );

	normal = NULL;

	return -1;
}




