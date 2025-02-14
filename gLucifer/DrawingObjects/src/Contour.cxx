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
** $Id: Contour.c 791 2008-09-01 02:09:06Z JulianGiordani $
** 
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include <glucifer/Base/Base.h>
#include <glucifer/RenderingEngines/RenderingEngines.h>

#include "types.h"
#include "OpenGLDrawingObject.h"
#include "Contour.h"

#include <assert.h>
#include <gl.h>
#include <glu.h>
#include <string.h>

#ifndef MASTER
	#define MASTER 0
#endif

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucContour_Type = "lucContour";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucContour* _lucContour_New(  LUCCONTOUR_DEFARGS  ) 
{
	lucContour*					self;

	/* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
	assert( _sizeOfSelf >= sizeof(lucContour) );
	self = (lucContour*) _lucOpenGLDrawingObject_New(  LUCOPENGLDRAWINGOBJECT_PASSARGS  );
	
	return self;
}

void _lucContour_Init( 
		lucContour*      self,
		FieldVariable*   fieldVariable,
		lucColourMap*    colourMap,
		Name             colourName,
		IJK              resolution,
		double           lineWidth,
    Bool             showValues,
		double           interval )
{
	self->fieldVariable = fieldVariable;
	self->colourMap     = colourMap;
	lucColour_FromString( &self->colour, colourName );
	memcpy( self->resolution, resolution, sizeof(IJK) );
	self->lineWidth = lineWidth;
	self->showValues = showValues;
	self->interval = interval;
}

void _lucContour_Delete( void* drawingObject ) {
	lucContour*  self = (lucContour*)drawingObject;

	_lucOpenGLDrawingObject_Delete( self );
}

void _lucContour_Print( void* drawingObject, Stream* stream ) {
	lucContour*  self = (lucContour*)drawingObject;

	_lucOpenGLDrawingObject_Print( self, stream );
}

void* _lucContour_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) {
	lucContour*  self = (lucContour*)drawingObject;
	lucContour* newDrawingObject;

	newDrawingObject = _lucOpenGLDrawingObject_Copy( self, dest, deep, nameExt, ptrMap );

	/* TODO */
	abort();

	return (void*) newDrawingObject;
}


void* _lucContour_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                     _sizeOfSelf = sizeof(lucContour);
	Type                                                             type = lucContour_Type;
	Stg_Class_DeleteFunction*                                     _delete = _lucContour_Delete;
	Stg_Class_PrintFunction*                                       _print = _lucContour_Print;
	Stg_Class_CopyFunction*                                         _copy = NULL;
	Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucContour_DefaultNew;
	Stg_Component_ConstructFunction*                           _construct = _lucContour_AssignFromXML;
	Stg_Component_BuildFunction*                                   _build = _lucContour_Build;
	Stg_Component_InitialiseFunction*                         _initialise = _lucContour_Initialise;
	Stg_Component_ExecuteFunction*                               _execute = _lucContour_Execute;
	Stg_Component_DestroyFunction*                               _destroy = _lucContour_Destroy;
	lucDrawingObject_SetupFunction*                                _setup = _lucContour_Setup;
	lucDrawingObject_DrawFunction*                                  _draw = _lucContour_Draw;
	lucDrawingObject_CleanUpFunction*                            _cleanUp = _lucContour_CleanUp;
	lucOpenGLDrawingObject_BuildDisplayListFunction*    _buildDisplayList = _lucContour_BuildDisplayList;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _lucContour_New(  LUCCONTOUR_PASSARGS  );
}

void _lucContour_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data ){
	lucContour*      self = (lucContour*)drawingObject;
	Index            defaultResolution;
	FieldVariable*   fieldVariable;
	lucColourMap*    colourMap;
	IJK              resolution;
	Bool             showValues;

	/* Construct Parent */
	_lucOpenGLDrawingObject_AssignFromXML( self, cf, data );

	fieldVariable =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FieldVariable", FieldVariable, True, data  );
	colourMap     =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ColourMap", lucColourMap, False, data  );

	showValues = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"showValues", True  );
	defaultResolution = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolution", 8  );
	resolution[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionX", defaultResolution  );
	resolution[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionY", defaultResolution  );
	resolution[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"resolutionZ", defaultResolution  );
			
	_lucContour_Init( 
			self, 
			fieldVariable,
			colourMap,
			Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"colour", "black"  ),
			resolution,
			(float) Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"lineWidth", 1.0  ),
      showValues,
			Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"interval", 0.33 )  ) ;
}

void _lucContour_Build( void* drawingObject, void* data ) {}
void _lucContour_Initialise( void* drawingObject, void* data ) {}
void _lucContour_Execute( void* drawingObject, void* data ) {}
void _lucContour_Destroy( void* drawingObject, void* data ) {}

void _lucContour_Setup( void* drawingObject, void* _context ) {
	lucContour*            self            = (lucContour*)drawingObject;
	_lucOpenGLDrawingObject_Setup( self, _context );
}

void _lucContour_Draw( void* drawingObject, lucWindow* window, lucViewportInfo* viewportInfo, void* _context ) {
	lucContour*            self            = (lucContour*)drawingObject;
	_lucOpenGLDrawingObject_Draw( self, window, viewportInfo, _context );
}

void _lucContour_CleanUp( void* drawingObject, void* _context ) {
	lucContour*            self            = (lucContour*)drawingObject;
	_lucOpenGLDrawingObject_CleanUp( self, _context );
}
	
void _lucContour_BuildDisplayList( void* drawingObject, void* _context ) {
	lucContour*            self            = (lucContour*)drawingObject;
	FieldVariable*         fieldVariable   = self->fieldVariable;
	double                 isovalue;
	double                 interval        = self->interval;
	double                 minIsovalue     = FieldVariable_GetMinGlobalFieldMagnitude( fieldVariable );
	double                 maxIsovalue     = FieldVariable_GetMaxGlobalFieldMagnitude( fieldVariable );
	lucColourMap*          colourMap       = self->colourMap;
	Coord min, max;
	
	glLineWidth(self->lineWidth);

	FieldVariable_GetMinAndMaxLocalCoords( fieldVariable, min, max );
	
	lucColour_SetOpenGLColour( &self->colour );

	/* Draw isovalues at interval */
	if ( interval <= 0.0 ) 
		return;

	for ( isovalue = minIsovalue + interval ; isovalue < maxIsovalue ; isovalue += interval ) {
		if ( colourMap )
			lucColourMap_SetOpenGLColourFromValue( colourMap, isovalue );
		
		lucContour_DrawContourWalls( self, isovalue, min, max );
	}
}


#define LEFT   0
#define RIGHT  1
#define BOTTOM 2
#define TOP    3

void lucContour_DrawContourWalls( 
		void*                                             drawingObject,
		double                                            isovalue,
		Coord                                             min,
		Coord                                             max )
{
	lucContour*            self            = (lucContour*)drawingObject;
	Dimension_Index        dim             = self->fieldVariable->dim;

	if ( dim == 2 ) {
		lucContour_DrawContour( self, isovalue, 0.0, K_AXIS, min, max ); 
	}
	else {
		lucContour_DrawContour( self, isovalue, min[ I_AXIS ], I_AXIS, min, max ); 
		lucContour_DrawContour( self, isovalue, max[ I_AXIS ], I_AXIS, min, max ); 
		lucContour_DrawContour( self, isovalue, min[ J_AXIS ], J_AXIS, min, max ); 
		lucContour_DrawContour( self, isovalue, max[ J_AXIS ], J_AXIS, min, max ); 
		lucContour_DrawContour( self, isovalue, min[ K_AXIS ], K_AXIS, min, max ); 
		lucContour_DrawContour( self, isovalue, max[ K_AXIS ], K_AXIS, min, max ); 
	}
}
	

void lucContour_DrawContour( 
		void*                                             drawingObject,
		double                                            isovalue,
		double                                            planeHeight,
		Axis                                              planeAxis,
		Coord                                             min,
		Coord                                             max )
{
	lucContour*            self            = (lucContour*)drawingObject;
	FieldVariable*         fieldVariable   = self->fieldVariable;
	Axis                   aAxis           = ( planeAxis == I_AXIS ? J_AXIS : I_AXIS );
	Axis                   bAxis           = ( planeAxis == K_AXIS ? J_AXIS : K_AXIS );
	unsigned int           elementType;
	unsigned int           i, j;
	Coord                  pos;
	double **              array;
	Index                  resolutionA     = self->resolution[ aAxis ];
	Index                  resolutionB     = self->resolution[ bAxis ];
  char                   numberStr[10];
	double                 dA, dB;
  int rememberCoord = 0;
  double writePos[2];
	
	/* Find position of cross - section */
	pos[planeAxis] = planeHeight;
	
	/* Calculate number of points in direction A and B */
	dA = (max[ aAxis ] - min[ aAxis ])/(double) (resolutionA - 1);
	dB = (max[ bAxis ] - min[ bAxis ])/(double) (resolutionB - 1);

	array = Memory_Alloc_2DArray( double , resolutionA, resolutionB, (Name)"Field Values" );
	for ( i = 0, pos[ aAxis ] = min[ aAxis ] ; i < resolutionA ; i++, pos[aAxis] += dA ) {
		for ( j = 0, pos[bAxis] = min[ bAxis ] ; j < resolutionB ; j++, pos[bAxis] += dB ) {
			if (pos[aAxis] > max[ aAxis ]) 
				pos[aAxis] = max[ aAxis ];
			if (pos[bAxis] > max[ bAxis ]) 
				pos[bAxis] = max[ bAxis ];

			/* Interpolate value to point */
			FieldVariable_InterpolateValueAt( fieldVariable, pos, &array[i][j] );
		}
	}
		
	/* Initialise OpenGL stuff */
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glDisable(GL_DEPTH_TEST); /* Depth test messes up line anti-aliasing, needs checking on multiple procs */
	glBegin(GL_LINES);

	/* Start marching rectangles */
	for ( i = 0, pos[ aAxis ] = min[ aAxis ] ; i < resolutionA - 1 ; i++, pos[ aAxis ] += dA ) {
		for ( j = 0, pos[ bAxis ] = min[ bAxis ] ; j < resolutionB - 1 ; j++, pos[ bAxis ] += dB ) {
			/* Assign a unique number to the square type from 0 to 15 */
			elementType = 0;
			if (array[i][j]     > isovalue) 	elementType += 1;
			if (array[i+1][j]   > isovalue) 	elementType += 2;
			if (array[i][j+1]   > isovalue) 	elementType += 4;
			if (array[i+1][j+1] > isovalue) 	elementType += 8;

			switch ( elementType ) {
				case 0:
					/*  @@  */
					/*  @@  */
					break;
				case 1:		
					/*  @@  */
					/*  #@  */
					lucContour_PlotPoint( LEFT,   isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 2:
					/*  @@  */
					/*  @#  */	
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 3:
					/*  @@  */
					/*  ##  */	
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					if( rememberCoord == 0 )  {
            writePos[0] = pos[aAxis] + dA;
            writePos[1] = pos[bAxis] + dB;
            rememberCoord = 1;
          }break;
				case 4:
					/*  #@  */
					/*  @@  */
					lucContour_PlotPoint( LEFT  , isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( TOP   , isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 5:
					/*  #@  */
					/*  #@  */
					lucContour_PlotPoint( TOP   , isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					if( rememberCoord == 0 )  {
            writePos[0] = pos[aAxis] + dA;
            writePos[1] = pos[bAxis] + dB;
            rememberCoord = 1;
          }break;
				case 6:
					/*  #@  */
					/*  @#  */
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( TOP , isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );

					lucContour_PlotPoint( RIGHT , isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 7:
					/*  #@  */
					/*  ##  */
					lucContour_PlotPoint( TOP, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 8:
					/*  @#  */
					/*  @@  */
					lucContour_PlotPoint( TOP, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 9:
					/*  @#  */
					/*  #@  */
					lucContour_PlotPoint( TOP, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );

					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 10:
					/*  @#  */
					/*  @#  */
					lucContour_PlotPoint( TOP, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
          if( rememberCoord == 0 )  {
            writePos[0] = pos[aAxis] + dA;
            writePos[1] = pos[bAxis] + dB;
            rememberCoord = 1;
          }
					break;
				case 11:
					/*  @#  */
					/*  ##  */
					lucContour_PlotPoint( TOP, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 12:
					/*  ##  */
					/*  @@  */
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					if( rememberCoord == 0 )  {
            writePos[0] = pos[aAxis] + dA;
            writePos[1] = pos[bAxis] + dB;
            rememberCoord = 1;
          }break;
				case 13:
					/*  ##  */
					/*  #@  */
					lucContour_PlotPoint( RIGHT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 14:
					/*  ##  */
					/*  @#  */
					lucContour_PlotPoint( LEFT, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					lucContour_PlotPoint( BOTTOM, isovalue, array[i][j], array[i+1][j], array[i][j+1], array[i+1][j+1], pos, dA, dB, planeAxis );
					break;
				case 15:
					/*  ##  */
					/*  ##  */
					break;
				default:
					fprintf(stderr, "In func %s: Cannot find element %d.\n", __func__, elementType );
					abort();
			}
			
		}
	}
	glEnd();

  /* print the isovalue "near" the last isovalue */
  sprintf( numberStr, "%g", isovalue );
  lucPrint( writePos[0], writePos[1], numberStr);
  rememberCoord= 0;

	glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

	/* Clean up */
	Memory_Free(array);
}

void lucContour_PlotPoint( char edge, double isovalue, double leftBtm, double rightBtm, double leftTop, double rightTop , Coord pos, double dA, double dB, Axis planeAxis ) {
	Axis            aAxis           = ( planeAxis == I_AXIS ? J_AXIS : I_AXIS );
	Axis            bAxis           = ( planeAxis == K_AXIS ? J_AXIS : K_AXIS );
	Coord           vertex;
	
	vertex[planeAxis] = pos[planeAxis];

	switch (edge) {
		case BOTTOM:
			vertex[aAxis] = pos[aAxis] + dA * (isovalue - leftBtm)/(rightBtm - leftBtm) ; 
			vertex[bAxis] = pos[bAxis];
			break;
		case TOP:	
			vertex[aAxis] = pos[aAxis] + dA * (isovalue - leftTop)/(rightTop - leftTop); 
			vertex[bAxis] = pos[bAxis] + dB;
			break;
		case LEFT:	
			vertex[aAxis] = pos[aAxis];
			vertex[bAxis] = pos[bAxis] + dB * (isovalue - leftBtm)/(leftTop - leftBtm); 
			break;
		case RIGHT:	
			vertex[aAxis] = pos[aAxis] + dA;
			vertex[bAxis] = pos[bAxis] + dB * (isovalue - rightBtm)/(rightTop - rightBtm ); 
			break;
	}
	glVertex3dv(vertex);
}


