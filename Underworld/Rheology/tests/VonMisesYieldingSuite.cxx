#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include "Underworld/Underworld.h"

/* silly stgermain, I must define this */
#define CURR_MODULE_NAME "UnderworldContext.c"

const Type Underworld_testVonMisesYielding_Type = "Underworld_testVonMisesYielding";

/* Define plugin structure */
typedef struct {
	__Codelet
	YieldRheology_HasYieldedFunction* realHasYieldedFunction;
	FeMesh* 		          mesh;
	XYZ                   min;
	XYZ                   max;
	Bool                  hasYielded;
} Underworld_testVonMisesYielding;

/* define global vars .... that's very ugly, but it's only a pcu test */
Underworld_testVonMisesYielding VMYS_globalSelf;

typedef struct {
} VonMisesYieldingSuiteData;

void VonMisesYieldingSuite_Setup( VonMisesYieldingSuiteData* data ) { 
}

void VonMisesYieldingSuite_Teardown( VonMisesYieldingSuiteData* data ) {
}

double Underworld_testVonMisesYielding_dt( FiniteElementContext* context ) {
	if ( context->currentTime >= 0.65 ) {
		return 0.01;
	}
	return HUGE_VAL;
}

void testVonMisesYielding_HasYielded( 
		void*                            yieldRheology,
		ConstitutiveMatrix*              constitutiveMatrix,
		MaterialPointsSwarm*             materialPointsSwarm,
		Element_LocalIndex               lElement_I,
		MaterialPoint*                   materialPoint,
		double                           yieldCriterion,
		double                           yieldIndicator ) 
{
	Dimension_Index dim_I;

	/* Call real 'HasYielded' function */
	VMYS_globalSelf.realHasYieldedFunction( 
			yieldRheology, constitutiveMatrix, materialPointsSwarm, lElement_I, materialPoint, yieldCriterion, yieldIndicator );

	/* Don't output information if this is the first non-linear iteration */
	if ( constitutiveMatrix->sleNonLinearIteration_I == 0 ) {
		return;
	}

	/* Store information */
	VMYS_globalSelf.hasYielded = True;
	for ( dim_I = 0 ; dim_I < constitutiveMatrix->dim ; dim_I++ ) {
		if ( materialPoint->coord[ dim_I ] < VMYS_globalSelf.min[ dim_I ] )
			VMYS_globalSelf.min[ dim_I ] = materialPoint->coord[ dim_I ];
		if ( materialPoint->coord[ dim_I ] > VMYS_globalSelf.max[ dim_I ] )
			VMYS_globalSelf.max[ dim_I ] = materialPoint->coord[ dim_I ];
	}
}


void Underworld_testVonMisesYielding_Check( FiniteElementContext* context ) {
	Stream* stream = Journal_Register( Dump_Type, (Name)Underworld_testVonMisesYielding_Type );

   if(context->rank == 0){
      /* Don't do anything if nothing has yielded yet */
      if ( !VMYS_globalSelf.hasYielded ) {
         return;
      }
   
      /* Set the stream to point to our output file (so we can do a diff on it later ) */
      Stream_Enable( stream, True );
      Stream_RedirectFile_WithPrependedPath( stream, context->outputPath, "testVonMisesYielding.dat" );
   
      Journal_Printf( stream, "Material yielded at time %12.4g (step %u) within:\n", context->currentTime, context->timeStep ); 
   
      /* Output information */
      Journal_Printf( stream, "\tx: %12.4g - %12.4g\n", VMYS_globalSelf.min[ I_AXIS ], VMYS_globalSelf.max[ I_AXIS ] );
      Journal_Printf( stream, "\ty: %12.4g - %12.4g\n", VMYS_globalSelf.min[ J_AXIS ], VMYS_globalSelf.max[ J_AXIS ] );
      if ( context->dim == 3 ) {
         Journal_Printf( stream, "\tz: %12.4g - %12.4g\n", VMYS_globalSelf.min[ K_AXIS ], VMYS_globalSelf.max[ K_AXIS ] );
      }
      Stream_CloseAndFreeFile( stream );
   }
}

void Underworld_testVonMisesYielding_Check_Sync( FiniteElementContext* context ) {
   Coord mins;
   Coord maxs;
   
   /* get true global max/min */

   MPI_Allreduce( &VMYS_globalSelf.min[ I_AXIS ], &mins[I_AXIS], 1, MPI_DOUBLE, MPI_MIN, context->communicator );
   MPI_Allreduce( &VMYS_globalSelf.min[ J_AXIS ], &mins[J_AXIS], 1, MPI_DOUBLE, MPI_MIN, context->communicator );
   if ( context->dim == 3 )
      MPI_Allreduce( &VMYS_globalSelf.min[ K_AXIS ], &mins[K_AXIS], 1, MPI_DOUBLE, MPI_MIN, context->communicator );

   MPI_Allreduce( &VMYS_globalSelf.max[ I_AXIS ], &maxs[I_AXIS], 1, MPI_DOUBLE, MPI_MAX, context->communicator );
   MPI_Allreduce( &VMYS_globalSelf.max[ J_AXIS ], &maxs[J_AXIS], 1, MPI_DOUBLE, MPI_MAX, context->communicator );
   if ( context->dim == 3 )
      MPI_Allreduce( &VMYS_globalSelf.max[ K_AXIS ], &maxs[K_AXIS], 1, MPI_DOUBLE, MPI_MAX, context->communicator );
   
   /* now set to these values */
   VMYS_globalSelf.min[ I_AXIS ] = mins[ I_AXIS ];
   VMYS_globalSelf.min[ J_AXIS ] = mins[ J_AXIS ];
   if ( context->dim == 3 )
      VMYS_globalSelf.min[ K_AXIS ] = mins[ K_AXIS ];
   VMYS_globalSelf.max[ I_AXIS ] = maxs[ I_AXIS ];
   VMYS_globalSelf.max[ J_AXIS ] = maxs[ J_AXIS ];
   if ( context->dim == 3 )
      VMYS_globalSelf.max[ K_AXIS ] = maxs[ K_AXIS ];
}


void VonMisesYieldingSuite_VonMises2D( VonMisesYieldingSuiteData* data ) {
	UnderworldContext* context;
	YieldRheology*          yieldRheology;
	Stg_ComponentFactory*	cf;
	char							expected_file[PCU_PATH_MAX];
	char							xml_input[PCU_PATH_MAX];

	/* read in the xml input file */
	pcu_filename_input( "testVonMisesYieldCriterion.xml", xml_input );
	cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
	context = (UnderworldContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context" );

	stgMainBuildAndInitialise( cf  );

	/* get pointer to the mesh */
	VMYS_globalSelf.mesh = Stg_ComponentFactory_ConstructByName( cf, (Name)"linearMesh", FeMesh, True, NULL ); 
	
	/* Get a pointer the yield rheology that we are trying to test */
	yieldRheology = (YieldRheology* ) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"yieldRheology"  );
	
	/* Store the pointer to the original 'HasYielded' function */
	VMYS_globalSelf.realHasYieldedFunction = yieldRheology->_hasYielded;

	/* Reset this function pointer with our own */
	yieldRheology->_hasYielded = testVonMisesYielding_HasYielded;

   ContextEP_Append( context, AbstractContext_EP_Step, Underworld_testVonMisesYielding_Check_Sync );
	ContextEP_Append( context, AbstractContext_EP_Step, Underworld_testVonMisesYielding_Check );
	EP_AppendClassHook( Context_GetEntryPoint( context, FiniteElementContext_EP_CalcDt ), Underworld_testVonMisesYielding_dt, context );

	VMYS_globalSelf.min[ I_AXIS ] =  HUGE_VAL;
	VMYS_globalSelf.min[ J_AXIS ] =  HUGE_VAL;
	VMYS_globalSelf.min[ K_AXIS ] =  HUGE_VAL;
	VMYS_globalSelf.max[ I_AXIS ] = -HUGE_VAL;
	VMYS_globalSelf.max[ J_AXIS ] = -HUGE_VAL;
	VMYS_globalSelf.max[ K_AXIS ] = -HUGE_VAL;

	stgMainLoop( cf );

	MPI_Barrier( context->communicator );
	pcu_filename_expected( "testVonMisesYieldCriterion.expected", expected_file );
	pcu_check_fileEq( "output/testVonMisesYielding.dat", expected_file );

	if(context->rank == 0){
      remove("output/testVonMisesYielding.dat");
      remove("output");
   }

	stgMainDestroy( cf );
}

void VonMisesYieldingSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, VonMisesYieldingSuiteData );
   pcu_suite_setFixtures( suite, VonMisesYieldingSuite_Setup, VonMisesYieldingSuite_Teardown );
   pcu_suite_addTest( suite, VonMisesYieldingSuite_VonMises2D );
}


