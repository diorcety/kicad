/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2017 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file eeschema/hotkeys.cpp
 */

#include <fctsys.h>
#include <eeschema_id.h>
#include <hotkeys.h>
#include <sch_edit_frame.h>
#include <sch_draw_panel.h>

#include <general.h>
#include <lib_edit_frame.h>
#include <viewlib_frame.h>
#include <class_libentry.h>
#include <sch_junction.h>
#include <sch_line.h>
#include <sch_component.h>
#include <sch_sheet.h>

#include <dialogs/dialog_schematic_find.h>

// Remark: the hotkey message info is used as keyword in hotkey config files and
// as comments in help windows, therefore translated only when displayed
// they are marked _HKI to be extracted by translation tools
// See hotkeys_basic.h for more info


/* How to add a new hotkey:
 * add a new id in the enum hotkey_id_command like MY_NEW_ID_FUNCTION (see
 * hotkeys.h).
 * add a new EDA_HOTKEY entry like:
 *  static EDA_HOTKEY HkMyNewEntry(_HKI("Command Label"), MY_NEW_ID_FUNCTION,
 *                                    default key value);
 * _HKI("Command Label") is the name used in hotkey list display, and the
 * identifier in the hotkey list file
 * MY_NEW_ID_FUNCTION is an equivalent id function used in the switch in
 * OnHotKey() function.
 * default key value is the default hotkey for this command. Can be overridden
 * by the user hotkey list file
 * add the HkMyNewEntry pointer in the schematic_Hotkey_List list or the
 * libEdit_Hotkey_List list or common_Hotkey_List if the same command is
 * added both in Eeschema and libedit)
 * Add the new code in the switch in OnHotKey() function.
 * when the variable itemInEdit is true, an item is currently edited.
 * This can be useful if the new function cannot be executed while an item is
 * currently being edited
 * ( For example, one cannot start a new wire when a component is moving.)
 *
 * Note: If an hotkey is a special key be sure the corresponding wxWidget
 *       keycode (WXK_XXXX) is handled in the hotkey_name_descr
 *       s_Hotkey_Name_List list (see hotkeys_basic.cpp) and see this list
 *       for some ascii keys (space ...)
 *
 *  Key modifier are: GR_KB_CTRL GR_KB_ALT
 */


// Common commands

// Fit on Screen
#if !defined( __WXMAC__ )
static EDA_HOTKEY HkZoomAuto( _HKI( "Fit on Screen" ), HK_ZOOM_AUTO, WXK_HOME, ID_ZOOM_PAGE );
#else
static EDA_HOTKEY HkZoomAuto( _HKI( "Zoom Auto" ), HK_ZOOM_AUTO, GR_KB_CTRL + '0',
                              ID_ZOOM_PAGE );
#endif

static EDA_HOTKEY HkZoomCenter( _HKI( "Zoom Center" ), HK_ZOOM_CENTER, WXK_F4,
                                ID_POPUP_ZOOM_CENTER );

// Refresh Screen
#if !defined( __WXMAC__ )
static EDA_HOTKEY HkZoomRedraw( _HKI( "Zoom Redraw" ), HK_ZOOM_REDRAW, WXK_F3, ID_ZOOM_REDRAW );
#else
static EDA_HOTKEY HkZoomRedraw( _HKI( "Zoom Redraw" ), HK_ZOOM_REDRAW, GR_KB_CTRL + 'R',
                                ID_ZOOM_REDRAW );
#endif

// Zoom In
#if !defined( __WXMAC__ )
static EDA_HOTKEY HkZoomIn( _HKI( "Zoom In" ), HK_ZOOM_IN, WXK_F1, ID_KEY_ZOOM_IN );
#else
static EDA_HOTKEY HkZoomIn( _HKI( "Zoom In" ), HK_ZOOM_IN, GR_KB_CTRL + '+', ID_KEY_ZOOM_IN );
#endif

// Zoom Out
#if !defined( __WXMAC__ )
static EDA_HOTKEY HkZoomOut( _HKI( "Zoom Out" ), HK_ZOOM_OUT, WXK_F2, ID_KEY_ZOOM_OUT );
#else
static EDA_HOTKEY HkZoomOut( _HKI( "Zoom Out" ), HK_ZOOM_OUT, GR_KB_CTRL + '-', ID_KEY_ZOOM_OUT );
#endif

static EDA_HOTKEY HkHelp( _HKI( "List Hotkeys" ), HK_HELP, GR_KB_CTRL + WXK_F1 );
static EDA_HOTKEY HkPreferences( _HKI( "Preferences" ), HK_PREFERENCES, GR_KB_CTRL + ',', (int) wxID_PREFERENCES );
static EDA_HOTKEY HkResetLocalCoord( _HKI( "Reset Local Coordinates" ), HK_RESET_LOCAL_COORD, ' ' );
static EDA_HOTKEY HkLeaveSheet( _HKI( "Leave Sheet" ), HK_LEAVE_SHEET, GR_KB_ALT + WXK_BACK,
                                ID_POPUP_SCH_LEAVE_SHEET );

// mouse click command:
static EDA_HOTKEY HkMouseLeftClick( _HKI( "Mouse Left Click" ), HK_LEFT_CLICK, WXK_RETURN, 0 );
static EDA_HOTKEY HkMouseLeftDClick( _HKI( "Mouse Left Double Click" ), HK_LEFT_DCLICK, WXK_END, 0 );

// Schematic editor
static EDA_HOTKEY HkBeginWire( _HKI( "Begin Wire" ), HK_BEGIN_WIRE, 'W', ID_WIRE_BUTT );
static EDA_HOTKEY HkBeginBus( _HKI( "Begin Bus" ), HK_BEGIN_BUS, 'B', ID_BUS_BUTT );
static EDA_HOTKEY HkEndLineWireBus( _HKI( "End Line Wire Bus" ), HK_END_CURR_LINEWIREBUS, 'K',
                                  ID_POPUP_END_LINE );

static EDA_HOTKEY HkAddLabel( _HKI( "Add Label" ), HK_ADD_LABEL, 'L', ID_LABEL_BUTT );
static EDA_HOTKEY HkAddHierarchicalLabel( _HKI( "Add Hierarchical Label" ), HK_ADD_HLABEL, 'H',
                                          ID_HIERLABEL_BUTT );
static EDA_HOTKEY HkAddGlobalLabel( _HKI( "Add Global Label" ), HK_ADD_GLABEL, GR_KB_CTRL + 'H',
                                    ID_GLABEL_BUTT );
static EDA_HOTKEY HkAddJunction( _HKI( "Add Junction" ), HK_ADD_JUNCTION, 'J', ID_JUNCTION_BUTT );
static EDA_HOTKEY HkAddComponent( _HKI( "Add Symbol" ), HK_ADD_NEW_COMPONENT, 'A',
                                  ID_SCH_PLACE_COMPONENT );
static EDA_HOTKEY HkAddPower( _HKI( "Add Power" ), HK_ADD_NEW_POWER, 'P',
                              ID_PLACE_POWER_BUTT );
static EDA_HOTKEY HkAddNoConn( _HKI( "Add No Connect Flag" ), HK_ADD_NOCONN_FLAG, 'Q',
                               ID_NOCONN_BUTT );
static EDA_HOTKEY HkAddHierSheet( _HKI( "Add Sheet" ), HK_ADD_HIER_SHEET, 'S',
                                  ID_SHEET_SYMBOL_BUTT );
static EDA_HOTKEY HkAddBusEntry( _HKI( "Add Bus Entry" ), HK_ADD_BUS_ENTRY, '/',
                                 ID_BUSTOBUS_ENTRY_BUTT );
static EDA_HOTKEY HkAddWireEntry( _HKI( "Add Wire Entry" ), HK_ADD_WIRE_ENTRY, 'Z',
                                  ID_WIRETOBUS_ENTRY_BUTT );
static EDA_HOTKEY HkAddGraphicPolyLine( _HKI( "Add Graphic PolyLine" ), HK_ADD_GRAPHIC_POLYLINE,
                                        'I', ID_LINE_COMMENT_BUTT );
static EDA_HOTKEY HkAddGraphicText( _HKI( "Add Graphic Text" ), HK_ADD_GRAPHIC_TEXT, 'T',
                                    ID_TEXT_COMMENT_BUTT );
static EDA_HOTKEY HkMirrorY( _HKI( "Mirror Y" ), HK_MIRROR_Y, 'Y',
                             ID_SCH_MIRROR_Y );
static EDA_HOTKEY HkMirrorX( _HKI( "Mirror X" ), HK_MIRROR_X, 'X',
                             ID_SCH_MIRROR_X );
static EDA_HOTKEY HkOrientNormalComponent( _HKI( "Orient Normal Component" ),
                                           HK_ORIENT_NORMAL_COMPONENT, 'N', ID_SCH_ORIENT_NORMAL );
static EDA_HOTKEY HkRotate( _HKI( "Rotate Item" ), HK_ROTATE, 'R', ID_SCH_ROTATE_CLOCKWISE );
static EDA_HOTKEY HkEdit( _HKI( "Edit Item" ), HK_EDIT, 'E', ID_SCH_EDIT_ITEM );
static EDA_HOTKEY HkEditComponentValue( _HKI( "Edit Symbol Value" ),
                                        HK_EDIT_COMPONENT_VALUE, 'V',
                                        ID_SCH_EDIT_COMPONENT_VALUE );
static EDA_HOTKEY HkEditComponentReference( _HKI( "Edit Symbol Reference" ),
                                        HK_EDIT_COMPONENT_REFERENCE, 'U',
                                        ID_SCH_EDIT_COMPONENT_REFERENCE );
static EDA_HOTKEY HkEditComponentFootprint( _HKI( "Edit Symbol Footprint" ),
                                            HK_EDIT_COMPONENT_FOOTPRINT, 'F',
                                            ID_SCH_EDIT_COMPONENT_FOOTPRINT );
static EDA_HOTKEY HkShowComponentDatasheet( _HKI( "Show Symbol Datasheet" ),
                                           HK_SHOW_COMPONENT_DATASHEET, 'D' + GR_KB_CTRL,
                                           ID_POPUP_SCH_DISPLAYDOC_CMP );
static EDA_HOTKEY HkEditComponentWithLibedit( _HKI( "Edit with Symbol Editor" ),
                                              HK_EDIT_COMPONENT_WITH_LIBEDIT, 'E' + GR_KB_CTRL,
                                              ID_POPUP_SCH_CALL_LIBEDIT_AND_LOAD_CMP );

static EDA_HOTKEY HkMove( _HKI( "Move Schematic Item" ),
                          HK_MOVE_COMPONENT_OR_ITEM, 'M',
                          ID_SCH_MOVE_ITEM );

static EDA_HOTKEY HkDuplicateItem( _HKI( "Duplicate Symbol or Label" ),
                                         HK_DUPLICATE_ITEM, 'C',
                                         ID_POPUP_SCH_DUPLICATE_ITEM );

static EDA_HOTKEY HkDrag( _HKI( "Drag Item" ), HK_DRAG, 'G', ID_SCH_DRAG_ITEM );
static EDA_HOTKEY HkMove2Drag( _HKI( "Move Block -> Drag Block" ),
                               HK_MOVEBLOCK_TO_DRAGBLOCK, '\t', ID_POPUP_DRAG_BLOCK );
static EDA_HOTKEY HkInsert( _HKI( "Repeat Last Item" ), HK_REPEAT_LAST, WXK_INSERT );
static EDA_HOTKEY HkDelete( _HKI( "Delete Item" ), HK_DELETE, WXK_DELETE );
static EDA_HOTKEY HkDeleteNode( _HKI( "Delete Node" ), HK_DELETE_NODE, WXK_BACK,
                                ID_POPUP_SCH_DELETE_NODE );

static EDA_HOTKEY HkFindItem( _HKI( "Find Item" ), HK_FIND_ITEM, 'F' + GR_KB_CTRL, ID_FIND_ITEMS );
static EDA_HOTKEY HkFindNextItem( _HKI( "Find Next Item" ), HK_FIND_NEXT_ITEM, WXK_F5,
                                  wxEVT_COMMAND_FIND );
static EDA_HOTKEY HkFindReplace( _HKI( "Find and Replace" ), HK_FIND_REPLACE,
                                 'F' + GR_KB_CTRL + GR_KB_ALT, wxID_REPLACE );
static EDA_HOTKEY HkFindNextDrcMarker( _HKI( "Find Next DRC Marker" ), HK_FIND_NEXT_DRC_MARKER,
                                       WXK_F5 + GR_KB_SHIFT, EVT_COMMAND_FIND_DRC_MARKER );
static EDA_HOTKEY HkZoomSelection( _HKI( "Zoom to Selection" ), HK_ZOOM_SELECTION,
                                   GR_KB_CTRL + WXK_F5, ID_ZOOM_SELECTION );

// Special keys for library editor:
static EDA_HOTKEY HkCreatePin( _HKI( "Create Pin" ), HK_LIBEDIT_CREATE_PIN, 'P' );
static EDA_HOTKEY HkInsertPin( _HKI( "Repeat Pin" ), HK_REPEAT_LAST, WXK_INSERT );
static EDA_HOTKEY HkMoveLibItem( _HKI( "Move Library Item" ), HK_LIBEDIT_MOVE_GRAPHIC_ITEM, 'M' );
static EDA_HOTKEY HkViewDoc( _HKI( "Show Datasheet" ), HK_LIBEDIT_VIEW_DOC, 'D' + GR_KB_CTRL,
                             ID_LIBEDIT_VIEW_DOC );

// Autoplace fields
static EDA_HOTKEY HkAutoplaceFields( _HKI( "Autoplace Fields" ), HK_AUTOPLACE_FIELDS, 'O',
                                        ID_AUTOPLACE_FIELDS );

static EDA_HOTKEY HkUpdatePcbFromSch( _HKI( "Update PCB from Schematic" ), HK_UPDATE_PCB_FROM_SCH,
                                      WXK_F8 );

// Higtlight connection
static EDA_HOTKEY HkHighlightConnection( _HKI( "Highlight Connection" ), ID_HOTKEY_HIGHLIGHT,
                                         'B' + GR_KB_CTRL );

// Common: hotkeys_basic.h
static EDA_HOTKEY HkNew( _HKI( "New" ), HK_NEW, GR_KB_CTRL + 'N', (int) wxID_NEW );
static EDA_HOTKEY HkOpen( _HKI( "Open" ), HK_OPEN, GR_KB_CTRL + 'O', (int) wxID_OPEN );
static EDA_HOTKEY HkSave( _HKI( "Save" ), HK_SAVE, GR_KB_CTRL + 'S', (int) wxID_SAVE );
static EDA_HOTKEY HkSaveAs( _HKI( "Save As" ), HK_SAVEAS, GR_KB_SHIFT + GR_KB_CTRL + 'S',
                            (int) wxID_SAVEAS );
static EDA_HOTKEY HkPrint( _HKI( "Print" ), HK_PRINT, GR_KB_CTRL + 'P', (int) wxID_PRINT );

static EDA_HOTKEY HkUndo( _HKI( "Undo" ), HK_UNDO, GR_KB_CTRL + 'Z', (int) wxID_UNDO );

#if !defined( __WXMAC__ )
static EDA_HOTKEY HkRedo( _HKI( "Redo" ), HK_REDO, GR_KB_CTRL + 'Y', (int) wxID_REDO );
#else
static EDA_HOTKEY HkRedo( _HKI( "Redo" ), HK_REDO,
                       GR_KB_SHIFT + GR_KB_CTRL + 'Z',
                       (int) wxID_REDO );
#endif

static EDA_HOTKEY HkEditCut( _HKI( "Cut" ), HK_EDIT_CUT, GR_KB_CTRL + 'X', (int) wxID_CUT );
static EDA_HOTKEY HkEditCopy( _HKI( "Copy" ), HK_EDIT_COPY, GR_KB_CTRL + 'C', (int) wxID_COPY );
static EDA_HOTKEY HkEditPaste( _HKI( "Paste" ), HK_EDIT_PASTE, GR_KB_CTRL + 'V', (int) wxID_PASTE );

static EDA_HOTKEY HkCanvasOpenGL( _HKI( "Switch to Modern Toolset with hardware-accelerated graphics (recommended)" ),
                                  HK_CANVAS_OPENGL,
#ifdef __WXMAC__
                                  GR_KB_ALT +
#endif
                                  WXK_F11, ID_MENU_CANVAS_OPENGL );
static EDA_HOTKEY HkCanvasCairo( _HKI( "Switch to Modern Toolset with software graphics (fall-back)" ),
                                 HK_CANVAS_CAIRO,
#ifdef __WXMAC__
                                 GR_KB_ALT +
#endif
                                 WXK_F12, ID_MENU_CANVAS_CAIRO );

// List of common hotkey descriptors
static EDA_HOTKEY* common_Hotkey_List[] =
{
    &HkNew,         &HkOpen,            &HkSave,          &HkSaveAs,        &HkPrint,
    &HkUndo,        &HkRedo,
    &HkEditCut,     &HkEditCopy,        &HkEditPaste,
    &HkHelp,
    &HkPreferences,
    &HkZoomIn,
    &HkZoomOut,
    &HkZoomRedraw,
    &HkZoomCenter,
    &HkZoomAuto,
    &HkZoomSelection,
    &HkResetLocalCoord,
    &HkEdit,
    &HkDelete,
    &HkRotate,
    &HkDrag,
    &HkMouseLeftClick,
    &HkMouseLeftDClick,
    NULL
};

// List of common hotkey descriptors, for the library vierwer
static EDA_HOTKEY* common_basic_Hotkey_List[] =
{
    &HkHelp,
    &HkZoomIn,
    &HkZoomOut,
    &HkZoomRedraw,
    &HkZoomCenter,
    &HkZoomAuto,
    &HkResetLocalCoord,
    &HkMouseLeftClick,
    &HkMouseLeftDClick,
    NULL
};

// List of hotkey descriptors for schematic
static EDA_HOTKEY* schematic_Hotkey_List[] =
{
    &HkFindItem,
    &HkFindNextItem,
    &HkFindNextDrcMarker,
    &HkFindReplace,
    &HkInsert,
    &HkMove2Drag,
    &HkMove,
    &HkDuplicateItem,
    &HkAddComponent,
    &HkAddPower,
    &HkMirrorX,
    &HkMirrorY,
    &HkOrientNormalComponent,
    &HkEditComponentValue,
    &HkEditComponentReference,
    &HkEditComponentFootprint,
    &HkShowComponentDatasheet,
    &HkEditComponentWithLibedit,
    &HkBeginWire,
    &HkBeginBus,
    &HkEndLineWireBus,
    &HkAddLabel,
    &HkAddHierarchicalLabel,
    &HkAddGlobalLabel,
    &HkAddJunction,
    &HkAddNoConn,
    &HkAddHierSheet,
    &HkAddWireEntry,
    &HkAddBusEntry,
    &HkAddGraphicPolyLine,
    &HkAddGraphicText,
    &HkUpdatePcbFromSch,
    &HkAutoplaceFields,
    &HkLeaveSheet,
    &HkDeleteNode,
    &HkHighlightConnection,
    &HkCanvasCairo,
    &HkCanvasOpenGL,
    NULL
};

// List of hotkey descriptors for library editor
static EDA_HOTKEY* libEdit_Hotkey_List[] =
{
    &HkCreatePin,
    &HkInsertPin,
    &HkMoveLibItem,
    &HkMirrorX,
    &HkMirrorY,
    &HkViewDoc,
    NULL
};

// List of hotkey descriptors for library viewer (currently empty
static EDA_HOTKEY* viewlib_Hotkey_List[] =
{
    NULL
};

// Keyword Identifiers (tags) in key code configuration file (section names)
// (.m_SectionTag member of a EDA_HOTKEY_CONFIG)
static wxString schematicSectionTag( wxT( "[eeschema]" ) );
static wxString libEditSectionTag( wxT( "[libedit]" ) );

// Titles for hotkey editor and hotkey display
static wxString commonSectionTitle( _HKI( "Common" ) );
static wxString schematicSectionTitle( _HKI( "Schematic Editor" ) );
static wxString libEditSectionTitle( _HKI( "Library Editor" ) );

// list of sections and corresponding hotkey list for Eeschema (used to create
// an hotkey config file)
struct EDA_HOTKEY_CONFIG g_Eeschema_Hotkeys_Descr[] =
{
    { &g_CommonSectionTag,    common_Hotkey_List,    &commonSectionTitle    },
    { &schematicSectionTag,   schematic_Hotkey_List, &schematicSectionTitle },
    { &libEditSectionTag,     libEdit_Hotkey_List,   &libEditSectionTitle   },
    { NULL,                   NULL,                  NULL                   }
};

// list of sections and corresponding hotkey list for the schematic editor
// (used to list current hotkeys)
struct EDA_HOTKEY_CONFIG g_Schematic_Hotkeys_Descr[] =
{
    { &g_CommonSectionTag,    common_Hotkey_List,    &commonSectionTitle    },
    { &schematicSectionTag,   schematic_Hotkey_List, &schematicSectionTitle },
    { NULL,                   NULL,                  NULL }
};

// list of sections and corresponding hotkey list for the component editor
// (used to list current hotkeys)
struct EDA_HOTKEY_CONFIG g_Libedit_Hotkeys_Descr[] =
{
    { &g_CommonSectionTag,  common_Hotkey_List,   &commonSectionTitle  },
    { &libEditSectionTag,   libEdit_Hotkey_List,  &libEditSectionTitle },
    { NULL,                 NULL,                 NULL }
};

// list of sections and corresponding hotkey list for the component browser
// (used to list current hotkeys)
struct EDA_HOTKEY_CONFIG g_Viewlib_Hotkeys_Descr[] =
{
    { &g_CommonSectionTag, common_basic_Hotkey_List, &commonSectionTitle },
    { NULL,                NULL,                 NULL }
};


EDA_HOTKEY* SCH_EDIT_FRAME::GetHotKeyDescription( int aCommand ) const
{
    EDA_HOTKEY* HK_Descr = GetDescriptorFromCommand( aCommand, common_Hotkey_List );

    if( HK_Descr == NULL )
        HK_Descr = GetDescriptorFromCommand( aCommand, schematic_Hotkey_List );

    return HK_Descr;
}


/*
 * Hot keys. Some commands are relative to the item under the mouse cursor
 * Commands are case insensitive
 */
bool SCH_EDIT_FRAME::OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem )
{
    if( aHotKey == 0 )
        return false;

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );

    SCH_SCREEN* screen = GetScreen();

    // itemInEdit == false means no item currently edited. We can ask for editing a new item
    bool itemInEdit = screen->GetCurItem() && screen->GetCurItem()->GetFlags();

    // blocInProgress == false means no block in progress.
    // Because a drag command uses a drag block, false means also no drag in progress
    // If false, we can ask for editing a new item
    bool blocInProgress = screen->m_BlockLocate.GetState() != STATE_NO_BLOCK;

    // notBusy == true means no item currently edited and no other command in progress
    // We can change active tool and ask for editing a new item
    bool notBusy = (!itemInEdit) && (!blocInProgress);

    /* Convert lower to upper case (the usual toupper function has problem
     * with non ascii codes like function keys */
    if( (aHotKey >= 'a') && (aHotKey <= 'z') )
        aHotKey += 'A' - 'a';

    // Search command from key :
    EDA_HOTKEY* hotKey = GetDescriptorFromHotkey( aHotKey, common_Hotkey_List );

    if( hotKey == NULL )
        hotKey = GetDescriptorFromHotkey( aHotKey, schematic_Hotkey_List );

    if( hotKey == NULL )
        return false;

    switch( hotKey->m_Idcommand )
    {
    default:
    case HK_NOT_FOUND:
        return false;

    case HK_HELP:       // Display Current hotkey list
        DisplayHotkeyList( this, g_Schematic_Hotkeys_Descr );
        break;

    case HK_PREFERENCES:
        cmd.SetId( wxID_PREFERENCES );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_RESET_LOCAL_COORD:         // Reset the relative coord
        GetScreen()->m_O_Curseur = GetCrossHairPosition();
        break;

    case ID_HOTKEY_HIGHLIGHT:
        if( notBusy )
            HighlightConnectionAtPosition( GetCrossHairPosition() );
        break;

    case HK_LEFT_CLICK:
    case HK_LEFT_DCLICK:    // Simulate a double left click: generate 2 events
        if( screen->m_BlockLocate.GetState() == STATE_BLOCK_MOVE )
        {
            GetCanvas()->SetAutoPanRequest( false );
            HandleBlockPlace( aDC );
        }
        else if( screen->m_BlockLocate.GetState() == STATE_NO_BLOCK )
        {
            auto pos = GetCrossHairPosition();
            OnLeftClick( aDC, pos );

            if( hotKey->m_Idcommand == HK_LEFT_DCLICK )
                OnLeftDClick( aDC, pos );
        }
        break;

    case HK_ZOOM_IN:
    case HK_ZOOM_OUT:
    case HK_ZOOM_REDRAW:
    case HK_ZOOM_CENTER:
    case HK_ZOOM_AUTO:
    case HK_ZOOM_SELECTION:
    case HK_MOVEBLOCK_TO_DRAGBLOCK:          // Switch to drag mode, when block moving
    case HK_EDIT_PASTE:
    case HK_EDIT_COPY:                      // Copy block to paste buffer.
    case HK_EDIT_CUT:
        cmd.SetId( hotKey->m_IdMenuEvent );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_DELETE:
        if( blocInProgress )
        {
            cmd.SetId( ID_POPUP_DELETE_BLOCK );
            GetEventHandler()->ProcessEvent( cmd );
        }
        else if( notBusy )
            DeleteItemAtCrossHair();
        break;

    case HK_REPEAT_LAST:
        if( notBusy )
            RepeatDrawItem();
        break;

    case HK_END_CURR_LINEWIREBUS:
        // this key terminates a new line/bus/wire in progress
        if( aItem && aItem->IsNew() &&
            aItem->Type() == SCH_LINE_T )
        {
            cmd.SetId( hotKey->m_IdMenuEvent );
            GetEventHandler()->ProcessEvent( cmd );
        }
        break;

    case HK_UNDO:             // Hot keys that map to command IDs that cannot be called
    case HK_REDO:             // while busy performing another command.
    case HK_FIND_ITEM:
    case HK_FIND_REPLACE:
    case HK_DELETE_NODE:
    case HK_LEAVE_SHEET:
        if( notBusy )
        {
            cmd.SetId( hotKey->m_IdMenuEvent );
            GetEventHandler()->ProcessEvent( cmd );
        }
        break;

    case HK_FIND_NEXT_ITEM:
    case HK_FIND_NEXT_DRC_MARKER:
        if( notBusy )
        {
            wxFindDialogEvent event( hotKey->m_IdMenuEvent, GetId() );
            event.SetEventObject( this );
            event.SetFlags( m_findReplaceData->GetFlags() );
            event.SetFindString( m_findReplaceData->GetFindString() );
            GetEventHandler()->ProcessEvent( event );
        }
        break;

    case HK_ADD_NEW_COMPONENT:      // Add component
    case HK_ADD_NEW_POWER:          // Add power component
    case HK_ADD_LABEL:
    case HK_ADD_HLABEL:
    case HK_ADD_GLABEL:
    case HK_ADD_JUNCTION:
    case HK_ADD_WIRE_ENTRY:
    case HK_ADD_BUS_ENTRY:
    case HK_ADD_HIER_SHEET:
    case HK_ADD_GRAPHIC_TEXT:
    case HK_ADD_GRAPHIC_POLYLINE:
    case HK_ADD_NOCONN_FLAG:        // Add a no connected flag
    case HK_BEGIN_BUS:
    case HK_BEGIN_WIRE:
        if( notBusy )
        {
            EDA_HOTKEY_CLIENT_DATA data( aPosition );
            cmd.SetInt( aHotKey );
            cmd.SetClientObject( &data );
            cmd.SetId( hotKey->m_IdMenuEvent );
            GetEventHandler()->ProcessEvent( cmd );
        }
        else if( aItem && aItem->IsNew() )
        {
            // If the item is a bus or a wire, a begin command is not possible.
            if( (GetToolId() == ID_BUS_BUTT) && (aItem->Type() == SCH_LINE_T) )
            {
                SCH_LINE* segment = (SCH_LINE*) aItem;

                if( segment->GetLayer() != LAYER_BUS )
                    break;

                // Bus in progress:
                OnLeftClick( aDC, aPosition );
            }
            else if( (GetToolId() == ID_WIRE_BUTT ) && (aItem->Type() == SCH_LINE_T) )
            {
                SCH_LINE* segment = (SCH_LINE*) aItem;

                if( segment->GetLayer() != LAYER_WIRE )
                    break;

                // Wire in progress:
                OnLeftClick( aDC, aPosition );
            }
        }
        break;

    case HK_DUPLICATE_ITEM:        // Duplicate component or text/label
        if( itemInEdit )
            break;

        if( aItem == NULL )
        {
            aItem = LocateAndShowItem( aPosition, SCH_COLLECTOR::CopyableItems );

            if( aItem == NULL )
                break;
        }

        cmd.SetId( hotKey->m_IdMenuEvent );
        wxPostEvent( this, cmd );
        break;

    case HK_DRAG:                           // Start drag
    case HK_MOVE_COMPONENT_OR_ITEM:         // Start move schematic item.
        if( ! notBusy )
            break;

        // Fall through
    case HK_EDIT:
        // Edit schematic item. Do not allow sheet editing when mowing because sheet editing
        // can be complex.
        if( itemInEdit && screen->GetCurItem()->Type() == SCH_SHEET_T )
                break;

        // Fall through
    case HK_EDIT_COMPONENT_VALUE:           // Edit component value field.
    case HK_EDIT_COMPONENT_REFERENCE:       // Edit component value reference.
    case HK_EDIT_COMPONENT_FOOTPRINT:       // Edit component footprint field.
    case HK_SHOW_COMPONENT_DATASHEET:       // Show component datasheet in browser.
    case HK_MIRROR_Y:                       // Mirror Y
    case HK_MIRROR_X:                       // Mirror X
    case HK_ORIENT_NORMAL_COMPONENT:        // Orient 0, no mirror (Component)
    case HK_ROTATE:                         // Rotate schematic item.
    case HK_EDIT_COMPONENT_WITH_LIBEDIT:    // Call Libedit and load the current component
    case HK_AUTOPLACE_FIELDS:               // Autoplace all fields around component
    case HK_CANVAS_CAIRO:
    case HK_CANVAS_OPENGL:
        {
           // force a new item search on hot keys at current position,
            // if there is no currently edited item,
            // to avoid using a previously selected item
            if( ! itemInEdit )
                screen->SetCurItem( NULL );

            EDA_HOTKEY_CLIENT_DATA data( aPosition );
            cmd.SetInt( hotKey->m_Idcommand );
            cmd.SetClientObject( &data );
            cmd.SetId( hotKey->m_IdMenuEvent );
            GetEventHandler()->ProcessEvent( cmd );
        }
        break;
    }

    // Hot key handled.
    return true;
}


EDA_HOTKEY* LIB_EDIT_FRAME::GetHotKeyDescription( int aCommand ) const
{
    EDA_HOTKEY* HK_Descr = GetDescriptorFromCommand( aCommand, common_Hotkey_List );

    if( HK_Descr == NULL )
        HK_Descr = GetDescriptorFromCommand( aCommand, libEdit_Hotkey_List );

    return HK_Descr;
}


bool LIB_EDIT_FRAME::OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem )
{
    if( aHotKey == 0 )
        return false;

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );

    cmd.SetEventObject( this );

    /* Convert lower to upper case (the usual toupper function has problem
     * with non ascii codes like function keys */
    if( (aHotKey >= 'a') && (aHotKey <= 'z') )
        aHotKey += 'A' - 'a';

    EDA_HOTKEY* hotKey = GetDescriptorFromHotkey( aHotKey, common_Hotkey_List );

    if( hotKey == NULL )
        hotKey = GetDescriptorFromHotkey( aHotKey, libEdit_Hotkey_List );

    if( hotKey == NULL )
        return false;

    // itemInEdit == false means no item currently edited. We can ask for editing a new item
    bool itemInEdit = IsEditingDrawItem();

    bool blocInProgress = GetScreen()->m_BlockLocate.GetState() != STATE_NO_BLOCK;

    switch( hotKey->m_Idcommand )
    {
    default:
    case HK_NOT_FOUND:
        return false;

    case HK_HELP:       // Display Current hotkey list
        DisplayHotkeyList( this, g_Libedit_Hotkeys_Descr );
        break;

    case HK_PREFERENCES:
        cmd.SetId( wxID_PREFERENCES );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_RESET_LOCAL_COORD:         // Reset the relative coord
        GetScreen()->m_O_Curseur = GetCrossHairPosition();
        break;

    case HK_ZOOM_IN:
    case HK_ZOOM_OUT:
    case HK_ZOOM_REDRAW:
    case HK_ZOOM_CENTER:
    case HK_ZOOM_AUTO:
    case HK_EDIT_PASTE:
    case HK_EDIT_COPY:
    case HK_EDIT_CUT:
        cmd.SetId( hotKey->m_IdMenuEvent );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_UNDO:
    case HK_REDO:
        if( !itemInEdit )
        {
            cmd.SetId( hotKey->m_IdMenuEvent );
            GetEventHandler()->ProcessEvent( cmd );
        }
        break;

    case HK_REPEAT_LAST:
        if( ! itemInEdit )
        {
            if( m_lastDrawItem && !m_lastDrawItem->InEditMode() &&
                ( m_lastDrawItem->Type() == LIB_PIN_T ) )
                RepeatPinItem( aDC, (LIB_PIN*) m_lastDrawItem );
        }
        break;

    case HK_EDIT:
        if ( !itemInEdit )
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

        if( GetDrawItem() )
        {
            switch( GetDrawItem()->Type() )
            {
            case LIB_PIN_T:
                cmd.SetId( ID_LIBEDIT_EDIT_PIN );
                GetEventHandler()->ProcessEvent( cmd );
                break;

            case LIB_ARC_T:
            case LIB_CIRCLE_T:
            case LIB_RECTANGLE_T:
            case LIB_POLYLINE_T:
            case LIB_TEXT_T:
                cmd.SetId( ID_POPUP_LIBEDIT_BODY_EDIT_ITEM );
                GetEventHandler()->ProcessEvent( cmd );
                break;

            case LIB_FIELD_T:
                cmd.SetId( ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM );
                GetEventHandler()->ProcessEvent( cmd );
                break;

            default:
                break;
            }
        }
        break;

    case HK_ROTATE:
        if ( !itemInEdit && !blocInProgress )
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

        if( blocInProgress || GetDrawItem() )
        {
            cmd.SetId( ID_LIBEDIT_ROTATE_ITEM );
            OnRotate( cmd );
        }
        break;

    case HK_LIBEDIT_CREATE_PIN:
        if( ! itemInEdit )
        {
            SetToolID( ID_LIBEDIT_PIN_BUTT, wxCURSOR_PENCIL, _( "Add Pin" ) );
            OnLeftClick( aDC, aPosition );
        }
        break;

    case HK_DELETE:
        if( blocInProgress )
        {
            cmd.SetId( ID_POPUP_DELETE_BLOCK );
            Process_Special_Functions( cmd );
        }
        else
        {
            if( !itemInEdit )
                SetDrawItem( LocateItemUsingCursor( aPosition ) );

            if( GetDrawItem() )
            {
                cmd.SetId( ID_POPUP_LIBEDIT_DELETE_ITEM );
                Process_Special_Functions( cmd );
            }
        }
        break;

    case HK_LIBEDIT_MOVE_GRAPHIC_ITEM:
        if( !itemInEdit && !blocInProgress )
        {
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

            if( GetDrawItem() )
            {
                cmd.SetId( ID_POPUP_LIBEDIT_MOVE_ITEM_REQUEST );
                Process_Special_Functions( cmd );
            }
        }
        break;

    case HK_DRAG:
        if( !itemInEdit && !blocInProgress  )
        {
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

            if( GetDrawItem() )
            {
                cmd.SetId( ID_POPUP_LIBEDIT_MODIFY_ITEM );
                Process_Special_Functions( cmd );
            }
        }
        break;

    case HK_MIRROR_Y:                       // Mirror Y
        if( !itemInEdit && !blocInProgress )
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

        if( blocInProgress || GetDrawItem() )
        {
            cmd.SetId( ID_LIBEDIT_MIRROR_Y );
            OnOrient( cmd );
        }
        break;

    case HK_MIRROR_X:                       // Mirror X
        if( !itemInEdit && !blocInProgress )
            SetDrawItem( LocateItemUsingCursor( aPosition ) );

        if( blocInProgress || GetDrawItem() )
        {
            cmd.SetId( ID_LIBEDIT_MIRROR_X );
            OnOrient( cmd );
        }
        break;

    case HK_LEFT_CLICK:
    case HK_LEFT_DCLICK:    // Simulate a double left click: generate 2 events
        if( GetScreen()->m_BlockLocate.GetState() == STATE_BLOCK_MOVE )
        {
            GetCanvas()->SetAutoPanRequest( false );
            HandleBlockPlace( aDC );
        }
        else if( GetScreen()->m_BlockLocate.GetState() == STATE_NO_BLOCK )
        {
            auto pos = GetCrossHairPosition();
            OnLeftClick( aDC, pos );

            if( hotKey->m_Idcommand == HK_LEFT_DCLICK )
                OnLeftDClick( aDC, pos );
        }
        break;
    }

    // Hot key handled.
    return true;
}


EDA_HOTKEY* LIB_VIEW_FRAME::GetHotKeyDescription( int aCommand ) const
{
    EDA_HOTKEY* HK_Descr = GetDescriptorFromCommand( aCommand, common_Hotkey_List );

    if( HK_Descr == NULL )
        HK_Descr = GetDescriptorFromCommand( aCommand, viewlib_Hotkey_List );

    return HK_Descr;
}


bool LIB_VIEW_FRAME::OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem )
{
    if( aHotKey == 0 )
        return false;

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
    cmd.SetEventObject( this );

    /* Convert lower to upper case (the usual toupper function has problem with non ascii
     * codes like function keys */
    if( (aHotKey >= 'a') && (aHotKey <= 'z') )
        aHotKey += 'A' - 'a';

    EDA_HOTKEY* HK_Descr = GetDescriptorFromHotkey( aHotKey, common_basic_Hotkey_List );

    if( HK_Descr == NULL )
        HK_Descr = GetDescriptorFromHotkey( aHotKey, viewlib_Hotkey_List );

    if( HK_Descr == NULL )
        return false;

    switch( HK_Descr->m_Idcommand )
    {
    default:
    case HK_NOT_FOUND:
        return false;

    case HK_HELP:                   // Display Current hotkey list
        DisplayHotkeyList( this, g_Viewlib_Hotkeys_Descr );
        break;

    case HK_RESET_LOCAL_COORD:      // set local (relative) coordinate origin
        GetScreen()->m_O_Curseur = GetCrossHairPosition();
        break;

    case HK_LEFT_CLICK:
        OnLeftClick( aDC, aPosition );
        break;

    case HK_LEFT_DCLICK:    // Simulate a double left click: generate 2 events
        OnLeftClick( aDC, aPosition );
        OnLeftDClick( aDC, aPosition );
        break;

    case HK_ZOOM_IN:
        cmd.SetId( ID_KEY_ZOOM_IN );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_ZOOM_OUT:
        cmd.SetId( ID_KEY_ZOOM_OUT );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_ZOOM_REDRAW:
        cmd.SetId( ID_ZOOM_REDRAW );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_ZOOM_CENTER:
        cmd.SetId( ID_POPUP_ZOOM_CENTER );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_ZOOM_AUTO:
        cmd.SetId( ID_ZOOM_PAGE );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case HK_CANVAS_CAIRO:
    case HK_CANVAS_OPENGL:
        cmd.SetInt( HK_Descr->m_Idcommand );
        cmd.SetId( HK_Descr->m_IdMenuEvent );
        GetEventHandler()->ProcessEvent( cmd );
    }

    return true;
}
