//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file GlobalStatistics2Access.h
 * @author Bernhard Heep
 */

#ifndef __GLOBALSTATISTICS2_ACCESS_H__
#define __GLOBALSTATISTICS2_ACCESS_H__

#include <omnetpp.h>
#include "GlobalStatistics2.h"

/**
 * Gives access to the GlobalStatistics module.
 */
class GlobalNodesHandlerAccess
{
public:

    /**
     * returns the GlobalStatistics module
     *
     * @return the GlobalStatistics module
     */
    GlobalNodesHandler* get()
    {
        return check_and_cast<GlobalNodesHandler*>(
                simulation.getModuleByPath("globalObserver.globalNodesHandler"));
    }
};

#endif

