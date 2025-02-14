#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#ifndef __Underworld_Velic_solHAy_h__
#define __Underworld_Velic_solHAy_h__

	extern const Type Velic_solHAy_Type;

	typedef struct {
		__AnalyticSolution
		double sigma;
	       	double eta;
		double dx;
	       	double dy;
		double x0;
	       	double y0;
	} Velic_solHAy;

	Index Underworld_Velic_solHAy_Register( PluginsManager* pluginsManager );
	void* _Velic_solHAy_DefaultNew( Name name );
	void _Velic_solHAy_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );
	void _Velic_solHAy_Init( Velic_solHAy* self, double sigma, double eta, double dx, double dy, double x0, double y0 );

	void Velic_solHAy_PressureFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* pressure );
	void Velic_solHAy_VelocityFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* velocity );
	void Velic_solHAy_StressFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* stress );
	void Velic_solHAy_StrainRateFunction( void* analyticSolution, FeVariable* analyticFeVariable, const double *coord, double* strainRate );

	void _Velic_solHAy( 
		const double pos[],
		double _sigma, double _eta,
		double _dx, double _dy,
		double _x_0, double _y_0,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] );

#endif
