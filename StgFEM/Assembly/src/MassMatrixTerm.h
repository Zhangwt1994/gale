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
*/


#ifndef __StgFEM_Assembly_MassMatrixTerm_h__
#define __StgFEM_Assembly_MassMatrixTerm_h__

	/** Textual name of this class */
	extern const Type MassMatrixTerm_Type;

	/** MassMatrixTerm class contents */
	#define __MassMatrixTerm \
		/* General info */ \
		__StiffnessMatrixTerm \
		\
		/* Virtual info */ \
		\
		/* MassMatrixTerm info */ \
		FeVariable*	field; \

	struct MassMatrixTerm { __MassMatrixTerm };

	MassMatrixTerm* MassMatrixTerm_New( 
		Name							name,
		FiniteElementContext*	context,
		StiffnessMatrix*			stiffMat,
		Swarm*						integrationSwarm,
		FeVariable*					gradField );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MASSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define MASSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	MassMatrixTerm* _MassMatrixTerm_New(  MASSMATRIXTERM_DEFARGS  );

	void _MassMatrixTerm_Init( void* matrixTerm, FeVariable* field );
	
	void _MassMatrixTerm_Delete( void* matrixTerm );

	void _MassMatrixTerm_Print( void* matrixTerm, Stream* stream );

	void* _MassMatrixTerm_DefaultNew( Name name );

	void _MassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _MassMatrixTerm_Build( void* matrixTerm, void* data );

	void _MassMatrixTerm_Initialise( void* matrixTerm, void* data );

	void _MassMatrixTerm_Execute( void* matrixTerm, void* data );

	void _MassMatrixTerm_Destroy( void* matrixTerm, void* data );

	void _MassMatrixTerm_AssembleElement(
		void*							matrixTerm,
		StiffnessMatrix*			stiffMat, 
		Element_LocalIndex		lElement_I, 
		SystemLinearEquations*	sle, 
		FiniteElementContext*	context, 
		double**						elStiffMat );

#endif

