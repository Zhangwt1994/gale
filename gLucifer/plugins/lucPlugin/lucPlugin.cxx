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
** $Id: lucPlugin.c 756 2008-02-12 00:28:41Z DavidMay $
** 
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <glucifer/glucifer.h>


const Type LucPlugin_Type = "LucPlugin";


/** Special run function which replaces the abstract context's normal one */
void lucPlugin_VisualOnlyRun( Context* _context ) {
	DomainContext*  context = (DomainContext*)_context;
	double                  dtLoadedFromFile = 0;
	Index                   fieldVar_I;
	FieldVariable*          fieldVar;
	Swarm_Register*         swarm_Register = Swarm_Register_GetSwarm_Register();
	Swarm*                  swarm;
	Index                   swarm_I;
	
	Journal_Printf( context->info, "** Running in visualOnly mode**\n" );

	if (context->maxTimeSteps>=0) {
		Journal_Printf( context->info, "Visualise timestep data until %u timeSteps have been run\n", context->maxTimeSteps );
	}
	if (context->finalTimeStep ) {
		if (context->maxTimeSteps>=0 ) {
			Journal_Printf( context->info, "or " );
		}	
		else {
			Journal_Printf( context->info, "visualise " );
		}
		Journal_Printf( context->info, "until absolute time step %u reached\n", context->stopTime );
	}
	
	if (context->stopTime) {
		if (context->maxTimeSteps>=0 || context->finalTimeStep ) {
			Journal_Printf( context->info, "or " );
		}	
		else {
			Journal_Printf( context->info, "Visualise " );
		}
		Journal_Printf( context->info, "until simulation time passes %g.\n", context->stopTime );
	}
	
	context->timeStepSinceJobRestart = 0;

	if ( False == context->loadFromCheckPoint ) { 
		context->timeStep = 0;
		/* Need to ensure restart mode enabled so the swarms & feVariables do the appropriate
		checkpoint loading stuff in their initialisation phase */
		context->loadFromCheckPoint = True;
		context->restartTimestep = 0;
		/* Need to set the swarm particle layout types now */
		for ( swarm_I = 0; swarm_I < swarm_Register->swarmList->count; swarm_I++ ) {

			swarm = Swarm_Register_At( swarm_Register, swarm_I );
			
			if ( True == swarm->isSwarmTypeToCheckPointAndReload ) {
				Swarm_ReplaceCurrentParticleLayoutWithFileParticleLayout( swarm, context );
			}
		}	
	}

	Dictionary_Set( context->dictionary, (Dictionary_Entry_Key)"restartTimestep", Dictionary_Entry_Value_FromUnsignedInt( context->restartTimestep ) );

	while (1 ) {
		/* If this checkpoint actually exists for this timestep, load it & possibly vis. it */
		if ( True == AbstractContext_CheckPointExists( context, context->timeStep ) ) {
			/* Note : always load timeInfo (currentTime) from file as we aren't solving anything,
			but may want to visualise currentTime. */
			_AbstractContext_LoadTimeInfoFromCheckPoint( (AbstractContext*)context, context->timeStep,
				&dtLoadedFromFile );
			/* We ignore dt, as this is the dt checkpointed to use for the _next_ timestep, so we
			 * don't want it to change the currentTime calculation for this timestep */

			/* Instead of solve here, re-load from checkpoint anything necessary to visualise results */
			
			for ( swarm_I = 0; swarm_I < swarm_Register->swarmList->count; swarm_I++ ) {

				swarm = Swarm_Register_At( swarm_Register, swarm_I );
				
				if ( True == swarm->isSwarmTypeToCheckPointAndReload ) {
               char*  swarmFileName     = NULL;
               char*  swarmFileNamePart = NULL;

					Stg_CheckType( swarm->particleLayout, FileParticleLayout );

               swarmFileNamePart = Context_GetCheckPointReadPrefixString( (AbstractContext*)context );
               #ifdef READ_HDF5
                  Stg_asprintf( &swarmFileName, "%s%s.%05d", swarmFileNamePart, swarm->name, context->restartTimestep );
               #else
                  Stg_asprintf( &swarmFileName, "%s%s.%05d.dat", swarmFileNamePart, swarm->name, context->restartTimestep );
               #endif
               
					((FileParticleLayout*)swarm->particleLayout)->filename         = swarmFileName;
					/* set to one incase reading ascii */
					((FileParticleLayout*)swarm->particleLayout)->checkpointfiles = 1;
					/* now check if using hdf5 */ 
					#ifdef READ_HDF5
					((FileParticleLayout*)swarm->particleLayout)->checkpointfiles = _FileParticleLayout_GetFileCountFromTimeInfoFile( context );
				        #endif
					/* Need to re-build & initialise the particles in case the number of particles changed
					due to pop. control */
					Memory_Free( swarm->cellParticleCountTbl );
					Memory_Free( swarm->cellParticleSizeTbl );
					Memory_Free( swarm->cellParticleTbl );
					ExtensionManager_Free( swarm->particleExtensionMgr, swarm->particles );
					_Swarm_BuildParticles( swarm, context );
					_Swarm_InitialiseParticles( swarm, context );

               Memory_Free( swarmFileName );
               Memory_Free( swarmFileNamePart );               
				}
			}	

			/* Re-initialise each fieldVariable - which will automatically load the checkpointed values for
			current timestep as a initial condition */
			for ( fieldVar_I = 0; fieldVar_I < context->fieldVariable_Register->objects->count; fieldVar_I++ ) {
				fieldVar = FieldVariable_Register_GetByIndex( context->fieldVariable_Register, fieldVar_I );

				if ( Stg_Class_IsInstance( fieldVar, FieldVariable_Type ) ) {
					/* Note: will automatically update fieldVariable checkpoint file name
					based on context passed in */
					Stg_Component_Initialise( fieldVar, context, True );
				}
			}	

			/* We allow re-doing of frequent output if user forgot to add all their freq. output
				plugins the first time round */
			if ( context->frequentOutputEvery ) {
				if ( context->timeStep % context->frequentOutputEvery == 0 )
					AbstractContext_FrequentOutput( context );
			}	
			if ( context->dumpEvery ) {
				if ( context->timeStep % context->dumpEvery == 0 )
					AbstractContext_Dump( context );
			}	
		}

		if (context->maxTimeSteps>=0 && (context->timeStepSinceJobRestart >= context->maxTimeSteps)) break;
		if (context->finalTimeStep && (context->timeStep >= context->finalTimeStep)) break;
		if (context->stopTime && (context->currentTime >= context->stopTime)) break; 
		context->timeStep++;
		context->timeStepSinceJobRestart++;
		/* Need to do this to fool the checkpoint loading functions, eg the Swarm's, to load the
		   correct timestep's data */
		context->restartTimestep++;
	}
}


void _lucPlugin_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	Codelet*	self		= (Codelet*)component;
	Dictionary*	pluginDict	= Codelet_GetPluginDictionary( component, cf->rootDict );

	AbstractContext* context;

	context = (AbstractContext*)Stg_ComponentFactory_ConstructByName( cf, Dictionary_GetString( pluginDict, (Dictionary_Entry_Key)"Context"  ), AbstractContext, True, data );
	self->context = context;
	//glucifer_Init();

	if ( Dictionary_GetBool( context->dictionary, "printGluciferComponents" ) ) {
		Journal_Printf( lucInfo, "\nPossible Windows are:\n" );
		PrintChildren( lucWindow_Type, lucInfo );

		Journal_Printf( lucInfo, "\nPossible Drawing Objects are:\n" );
		PrintChildren( lucDrawingObject_Type, lucInfo );
		
		Journal_Printf( lucInfo, "\nPossible Output Formats are:\n" );
		PrintChildren( lucOutputFormat_Type, lucInfo );
		
		Journal_Printf( lucInfo, "\nPossible Input Formats are:\n" );
		PrintChildren( lucInputFormat_Type, lucInfo );
		
		Journal_Printf( lucInfo, "\nPossible Rendering Engines are:\n" );
		PrintChildren( lucRenderingEngine_Type, lucInfo );
		
		Journal_Printf( lucInfo, "\nPossible Window Interactions are:\n" );
		PrintChildren( lucWindowInteraction_Type, lucInfo );
		exit(EXIT_SUCCESS);
	}

	ContextEP_Prepend( context, AbstractContext_EP_DestroyExtensions, glucifer_Finalise );

	/* This is a special option that will re-set the core behaviour of the code to do visualisation
	   only - so assumes you are using --restartTimestep=X to visualise one of your previously calculated
	   results.
	   TODO: assumes the gLucifer plugin is loaded last to a certain extent.
	   TODO: may want an option to only reload certain feVariables needed by visualisation */
	if ( True == Dictionary_GetBool_WithDefault( context->dictionary, (Dictionary_Entry_Key)"visualOnlyOneTimestep", False )  ) {
		Stream* errorStream = Journal_Register( Error_Type, (Name)LucPlugin_Type  );
		Journal_Firewall( context->loadFromCheckPoint == True, errorStream,
			"Error - in %s(): can only use gLucifer visualOnlyOneTimestep option if "
			"loadFromCheckPoint mode enabled.\n", __func__ );
		ContextEP_Purge( context, AbstractContext_EP_Execute );
	}	
	if ( True == Dictionary_GetBool_WithDefault( context->dictionary, (Dictionary_Entry_Key)"visualOnly", False )  ) {
		ContextEP_ReplaceAll( context, AbstractContext_EP_Execute, lucPlugin_VisualOnlyRun );
		
	}	
}


void* _lucPlugin_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof( Codelet );
	Type                                                      type = LucPlugin_Type;
	Stg_Class_DeleteFunction*                              _delete = _Codelet_Delete;
	Stg_Class_PrintFunction*                                _print = _Codelet_Print;
	Stg_Class_CopyFunction*                                  _copy = _Codelet_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucPlugin_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _lucPlugin_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Codelet_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Codelet_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _Codelet_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _Codelet_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _Codelet_New(  CODELET_PASSARGS  );
}


Index lucPlugin_Register( PluginsManager* pluginsManager ) {
	Index result;

	glucifer_Init();
	result = PluginsManager_Submit( pluginsManager, LucPlugin_Type, (Name)"0", _lucPlugin_DefaultNew  );

	return result;
}


