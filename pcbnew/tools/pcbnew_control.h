/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014-2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef PCBNEW_CONTROL_H
#define PCBNEW_CONTROL_H

#include <io_mgr.h>
#include <memory>
#include <tools/pcb_tool.h>

namespace KIGFX {
    class ORIGIN_VIEWITEM;
}

class PCB_BASE_FRAME;
class BOARD_ITEM;
/**
 * Class PCBNEW_CONTROL
 *
 * Handles actions that are shared between different frames in pcbnew.
 */

class PCBNEW_CONTROL : public PCB_TOOL
{
public:
    PCBNEW_CONTROL();
    ~PCBNEW_CONTROL();

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason ) override;

    // Display modes
    int ZoneDisplayMode( const TOOL_EVENT& aEvent );
    int TrackDisplayMode( const TOOL_EVENT& aEvent );
    int PadDisplayMode( const TOOL_EVENT& aEvent );
    int ViaDisplayMode( const TOOL_EVENT& aEvent );
    int GraphicDisplayMode( const TOOL_EVENT& aEvent );
    int ModuleEdgeOutlines( const TOOL_EVENT& aEvent );
    int ModuleTextOutlines( const TOOL_EVENT& aEvent );
    int HighContrastMode( const TOOL_EVENT& aEvent );
    int HighContrastInc( const TOOL_EVENT& aEvent );
    int HighContrastDec( const TOOL_EVENT& aEvent );

    // Layer control
    int LayerSwitch( const TOOL_EVENT& aEvent );
    int LayerNext( const TOOL_EVENT& aEvent );
    int LayerPrev( const TOOL_EVENT& aEvent );
    int LayerToggle( const TOOL_EVENT& aEvent );
    int LayerAlphaInc( const TOOL_EVENT& aEvent );
    int LayerAlphaDec( const TOOL_EVENT& aEvent );

    // Grid control
    int GridFast1( const TOOL_EVENT& aEvent );
    int GridFast2( const TOOL_EVENT& aEvent );
    int GridSetOrigin( const TOOL_EVENT& aEvent );
    int GridResetOrigin( const TOOL_EVENT& aEvent );

    // UI-level access (including undo) to setting the grid origin
    static bool SetGridOrigin( KIGFX::VIEW* aView, PCB_BASE_FRAME* aFrame,
                               BOARD_ITEM* originViewItem, const VECTOR2D& aPoint );

    // Low-level access (below undo) to setting the grid origin
    static bool DoSetGridOrigin( KIGFX::VIEW* aView, PCB_BASE_FRAME* aFrame,
                                 BOARD_ITEM* originViewItem, const VECTOR2D& aPoint );

    // Miscellaneous
    int ResetCoords( const TOOL_EVENT& aEvent );
    int SwitchCursor( const TOOL_EVENT& aEvent );
    int SwitchUnits( const TOOL_EVENT& aEvent );
    int DeleteItemCursor( const TOOL_EVENT& aEvent );
    int PasteItemsFromClipboard( const TOOL_EVENT& aEvent );
    int AppendBoardFromFile( const TOOL_EVENT& aEvent );
    int AppendBoard( PLUGIN& pi, wxString& fileName );
    int ShowHotkeyList( const TOOL_EVENT& aEvent );
    int ShowHelp( const TOOL_EVENT& aEvent );
    int ToBeDone( const TOOL_EVENT& aEvent );

    ///> Sets up handlers for various events.
    void setTransitions() override;

private:
    int placeBoardItems( BOARD* aBoard );

    /** add and selec or just select for move/place command a list of board items.
     * @param aItems is the list of items
     * @param aIsNew = true to add items to the current board, false to just select if
     * items are already managed by the current board
     */
    int placeBoardItems( std::vector<BOARD_ITEM*>& aItems, bool aIsNew );

    ///> Pointer to the currently used edit frame.
    PCB_BASE_FRAME* m_frame;

    ///> Grid origin marker.
    std::unique_ptr<KIGFX::ORIGIN_VIEWITEM> m_gridOrigin;

    ///> Applies the legacy canvas grid settings for GAL.
    void updateGrid();
};

#endif
