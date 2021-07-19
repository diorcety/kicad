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

#ifndef _EXPORT_VRML_
#define _EXPORT_VRML_

#include <wx/string.h>
#include "class_board.h"
#include "project.h"

/**
 * Wrapper to expose an API for writing VRML files
 */

class VRML_WRITER
{
public:

    /**
     * Function ExportVRML_File
     * Creates the file(s) exporting current BOARD to a VRML file.
     *
     * @note When copying 3D shapes files, the new filename is build from the full path
     *       name, changing the separators by underscore.  This is needed because files
     *       with the same shortname can exist in different directories
     * @note ExportVRML_File generates coordinates in board units (BIU) inside the file.
     * @todo Use mm inside the file.  A general scale transform is applied to the whole
     *       file (1.0 to have the actual WRML unit im mm, 0.001 to have the actual WRML
     *       unit in meters.
     * @note For 3D models built by a 3D modeler, the unit is 0,1 inches.  A specific scale
     *       is applied to 3D models to convert them to internal units.
     *
     * @param aFullFileName = the full filename of the file to create
     * @param aMMtoWRMLunit = the VRML scaling factor:
     *      1.0 to export in mm. 0.001 for meters
     * @param aExport3DFiles = true to copy 3D shapes in the subir a3D_Subdir
     * @param aUseRelativePaths set to true to use relative paths instead of absolute paths
     *                          in the board VRML file URLs.
     * @param aUsePlainPCB set to true to export a board with no copper or silkskreen;
     *                          this is useful for generating a VRML file which can be
     *                          converted to a STEP model.
     * @param a3D_Subdir = sub directory where 3D shapes files are copied.  This is only used
     *                     when aExport3DFiles == true
     * @param aXRef = X value of PCB (0,0) reference point
     * @param aYRef = Y value of PCB (0,0) reference point
     * @return true if Ok.
     */
    static bool ExportVRML_File( PROJECT& aProject, BOARD* aPcb, const wxString& aFullFileName, double aMMtoWRMLunit,
                                      bool aExport3DFiles, bool aUseRelativePaths,
                                      bool aUsePlainPCB, const wxString& a3D_Subdir,
                                      double aXRef, double aYRef );
};

#endif //_EXPORT_VRML_