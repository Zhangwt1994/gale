/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
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
** $Id: Object.c 4253 2008-04-17 01:50:25Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "CommonRoutines.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type Stg_Object_Type = "Stg_Object";

Index _Stg_Object_Counter = 0;
static const char _Stg_Object_Unnamed[] = "Unnamed";

Stg_Object* _Stg_Object_New(  STG_OBJECT_DEFARGS  )
{
	Stg_Object* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_Object) );
	self = (Stg_Object*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* Stg_Object info */
	_Stg_Object_Init( self, name, nameAllocationType );
	
	return self;
}


void _Stg_Object_Init( Stg_Object* self, Name name, AllocationType nameAllocationType ) {
	/* General and Virtual info should already be set */
	
	/* Stg_Object info */
	if( !name || strlen( name ) == 0 ) {
          char *tmp_name;
          Stg_asprintf(&tmp_name,"%s-%u",_Stg_Object_Unnamed,_Stg_Object_Counter);
          self->name=tmp_name;
          _Stg_Object_Counter += 1;
	}
	else if ( GLOBAL == nameAllocationType ) {
          /* This is a bit unsafe */
          self->name = (char*)name;
	}
	else {
		self->name = StG_Strdup( name );
	}

	self->nameAllocationType = nameAllocationType;	
}


void _Stg_Object_Delete( void* object ) {
	Stg_Object* self = (Stg_Object*)object;
	
	if ( GLOBAL != self->nameAllocationType )
          Memory_Free( self->name );

	/* Delete parent class */
	_Stg_Class_Delete( self );
}


void _Stg_Object_Print( void* object, struct Stream* stream) {
	Stg_Object* self = (Stg_Object*)object;
	
	/* General info */
	Journal_Printf( stream, "Stg_Object (ptr): %p\n", (void*)self );
	Stream_Indent( stream );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	
	/* Stg_Object */
	Journal_Printf( stream, "name: %s\n", self->name );
	Journal_Printf( stream, "nameAllocationType: %s\n", self->nameAllocationType == GLOBAL ? "GLOBAL" : "NON_GLOBAL" );
	
	Stream_UnIndent( stream );
}

void* _Stg_Object_Copy( const void* object, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_Object*	self = (Stg_Object*)object;
	Stg_Object*	newObject;
	
	newObject = (Stg_Object*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* TODO: if we are not deep copying we should not copy the name, just the pointer.  There
	 * is a problem with this; will try to fix it later. */
	
	if( nameExt ) {
		unsigned	nameLen = strlen( self->name );
                char *tmp_name;
                tmp_name=
                  Memory_Alloc_Array_Unnamed(char,nameLen+strlen(nameExt)+1);
		memcpy( tmp_name, self->name, nameLen );
		strcpy( tmp_name + nameLen, nameExt );
                newObject->name=tmp_name;
	}
	else {
		newObject->name = StG_Strdup( self->name );
	}
	
	newObject->nameAllocationType = NON_GLOBAL;
	
	return newObject;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void Stg_Object_SetName( void* object, Name name )
{
	Stg_Object* self = (Stg_Object*)object;
	
	if ( GLOBAL == self->nameAllocationType )
	{
          self->name = (char*)name;
	}
	else
	{
          Memory_Free( self->name );
          self->name = StG_Strdup( name );
	}
}


/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


Name _Stg_Object_GetNameFunc( void* object ) {
	Stg_Object* self = (Stg_Object*)object;
	
	return _Stg_Object_GetNameMacro( self );
}

char * Stg_Object_AppendSuffix( void* object, Name suffix ) {
	Stg_Object* self = (Stg_Object*)object;
	char *name;

	Stg_asprintf( &name, "%s-%s", _Stg_Object_GetNameMacro( self ), suffix );

	return name;
}


