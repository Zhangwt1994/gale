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
** $Id: Title.h 510 2006-02-17 04:33:32Z RobertTurnbull $
** 
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/



#ifndef __lucTitle_h__
#define __lucTitle_h__

	extern const Type lucTitle_Type;

	#define __lucTitle                              \
		__lucOpenGLDrawingObject \
		char*                                              titleString;  \
		lucColour                                          colour;       \
		int                                                yPos;
		
	struct lucTitle {__lucTitle};

	/** Constructors */
	lucTitle* lucTitle_New( 
		Name                                               name,
		char*                                              string,
		lucColour                                          colour,
		int                                                yPos );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LUCTITLE_DEFARGS \
                LUCOPENGLDRAWINGOBJECT_DEFARGS

	#define LUCTITLE_PASSARGS \
                LUCOPENGLDRAWINGOBJECT_PASSARGS

	lucTitle* _lucTitle_New(  LUCTITLE_DEFARGS  );

	void lucTitle_InitAll( 
		void*                                              title,
		char*                                              string, 
		lucColour                                          colour,
		int                                                yPos );

	void _lucTitle_Init( 
		void*                                              title,
		char*                                              string, 
		lucColour                                          colour,
		int                                                yPos );


	void _lucTitle_Setup( void* drawingObject, void* _context );
		
	/** Virtual Functions */
	void _lucTitle_Delete( void* title ) ;
	void _lucTitle_Print( void* title, Stream* stream ) ;
	void* _lucTitle_Copy( void* title, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
	#define lucTitle_Copy( self ) \
		(lucTitle*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	void* _lucTitle_DefaultNew( Name name ) ;
void _lucTitle_AssignFromXML( void* title, Stg_ComponentFactory* cf, void* data ) ;
	void _lucTitle_Build( void* title, void* data );
	void _lucTitle_Initialise( void* title, void* data );
	void _lucTitle_Execute( void* title, void* data );
	void _lucTitle_Destroy( void* title, void* data );
	void _lucTitle_Draw( void* drawingObject, lucWindow* window, lucViewportInfo* viewportInfo, void* _context );
	void _lucTitle_CleanUp( void* drawingObject, void* _context );
	void _lucTitle_BuildDisplayList( void* drawingObject, void* _context );

#endif

