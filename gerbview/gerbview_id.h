/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010-2014 Jean-Pierre Charras  jp.charras at wanadoo.fr
 * Copyright (C) 1992-2018 KiCad Developers, see change_log.txt for contributors.
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

#ifndef __GERBVIEW_ID_H__
#define __GERBVIEW_ID_H__

#include <id.h>
#include <pgm_base.h>

/**
 * Command IDs for the printed circuit board editor.
 *
 * Please add IDs that are unique to the gerber file viewer (GerbView) here and not in
 * the global id.h file.  This will prevent the entire project from being rebuilt when
 * adding new commands to the GerbView.
 */

enum gerbview_ids
{
    ID_MAIN_MENUBAR = ID_END_LIST,

    ID_GERBVIEW_SHOW_LIST_DCODES,
    ID_GERBVIEW_LOAD_DRILL_FILE,
    ID_GERBVIEW_LOAD_JOB_FILE,
    ID_GERBVIEW_LOAD_ZIP_ARCHIVE_FILE,
    ID_GERBVIEW_ERASE_ALL,
    ID_GERBVIEW_RELOAD_ALL,
    ID_TOOLBARH_GERBER_SELECT_ACTIVE_DCODE,
    ID_GERBVIEW_SHOW_SOURCE,
    ID_GERBVIEW_EXPORT_TO_PCBNEW,

    ID_MENU_GERBVIEW_SELECT_PREFERED_EDITOR,

    ID_GBR_AUX_TOOLBAR_PCB_CMP_CHOICE,
    ID_GBR_AUX_TOOLBAR_PCB_NET_CHOICE,
    ID_GBR_AUX_TOOLBAR_PCB_APERATTRIBUTES_CHOICE,

    ID_TOOLBARH_GERBVIEW_SELECT_ACTIVE_LAYER,
    ID_GERBVIEW_ERASE_CURR_LAYER,
    ID_GERBVIEW_OPTIONS_SETUP,
    ID_TB_OPTIONS_SHOW_LAYERS_MANAGER_VERTICAL_TOOLBAR,
    ID_TB_OPTIONS_SHOW_DCODES,
    ID_TB_OPTIONS_SHOW_FLASHED_ITEMS_SKETCH,
    ID_TB_OPTIONS_SHOW_LINES_SKETCH,
    ID_TB_OPTIONS_SHOW_POLYGONS_SKETCH,
    ID_TB_OPTIONS_SHOW_NEGATIVE_ITEMS,
    ID_TB_OPTIONS_SHOW_GBR_MODE_0,
    ID_TB_OPTIONS_SHOW_GBR_MODE_1,
    ID_TB_OPTIONS_SHOW_GBR_MODE_2,
    ID_TB_OPTIONS_DIFF_MODE,
    ID_TB_OPTIONS_HIGH_CONTRAST_MODE,
    ID_TB_MEASUREMENT_TOOL,

    // Right click context menu
    ID_HIGHLIGHT_REMOVE_ALL,
    ID_HIGHLIGHT_CMP_ITEMS,
    ID_HIGHLIGHT_NET_ITEMS,
    ID_HIGHLIGHT_APER_ATTRIBUTE_ITEMS,

    // IDs for drill file history (ID_FILEnn is already in use)
    ID_GERBVIEW_DRILL_FILE,
    ID_GERBVIEW_DRILL_FILE1,
    ID_GERBVIEW_DRILL_FILEMAX = ID_GERBVIEW_DRILL_FILE + MAX_FILE_HISTORY_SIZE,

    // IDs for job file history (ID_FILEnn is already in use)
    ID_GERBVIEW_JOB_FILE,
    ID_GERBVIEW_JOB_FILE1,
    ID_GERBVIEW_JOB_FILEMAX = ID_GERBVIEW_JOB_FILE + MAX_FILE_HISTORY_SIZE,

    // IDs for zip file history (ID_FILEnn is already in use)
    ID_GERBVIEW_ZIP_FILE,
    ID_GERBVIEW_ZIP_FILE1,
    ID_GERBVIEW_ZIP_FILEMAX = ID_GERBVIEW_ZIP_FILE + MAX_FILE_HISTORY_SIZE,

    ID_GERBVIEW_SHOW_HELP,

    ID_GERBER_END_LIST
};

#endif  /* __GERBVIEW_IDS_H__  */
