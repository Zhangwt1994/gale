/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	David May, PhD Student Monash University, VPAC. (david.may@sci.maths.monash.edu.au)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
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
** $Id: LevelSetPlg.c 200 2005-07-08 08:24:41Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "types.h"
#include "Context.h"
#include "EulerDeform.h"

Variable* EulerDeform_RegisterLocalNodeCoordsAsVariables( EulerDeform_System* sys, void* _variable_Register, Variable** variableList ) {
	FeMesh*					self = (FeMesh*)sys->mesh;
	Variable_Register*	variable_Register = (Variable_Register*) _variable_Register;
	Variable*				variable;
	char*						variableName;
	char*						variableNameX;
	char*						variableNameY;
	char*						variableNameZ;

	/* Allocate advection array. */
	sys->verts = AllocArray( double, Mesh_GetLocalSize( self, MT_VERTEX ) * Mesh_GetDimSize( self ) );
	
	/* Append Extension onto names */
	variableName  = Memory_Alloc_Array( char, strlen( self->name ) + strlen( "NodeCoords" ) + 1, "variableName" );
	sprintf( variableName , "%sNodeCoords", self->name );
	
	variableNameX = Memory_Alloc_Array( char, strlen( self->name ) + strlen( "NodeCoordX" ) + 1, "variableNameX" );
	sprintf( variableNameX, "%sNodeCoordX", self->name );

	variableNameY = Memory_Alloc_Array( char, strlen( self->name ) + strlen( "NodeCoordY" ) + 1, "variableNameY" );
	sprintf( variableNameY, "%sNodeCoordY", self->name );

	variableNameZ = Memory_Alloc_Array( char, strlen( self->name ) + strlen( "NodeCoordZ" ) + 1, "variableNameZ" );
	sprintf( variableNameZ, "%sNodeCoordZ", self->name );
	
	/* Construct */
	variable = Variable_NewVector( 
		variableName, 
		self->context,	
		Variable_DataType_Double, 
		Mesh_GetDimSize( self ), 
		(unsigned*)&((IGraph*)self->topo)->remotes[MT_VERTEX]->decomp->locals->size, 
		NULL,
		(void**)&sys->verts, 
		variable_Register, 
		variableNameX,
		variableNameY,
		variableNameZ );

	if ( variableList != NULL ) {
		variableList[ I_AXIS ] = Variable_Register_GetByName( variable_Register, variableNameX );
		variableList[ J_AXIS ] = Variable_Register_GetByName( variable_Register, variableNameY );
		variableList[ K_AXIS ] = Variable_Register_GetByName( variable_Register, variableNameZ );
	}

	/* Clean Up */
	Memory_Free( variableNameZ );
	Memory_Free( variableNameY );
	Memory_Free( variableNameX );
	Memory_Free( variableName );

	return variable;
}
