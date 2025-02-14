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
*/
/** \file
** <b>Role:</b>
**	External header file to this library.
**
** <b>Assumptions:</b>
**	None.
**
** <b>Comments:</b>
**	None.
**
** $Id: IO.h 4259 2008-04-17 12:26:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Base_IO_h__
#define __StGermain_Base_IO_h__
	
	#include "types.h"
	#include "shortcuts.h"
	#include "Dictionary.h"
	#include "Dictionary_Entry.h"
	#include "Dictionary_Entry_Value.h"
	#include "IO_Handler.h"
	#include "XML_IO_Handler.h"
	#include "Init.h"
	#include "Journal.h"
	#include "JournalFile.h"
	#include "CFile.h"
	#include "MPIFile.h"
	#include "Stream.h"
	#include "CStream.h"
	#include "MPIStream.h"
	#include "BinaryStream.h"
	#include "StreamFormatter.h"
	#include "LineFormatter.h"
	#include "IndentFormatter.h"
	#include "RankFormatter.h"
	#include "PathUtils.h"
	#include "CmdLineArgs.h"
	#include "Finalise.h"

	#include "StGermain/Base/IO/mpirecord/mpimessaging.h"

#endif /* __StGermain_Base_IO_h__ */

