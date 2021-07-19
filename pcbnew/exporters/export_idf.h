/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 Julian Fellinger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _EXPORT_IDF_
#define _EXPORT_IDF_

#include <wx/string.h>
#include <pcbnew_scripting_helpers.h>

/**
 * Wrapper to expose an API for writing IDF files
 */

class IDF_WRITER
{
public:

    /**
     * Function Export_IDF3
     * Creates an IDF3 compliant BOARD (*.emn) and LIBRARY (*.emp) file.
     *
     * @param aPcb = a pointer to the board to be exported to IDF
     * @param aFullFileName = the full filename of the export file
     * @param aUseThou = set to true if the desired IDF unit is thou (mil)
     * @param aXRef = the board Reference Point in mm, X value
     * @param aYRef = the board Reference Point in mm, Y value
     * @return true if OK
     */
    static bool Export_IDF3( PROJECT& aProject, BOARD* aPcb, const wxString& aFullFileName,
                      bool aUseThou, double aXRef, double aYRef );
};

#endif //_EXPORT_IDF_