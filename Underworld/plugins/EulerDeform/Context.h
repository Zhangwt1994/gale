/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Victorian Partnership for Advanced Computing (VPAC) Ltd, Australia
** (C) 2003-2005 All Rights Reserved
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	David May, PhD Student Monash University, VPAC. (david.may@sci.maths.monash.edu.au)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**
** Role:
**	Defines any header info, such as new structures, needed by this plugin
**
** Assumptions:
**
** Comments:
**
** $Id $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Underworld_plugins_EulerDeform_Context_h__
#define __Underworld_plugins_EulerDeform_Context_h__

	struct EulerDeform_Context {
		unsigned		nSystems;
		EulerDeform_System*	systems;
		AbstractContext*        ctx;
      TimeIntegrator*         timeIntegrator;
	};

	struct EulerDeform_System {
		Mesh*		mesh;
		Mesh*		p_mesh;
		Mesh*		T_mesh;
		double*		verts;
	        FeVariable*     dispField;
                Stg_Component_ExecuteFunction* energySolverExecute;
		Remesher*	remesher;
		int             interval;
		FieldVariable*	velField;
		Variable**	vars;
		Bool		wrapTop;
		Bool		wrapBottom;
		Bool		wrapLeft;

		Bool		staticLeft;
		Bool		staticRight;
		Bool		staticTop;
		Bool		staticBottom;
		Bool		staticFront;
		Bool		staticBack;

		Bool		staticLeftTop;
		Bool		staticRightTop;
		Bool		staticLeftTopFront;
		Bool		staticRightTopFront;
		Bool		staticLeftTopBack;
		Bool		staticRightTopBack;

		Bool		staticLeftBottom;
		Bool		staticRightBottom;
		Bool		staticLeftBottomFront;
		Bool		staticRightBottomFront;
		Bool		staticLeftBottomBack;
		Bool		staticRightBottomBack;

		Bool		staticLeftFront;
		Bool		staticRightFront;
		Bool		staticLeftBack;
		Bool		staticRightBack;

		Bool		staticTopFront;
		Bool		staticBottomFront;
		Bool		staticTopBack;
		Bool		staticBottomBack;

		Bool		floatLeftTop;
		Bool		floatRightTop;

		Bool		staticSides;
		double**	sideCoords;
                double          static_right_coord;
                double          static_left_coord;
                double          static_front_coord;
                double          static_back_coord;
                char*           static_right_equation;
                char*           static_left_equation;
                char*           static_front_equation;
                char*           static_back_equation;
	};

#endif
