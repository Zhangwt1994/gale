/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 
110 Victoria Street, Melbourne, 3053, Australia.
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
** $Id: Comm.c 3952 2007-01-09 06:24:06Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <assert.h>
#include <mpi.h>
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "types.h"
#include "Comm.h"
#include "StGermain/Base/Foundation/ClassDef.h"


void _Comm_Init( void* _self ) {
   Comm* self = Class_Cast( _self, Comm );

   _NewClass_Init( self );
   self->mpiComm = MPI_COMM_WORLD;
   IArray_Construct( &self->nbrs );
   IMap_Construct( &self->inv );
   self->recvs = NULL;
   self->sends = NULL;
   self->stats = NULL;
   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSize = 0;
   self->srcSizes = NULL;
}

void _Comm_Destruct( void* _self ) {
   Comm* self = Class_Cast( _self, Comm );

   IArray_Destruct( &self->nbrs );
   IMap_Destruct( &self->inv );
   Class_Free( self, self->recvs );
   Class_Free( self, self->sends );
   Class_Free( self, self->stats );
   _NewClass_Destruct( self );
}

void _Comm_Copy( void* _self, const void* _op ) {
   Comm* self = Class_Cast( _self, Comm );
   const Comm* op = Class_ConstCast( _op, Comm );
   int nNbrs;

   _NewClass_Copy( self, op );
   self->mpiComm = op->mpiComm;
   IArray_Copy( &self->nbrs, &op->nbrs );
   IMap_Copy( &self->inv, &op->inv );
   nNbrs = IArray_GetSize( &self->nbrs );
   self->recvs = Class_Rearray( self, self->recvs, MPI_Request, nNbrs );
   self->sends = Class_Rearray( self, self->sends, MPI_Request, nNbrs );
   self->stats = Class_Rearray( self, self->stats, MPI_Status, nNbrs );
}

SizeT _Comm_CalcMem( const void* _self, PtrMap* ptrs ) {
   const Comm* self = Class_ConstCast( _self, Comm );
   SizeT mem;

   if( PtrMap_Find( ptrs, (void*)self ) )
      return 0;
   mem = _NewClass_CalcMem( self, ptrs );
   mem += NewClass_CalcMem( &self->nbrs, ptrs );
   mem += NewClass_CalcMem( &self->inv, ptrs );
   return mem;
}

void Comm_SetMPIComm( void* _self, MPI_Comm mpiComm ) {
   Comm* self = Class_Cast( _self, Comm );

   self->mpiComm = mpiComm;
   IArray_Set( &self->nbrs, 0, NULL );
   IMap_Clear( &self->inv );
   Class_Free( self, self->recvs );
   self->recvs = NULL;
   Class_Free( self, self->sends );
   self->sends = NULL;
   Class_Free( self, self->stats );
   self->stats = NULL;
}

void Comm_SetNeighbours( void* _self, int nNbrs, const int* nbrs ) {
   Comm* self = Class_Cast( _self, Comm );
   int n_i;

   IArray_Set( &self->nbrs, nNbrs, nbrs );
   IMap_Clear( &self->inv );
   IMap_SetMaxSize( &self->inv, nNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      IMap_Insert( &self->inv, nbrs[n_i], n_i );
   self->recvs = Class_Rearray( self, self->recvs, MPI_Request, nNbrs );
   self->sends = Class_Rearray( self, self->sends, MPI_Request, nNbrs );
   self->stats = Class_Rearray( self, self->stats, MPI_Status, nNbrs );
}

void Comm_AddNeighbours( void* _self, int nNbrs, const int* nbrs ) {
   Comm* self = Class_Cast( _self, Comm );
   int netNbrs;
   int n_i;

   IArray_Add( &self->nbrs, nNbrs, nbrs );
   netNbrs = IArray_GetSize( &self->nbrs );
   IMap_SetMaxSize( &self->inv, netNbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      IMap_Insert( &self->inv, nbrs[n_i], netNbrs + n_i );
   self->recvs = Class_Rearray( self, self->recvs, MPI_Request, netNbrs );
   self->sends = Class_Rearray( self, self->sends, MPI_Request, netNbrs );
   self->stats = Class_Rearray( self, self->stats, MPI_Status, netNbrs );
}


/** Note: nbrLocalIndices is _not_ the ranks of neighbours to remove, but local indices of local
 * neighbours in the nbrs array */
void Comm_RemoveNeighbours( void* _self, int nNbrs, const int* nbrLocalIndices, IMap* map ) {
   Comm* self = Class_Cast( _self, Comm );
   int netNbrs;
   int n_i;
   int globalIndex;

   /* First, we need to map the local index to a global, so we know what to remove from the inverse
    * mapping array next */
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      globalIndex = Comm_RankLocalToGlobal( self, nbrLocalIndices[n_i] );
      IMap_Remove( &self->inv, globalIndex );
   }
   /* Now remove the local to global listings */
   IArray_Remove( &self->nbrs, nNbrs, nbrLocalIndices, map );
   /* Finally, update sizes */
   netNbrs = IArray_GetSize( &self->nbrs );
   IMap_SetMaxSize( &self->inv, netNbrs );
   self->recvs = Class_Rearray( self, self->recvs, MPI_Request, netNbrs );
   self->sends = Class_Rearray( self, self->sends, MPI_Request, netNbrs );
   self->stats = Class_Rearray( self, self->stats, MPI_Status, netNbrs );
}

MPI_Comm Comm_GetMPIComm( const void* self ) {
   assert( Class_IsSuper( self, Comm ) );
   return ((Comm*)self)->mpiComm;
}

int Comm_GetNumNeighbours( const void* self ) {
   assert( Class_IsSuper( self, Comm ) );
   return IArray_GetSize( &((Comm*)self)->nbrs );
}

void Comm_GetNeighbours( const void* self, int* nNbrs, const int** nbrs ) {
   assert( Class_IsSuper( self, Comm ) );
   IArray_GetArray( &((Comm*)self)->nbrs, nNbrs, nbrs );
}

int Comm_RankLocalToGlobal( const void* self, int local ) {
   assert( Class_IsSuper( self, Comm ) );
   assert( local < IArray_GetSize( &((Comm*)self)->nbrs ) );
   return IArray_GetPtr( &((Comm*)self)->nbrs )[local];
}

Bool Comm_RankGlobalToLocal( const void* self, int global, int* local ) {
   assert( Class_IsSuper( self, Comm ) );
   return IMap_TryMap( &((Comm*)self)->inv, global, local );
}

void Comm_AllgatherInit( const void* _self, int srcSize, 
			 int* dstSizes, int itmSize )
{
   Comm* self = Class_Cast( _self, Comm );
   const int sizeTag = 1001;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( itmSize );
   assert( !self->srcSize && !self->recvSizes );

   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstSizes + n_i, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( &srcSize, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = dstSizes;
   self->itmSize = itmSize;
   self->srcSize = srcSize;
}

void Comm_AllgatherBegin( const void* _self, const void* srcArray, 
			  void** dstArrays )
{
   Comm* self = Class_Cast( _self, Comm );
   const int dataTag = 2002;
   int nNbrs;
   const int* nbrs;
   int n_i;

   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstArrays[n_i], self->recvSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)srcArray, self->srcSize * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
}

void Comm_AllgatherEnd( const void* _self ) {
   Comm* self = Class_Cast( _self, Comm );
   int nNbrs;
   int n_i;

   nNbrs = IArray_GetSize( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSize = 0;
}

void Comm_AlltoallInit( const void* _self, const int* srcSizes, 
			int* dstSizes, int itmSize )
{
   Comm* self = (Comm*)_self;
   const int sizeTag = 1001;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( self && itmSize );
   assert( !self->srcSizes && !self->recvSizes );
   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstSizes + n_i, 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)(srcSizes + n_i), 1, MPI_INT, nbrs[n_i], sizeTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = dstSizes;
   self->itmSize = itmSize;
   self->srcSizes = (int*)srcSizes;
}

void Comm_AlltoallBegin( const void* _self, const void** srcArrays, 
			 void** dstArrays )
{
   Comm* self = (Comm*)_self;
   const int dataTag = 2002;
   int nNbrs;
   const int* nbrs;
   int n_i;

   assert( self );
   nNbrs = IArray_GetSize( &self->nbrs );
   nbrs = IArray_GetPtr( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Irecv( dstArrays[n_i], self->recvSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->recvs + n_i ), == MPI_SUCCESS );
   }
   for( n_i = 0; n_i < nNbrs; n_i++ ) {
      insist( MPI_Isend( (void*)(srcArrays[n_i]), self->srcSizes[n_i] * self->itmSize, 
			 MPI_BYTE, nbrs[n_i], dataTag, 
			 self->mpiComm, self->sends + n_i ), == MPI_SUCCESS );
   }
}

void Comm_AlltoallEnd( const void* _self ) {
   Comm* self = (Comm*)_self;
   int nNbrs;
   int n_i;

   assert( self );
   nNbrs = IArray_GetSize( &self->nbrs );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->sends + n_i, self->stats + n_i ), == MPI_SUCCESS );
   for( n_i = 0; n_i < nNbrs; n_i++ )
      insist( MPI_Wait( self->recvs + n_i, self->stats + n_i ), == MPI_SUCCESS );

   self->recvSizes = NULL;
   self->itmSize = 0;
   self->srcSizes = NULL;
}


