/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file eeschema/onrightclick.cpp
 */

#include <fctsys.h>
#include <eeschema_id.h>
#include <sch_draw_panel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <menus_helpers.h>

#include <general.h>
#include <hotkeys.h>
#include <class_library.h>
#include <sch_bus_entry.h>
#include <sch_marker.h>
#include <sch_text.h>
#include <sch_junction.h>
#include <sch_component.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include <sch_bitmap.h>
#include <symbol_lib_table.h>
#include <sch_view.h>

#include <iostream>

static void AddMenusForBlock( wxMenu* PopMenu, SCH_EDIT_FRAME* frame );
static void AddMenusForWire( wxMenu* PopMenu, SCH_LINE* Wire, SCH_EDIT_FRAME* frame );
static void AddMenusForBus( wxMenu* PopMenu, SCH_LINE* Bus, SCH_EDIT_FRAME* frame );
static void AddMenusForHierchicalSheet( wxMenu* PopMenu, SCH_SHEET* Sheet );
static void AddMenusForSheetPin( wxMenu* PopMenu, SCH_SHEET_PIN* PinSheet );
static void AddMenusForText( wxMenu* PopMenu, SCH_TEXT* Text );
static void AddMenusForLabel( wxMenu* PopMenu, SCH_LABEL* Label );
static void AddMenusForGLabel( wxMenu* PopMenu, SCH_GLOBALLABEL* GLabel );
static void AddMenusForHLabel( wxMenu* PopMenu, SCH_HIERLABEL* GLabel );
static void AddMenusForEditComponent( wxMenu* PopMenu, SCH_COMPONENT* Component,
                                      SYMBOL_LIB_TABLE* aLibs );
static void AddMenusForComponent( wxMenu* PopMenu, SCH_COMPONENT* Component,
                                  SYMBOL_LIB_TABLE* aLibs );
static void AddMenusForComponentField( wxMenu* PopMenu, SCH_FIELD* Field );
static void AddMenusForMarkers( wxMenu* aPopMenu, SCH_MARKER* aMarker, SCH_EDIT_FRAME* aFrame );
static void AddMenusForBitmap( wxMenu* aPopMenu, SCH_BITMAP * aBitmap );
static void AddMenusForBusEntry( wxMenu* aPopMenu, SCH_BUS_ENTRY_BASE * aBusEntry );


bool SCH_EDIT_FRAME::OnRightClick( const wxPoint& aPosition, wxMenu* PopMenu )
{
    SCH_ITEM*   item = GetScreen()->GetCurItem();
    bool        blockActive = GetScreen()->IsBlockActive();
    wxString    msg;

    // Ugly hack, clear any highligthed symbol, because the HIGHLIGHT flag create issues when creating menus
    // Will be fixed later
    GetCanvas()->GetView()->HighlightItem( nullptr, nullptr );

    // Do not start a block command on context menu.
    m_canvas->SetCanStartBlock( -1 );

    if( blockActive )
    {
        AddMenusForBlock( PopMenu, this );
        PopMenu->AppendSeparator();

        // If we have a block containing only one main element we append its edit submenu
        if( item != NULL )
        {
            switch( item->Type() )
            {
            case SCH_COMPONENT_T:
                AddMenusForEditComponent( PopMenu, (SCH_COMPONENT *) item,
                                          Prj().SchSymbolLibTable() );
                PopMenu->AppendSeparator();
                break;

            case SCH_TEXT_T:
                msg = AddHotkeyName( _( "Edit Text..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
                AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
                PopMenu->AppendSeparator();
                break;

            case SCH_LABEL_T:
                msg = AddHotkeyName( _( "Edit Label..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
                AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
                PopMenu->AppendSeparator();
                break;

            case SCH_GLOBAL_LABEL_T:
                msg = AddHotkeyName( _( "Edit Global Label..." ), g_Schematic_Hotkeys_Descr,
                                     HK_EDIT );
                AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
                PopMenu->AppendSeparator();
                break;

            case SCH_HIERARCHICAL_LABEL_T:
                msg = AddHotkeyName( _( "Edit Hierarchical Label..." ), g_Schematic_Hotkeys_Descr,
                                     HK_EDIT );
                AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
                PopMenu->AppendSeparator();
                break;

            case SCH_BITMAP_T:
                msg = AddHotkeyName( _( "Edit Image..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
                AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( image_xpm ) );
                PopMenu->AppendSeparator();
                break;

            default:
                break;
            }
        }
        return true;
    }

    // Try to locate items at cursor position.
    if( (item == NULL) || (item->GetFlags() == 0) )
    {
        bool actionCancelled = false;
        item = LocateAndShowItem( aPosition, SCH_COLLECTOR::AllItemsButPins, 0, &actionCancelled );

        // If the clarify item selection context menu is aborted, don't show the context menu.
        if( item == NULL && actionCancelled )
            return false;
    }

    // If a command is in progress: add "cancel" and "end tool" menu
    if( GetToolId() != ID_NO_TOOL_SELECTED )
    {
        if( item && item->GetFlags() )
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "Cancel" ),
                         KiBitmap( cancel_xpm ) );
        }
        else
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "End Tool" ),
                         KiBitmap( cursor_xpm ) );
        }

        PopMenu->AppendSeparator();

        switch( GetToolId() )
        {
        case ID_WIRE_BUTT:
            AddMenusForWire( PopMenu, NULL, this );
            if( item == NULL )
                PopMenu->AppendSeparator();
            break;

        case ID_BUS_BUTT:
            AddMenusForBus( PopMenu, NULL, this );
            if( item == NULL )
                PopMenu->AppendSeparator();
            break;

        default:
            break;
        }
    }
    else
    {
        if( item && item->GetFlags() )
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "Cancel" ),
                         KiBitmap( cancel_xpm ) );
            PopMenu->AppendSeparator();
        }
    }

    if( item == NULL )
    {
        if( GetToolId() == ID_NO_TOOL_SELECTED && m_blockItems.GetCount() > 0 )
       {
            msg = AddHotkeyName( _( "&Paste" ), g_Schematic_Hotkeys_Descr, HK_EDIT_PASTE );
            AddMenuItem( PopMenu, wxID_PASTE, msg,
                        _( "Pastes item(s) from the Clipboard" ),
                        KiBitmap( paste_xpm ) );
            PopMenu->AppendSeparator();
        }

        if( m_CurrentSheet->Last() != g_RootSheet )
        {
            msg = AddHotkeyName( _( "Leave Sheet" ), g_Schematic_Hotkeys_Descr, HK_LEAVE_SHEET );
            AddMenuItem( PopMenu, ID_POPUP_SCH_LEAVE_SHEET, msg,
                         KiBitmap( leave_sheet_xpm ) );
            PopMenu->AppendSeparator();
        }

        return true;
    }

    bool is_new = item->IsNew();

    switch( item->Type() )
    {
    case SCH_NO_CONNECT_T:
        AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, _( "Delete No Connect" ),
                     KiBitmap( delete_xpm ) );
        break;

    case SCH_JUNCTION_T:
        addJunctionMenuEntries( PopMenu, (SCH_JUNCTION*) item );
        break;

    case SCH_BUS_BUS_ENTRY_T:
    case SCH_BUS_WIRE_ENTRY_T:
        AddMenusForBusEntry( PopMenu, static_cast<SCH_BUS_ENTRY_BASE*>( item ) );
        break;

    case SCH_MARKER_T:
        AddMenusForMarkers( PopMenu, (SCH_MARKER*) item, this );
        break;

    case SCH_TEXT_T:
        AddMenusForText( PopMenu, (SCH_TEXT*) item );
        break;

    case SCH_LABEL_T:
        AddMenusForLabel( PopMenu, (SCH_LABEL*) item );
        break;

    case SCH_GLOBAL_LABEL_T:
        AddMenusForGLabel( PopMenu, (SCH_GLOBALLABEL*) item );
        break;

    case SCH_HIERARCHICAL_LABEL_T:
        AddMenusForHLabel( PopMenu, (SCH_HIERLABEL*) item );
        break;

    case SCH_FIELD_T:
        AddMenusForComponentField( PopMenu, (SCH_FIELD*) item );
        break;

    case SCH_COMPONENT_T:
        AddMenusForComponent( PopMenu, (SCH_COMPONENT*) item, Prj().SchSymbolLibTable() );
        break;

    case SCH_BITMAP_T:
        AddMenusForBitmap( PopMenu, (SCH_BITMAP*) item );
        break;

    case SCH_LINE_T:
        switch( item->GetLayer() )
        {
        case LAYER_WIRE:
            AddMenusForWire( PopMenu, (SCH_LINE*) item, this );
            break;

        case LAYER_BUS:
            AddMenusForBus( PopMenu, (SCH_LINE*) item, this );
            break;

        default:
            if( is_new )
                AddMenuItem( PopMenu, ID_POPUP_END_LINE, _( "End Drawing" ),
                             KiBitmap( checked_ok_xpm ) );

            AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, _( "Edit..." ),
                         KiBitmap( edit_xpm ) );
            AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, _( "Delete Drawing" ),
                         KiBitmap( delete_xpm ) );
            break;
        }
        break;

    case SCH_SHEET_T:
        AddMenusForHierchicalSheet( PopMenu, (SCH_SHEET*) item );
        break;

    case SCH_SHEET_PIN_T:
        AddMenusForSheetPin( PopMenu, (SCH_SHEET_PIN*) item );
        break;

    default:
        wxFAIL_MSG( wxString::Format( wxT( "Cannot create context menu for unknown type %d" ),
                                      item->Type() ) );
        break;
    }

    PopMenu->AppendSeparator();
    return true;
}


void AddMenusForComponentField( wxMenu* PopMenu, SCH_FIELD* Field )
{
    wxString msg, name;

    if( !Field->GetFlags() )
    {
        switch( Field->GetId() )
        {
        case REFERENCE: name = _( "Move Reference" ); break;
        case VALUE:     name = _( "Move Value" ); break;
        case FOOTPRINT: name = _( "Move Footprint Field" ); break;
        default:        name = _( "Move Field" ); break;
        }

        msg = AddHotkeyName( name, g_Schematic_Hotkeys_Descr,
                HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
    }

    switch( Field->GetId() )
    {
    case REFERENCE: name = _( "Rotate Reference" ); break;
    case VALUE:     name = _( "Rotate Value" ); break;
    case FOOTPRINT: name = _( "Rotate Footprint Field" ); break;
    default:        name = _( "Rotate Field" ); break;
    }

    msg = AddHotkeyName( name, g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );

    // Ref, value and footprint have specific hotkeys. Show the specific hotkey:
    hotkey_id_commnand id;
    switch( Field->GetId() )
    {
    case REFERENCE:
        id = HK_EDIT_COMPONENT_REFERENCE;
        name = _( "Edit Reference..." );
        break;
    case VALUE:
        id = HK_EDIT_COMPONENT_VALUE;
        name = _( "Edit Value..." );
        break;
    case FOOTPRINT:
        id = HK_EDIT_COMPONENT_FOOTPRINT;
        name = _( "Edit Footprint Field..." );
        break;
    case DATASHEET:
        id = HK_SHOW_COMPONENT_DATASHEET;
        name = _( "Show Datasheet" );
        break;
    default:
        id = HK_EDIT;
        name = _( "Edit Field..." );
        break;
    }
    msg = AddHotkeyName( name, g_Schematic_Hotkeys_Descr, id );
    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
}


void AddMenusForComponent( wxMenu* PopMenu, SCH_COMPONENT* Component, SYMBOL_LIB_TABLE* aLibs )
{
    if( Component->Type() != SCH_COMPONENT_T )
    {
        wxASSERT( 0 );
        return;
    }

    wxString       msg;

    if( !Component->GetFlags() )
    {
        msg.Printf( _( "Move %s" ), Component->GetField( REFERENCE )->GetText() );
        msg = AddHotkeyName( msg, g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );
    }

    wxMenu* orientmenu = new wxMenu;
    msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( orientmenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );
    AddMenuItem( orientmenu, ID_SCH_ROTATE_COUNTERCLOCKWISE, _( "Rotate Counterclockwise" ),
                 KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_X );
    AddMenuItem( orientmenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_Y );
    AddMenuItem( orientmenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );
    msg = AddHotkeyName( _( "Reset to Default" ), g_Schematic_Hotkeys_Descr,
                         HK_ORIENT_NORMAL_COMPONENT );
    AddMenuItem( orientmenu, ID_SCH_ORIENT_NORMAL, msg, KiBitmap( normal_xpm ) );
    AddMenuItem( PopMenu, orientmenu, ID_POPUP_SCH_GENERIC_ORIENT_CMP,
                 _( "Orientation" ), KiBitmap( orient_xpm ) );

    AddMenusForEditComponent( PopMenu, Component, aLibs );

    if( !Component->GetFlags() )
    {
        msg = AddHotkeyName( _( "Duplicate" ), g_Schematic_Hotkeys_Descr,
                             HK_DUPLICATE_ITEM );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DUPLICATE_ITEM, msg, KiBitmap( duplicate_xpm ) );
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE_CMP, msg, KiBitmap( delete_xpm ) );
    }

    msg = AddHotkeyName( _( "Autoplace Fields" ), g_Schematic_Hotkeys_Descr, HK_AUTOPLACE_FIELDS );
    AddMenuItem( PopMenu, ID_AUTOPLACE_FIELDS, msg, KiBitmap( autoplace_fields_xpm ) );

    if( !Component->GetField( DATASHEET )->GetFullyQualifiedText().IsEmpty() )
        AddMenuItem( PopMenu, ID_POPUP_SCH_DISPLAYDOC_CMP, _( "Open Documentation" ),
                     KiBitmap( datasheet_xpm ) );
}


void AddMenusForEditComponent( wxMenu* PopMenu, SCH_COMPONENT* Component, SYMBOL_LIB_TABLE* aLibs )
{
    if( Component->Type() != SCH_COMPONENT_T )
    {
        wxASSERT( 0 );
        return;
    }

    wxString    msg;
    LIB_PART*   part = NULL;
    LIB_ALIAS*  alias = NULL;

    try
    {
        alias = aLibs->LoadSymbol( Component->GetLibId() );
    }
    catch( ... )
    {
    }

    if( alias )
        part = alias->GetPart();

    wxMenu* editmenu = new wxMenu;
    msg = AddHotkeyName( _( "Edit Properties..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( editmenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( config_xpm ) );

    if( part && part->IsNormal() )
    {
        msg = AddHotkeyName( _( "Edit Value..." ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COMPONENT_VALUE );
        AddMenuItem( editmenu, ID_SCH_EDIT_COMPONENT_VALUE, msg,
                     KiBitmap( edit_comp_value_xpm ) );

        msg = AddHotkeyName( _( "Edit Reference..." ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COMPONENT_REFERENCE );
        AddMenuItem( editmenu, ID_SCH_EDIT_COMPONENT_REFERENCE, msg,
                     KiBitmap( edit_comp_ref_xpm ) );

        msg = AddHotkeyName( _( "Edit Footprint..." ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COMPONENT_FOOTPRINT );
        AddMenuItem( editmenu, ID_SCH_EDIT_COMPONENT_FOOTPRINT, msg,
                     KiBitmap( edit_comp_footprint_xpm ) );

        msg = AddHotkeyName( _( "Show Datasheet" ), g_Schematic_Hotkeys_Descr,
                             HK_SHOW_COMPONENT_DATASHEET );
        AddMenuItem( editmenu, ID_POPUP_SCH_DISPLAYDOC_CMP, msg,
                     KiBitmap( datasheet_xpm ) );
    }

    if( part && part->HasConversion() )
        AddMenuItem( editmenu, ID_POPUP_SCH_EDIT_CONVERT_CMP, _( "Convert" ),
                     KiBitmap( component_select_alternate_shape_xpm ) );

    if( part && part->GetUnitCount() >= 2 )
    {
        wxMenu* sel_unit_menu = new wxMenu; int ii;

        for( ii = 0; ii < part->GetUnitCount(); ii++ )
        {
            wxString num_unit;
            int unit = Component->GetUnit();
            num_unit.Printf( _( "Unit %s" ), GetChars( LIB_PART::SubReference(  ii + 1, false ) ) );
            wxMenuItem * item = sel_unit_menu->Append( ID_POPUP_SCH_SELECT_UNIT1 + ii,
                                                       num_unit, wxEmptyString,
                                                       wxITEM_CHECK );
            if( unit == ii + 1 )
                item->Check(true);

            // The ID max for these submenus is ID_POPUP_SCH_SELECT_UNIT_CMP_MAX
            // See eeschema_id to modify this value.
            if( ii >= (ID_POPUP_SCH_SELECT_UNIT_CMP_MAX - ID_POPUP_SCH_SELECT_UNIT1) )
                break;      // We have used all IDs for these submenus
        }

        AddMenuItem( editmenu, sel_unit_menu, ID_POPUP_SCH_SELECT_UNIT_CMP,
                     _( "Unit" ), KiBitmap( component_select_unit_xpm ) );
    }

    if( !Component->GetFlags() )
    {
        msg = AddHotkeyName( _( "Edit with Library Editor" ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COMPONENT_WITH_LIBEDIT );
        AddMenuItem( editmenu, ID_POPUP_SCH_CALL_LIBEDIT_AND_LOAD_CMP,
                     msg, KiBitmap( libedit_xpm ) );
    }

    AddMenuItem( PopMenu, editmenu, ID_SCH_EDIT_ITEM,
                 _( "Properties" ), KiBitmap( config_xpm ) );
}


void AddMenusForGLabel( wxMenu* PopMenu, SCH_GLOBALLABEL* GLabel )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    if( !GLabel->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr,
                             HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );
        msg = AddHotkeyName( _( "Duplicate" ), g_Schematic_Hotkeys_Descr,
                             HK_DUPLICATE_ITEM );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DUPLICATE_ITEM, msg, KiBitmap( duplicate_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );
    msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
    msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                 _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                 _( "Change to Label" ), KiBitmap( glabel2label_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( glabel2text_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForHLabel( wxMenu* PopMenu, SCH_HIERLABEL* HLabel )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    if( !HLabel->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );
        msg = AddHotkeyName( _( "Copy" ), g_Schematic_Hotkeys_Descr,
                             HK_DUPLICATE_ITEM );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DUPLICATE_ITEM, msg, KiBitmap( copy_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );
    msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
    msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                 _( "Change to Label" ), KiBitmap( glabel2label_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( glabel2text_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                 _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForLabel( wxMenu* PopMenu, SCH_LABEL* Label )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    if( !Label->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );
        msg = AddHotkeyName( _( "Duplicate" ), g_Schematic_Hotkeys_Descr, HK_DUPLICATE_ITEM );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DUPLICATE_ITEM, msg, KiBitmap( duplicate_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Counterclockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
    msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                 _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( label2text_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                 _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForText( wxMenu* PopMenu, SCH_TEXT* Text )
{
    wxString msg;
    wxMenu*  menu_change_type = new wxMenu;

    if( !Text->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Duplicate" ), g_Schematic_Hotkeys_Descr, HK_DUPLICATE_ITEM );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DUPLICATE_ITEM, msg, KiBitmap( duplicate_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Counterclockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( edit_text_xpm ) );
    msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    /* add menu change type text (to label, glabel, text),
     * but only if this is a single line text
     */
    if( Text->GetText().Find( wxT( "\n" ) ) ==  wxNOT_FOUND )
    {
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                     _( "Change to Label" ), KiBitmap( label2text_xpm ) );
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                     _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                     _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
        AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                     _( "Change Type" ), KiBitmap( gl_change_xpm ) );
    }
}


void SCH_EDIT_FRAME::addJunctionMenuEntries( wxMenu* aMenu, SCH_JUNCTION* aJunction )
{
    wxString msg;
    SCH_SCREEN* screen = GetScreen();

    msg = AddHotkeyName( _( "Delete Junction" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( aMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    if( !aJunction->IsNew() )
    {
        if( m_collectedItems.IsDraggableJunction() )
            AddMenuItem( aMenu, ID_SCH_DRAG_ITEM, _( "Drag Junction" ), KiBitmap( drag_xpm ) );
    }

    if( screen->GetWireOrBus( aJunction->GetPosition() ) )
    {
        AddMenuItem( aMenu, ID_POPUP_SCH_DELETE_NODE, _( "Delete Node" ),
                     KiBitmap( delete_node_xpm ) );
        AddMenuItem( aMenu, ID_POPUP_SCH_DELETE_CONNECTION, _( "Delete Connection" ),
                     KiBitmap( delete_connection_xpm ) );
    }
}


void AddMenusForWire( wxMenu* PopMenu, SCH_LINE* Wire, SCH_EDIT_FRAME* frame )
{
    wxPoint     pos    = frame->GetCrossHairPosition();
    wxString    msg;

    if( Wire == NULL )
    {
        msg = AddHotkeyName( _( "Begin Wire" ), g_Schematic_Hotkeys_Descr, HK_BEGIN_WIRE );
        AddMenuItem( PopMenu, ID_POPUP_SCH_BEGIN_WIRE, msg, KiBitmap( add_line_xpm ) );
        return;
    }

    if( Wire->IsNew() )
    {
        msg = AddHotkeyName( _( "Wire End" ), g_Schematic_Hotkeys_Descr, HK_END_CURR_LINEWIREBUS );
        AddMenuItem( PopMenu, ID_POPUP_END_LINE, msg, KiBitmap( checked_ok_xpm ) );
        return;
    }

    msg = AddHotkeyName( _( "Drag Wire" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
    AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );
    PopMenu->AppendSeparator();
    msg = AddHotkeyName( _( "Delete Wire" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE_NODE, _( "Delete Node" ),
                 KiBitmap( delete_node_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE_CONNECTION, _( "Delete Connection" ),
                 KiBitmap( delete_connection_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_BREAK_WIRE, _( "Break Wire" ), KiBitmap( break_line_xpm ) );

    PopMenu->AppendSeparator();

    msg = AddHotkeyName( _( "Add Junction" ), g_Schematic_Hotkeys_Descr, HK_ADD_JUNCTION );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_JUNCTION, msg, KiBitmap( add_junction_xpm ) );
    msg = AddHotkeyName( _( "Add Label..." ), g_Schematic_Hotkeys_Descr, HK_ADD_LABEL );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_LABEL, msg, KiBitmap( add_line_label_xpm ) );

    // Add global label command only if the cursor is over one end of the wire.
    if( Wire->IsEndPoint( pos ) )
        AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_GLABEL, _( "Add Global Label..." ),
                     KiBitmap( add_glabel_xpm ) );
}


void AddMenusForBus( wxMenu* PopMenu, SCH_LINE* Bus, SCH_EDIT_FRAME* frame )
{
    wxPoint     pos = frame->GetCrossHairPosition();
    wxString    msg;

    if( Bus == NULL )
    {
        msg = AddHotkeyName( _( "Begin Bus" ), g_Schematic_Hotkeys_Descr, HK_BEGIN_BUS );
        AddMenuItem( PopMenu, ID_POPUP_SCH_BEGIN_BUS, msg, KiBitmap( add_bus_xpm ) );
        return;
    }

    if( Bus->IsNew() )
    {
        msg = AddHotkeyName( _( "Bus End" ), g_Schematic_Hotkeys_Descr, HK_END_CURR_LINEWIREBUS );
        AddMenuItem( PopMenu, ID_POPUP_END_LINE, msg, KiBitmap( checked_ok_xpm ) );
        return;
    }

    msg = AddHotkeyName( _( "Delete Bus" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_bus_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_BREAK_WIRE, _( "Break Bus" ), KiBitmap( break_bus_xpm ) );

    PopMenu->AppendSeparator();
    msg = AddHotkeyName( _( "Add Junction" ), g_Schematic_Hotkeys_Descr, HK_ADD_JUNCTION );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_JUNCTION, msg, KiBitmap( add_junction_xpm ) );
    msg = AddHotkeyName( _( "Add Label..." ), g_Schematic_Hotkeys_Descr, HK_ADD_LABEL );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_LABEL, msg, KiBitmap( add_line_label_xpm ) );

    // Add global label command only if the cursor is over one end of the bus.
    if( Bus->IsEndPoint( pos ) )
        AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_GLABEL, _( "Add Global Label..." ),
                     KiBitmap( add_glabel_xpm ) );
}


void AddMenusForHierchicalSheet( wxMenu* PopMenu, SCH_SHEET* Sheet )
{
    wxString msg;

    if( !Sheet->GetFlags() )
    {
        AddMenuItem( PopMenu, ID_POPUP_SCH_ENTER_SHEET, _( "Enter Sheet" ),
                     KiBitmap( enter_sheet_xpm ) );
        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );

        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG_ITEM, msg, KiBitmap( drag_xpm ) );

        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Select Items On PCB" ), g_Schematic_Hotkeys_Descr,
                             HK_SELECT_ITEMS_ON_PCB );
        AddMenuItem( PopMenu, ID_POPUP_SCH_SELECT_ON_PCB, msg, KiBitmap( select_same_sheet_xpm ) );
        PopMenu->AppendSeparator();

        wxMenu* orientmenu = new wxMenu;
        msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
        AddMenuItem( orientmenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );

        AddMenuItem( orientmenu, ID_SCH_ROTATE_COUNTERCLOCKWISE, _( "Rotate Counterclockwise" ),
                     KiBitmap( rotate_ccw_xpm ) );

        msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                             HK_MIRROR_X );
        AddMenuItem( orientmenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
        msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                             HK_MIRROR_Y );
        AddMenuItem( orientmenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );

        AddMenuItem( PopMenu, orientmenu, ID_POPUP_SCH_GENERIC_ORIENT_CMP,
                 _( "Orientation" ), KiBitmap( orient_xpm ) );
    }

    if( Sheet->GetFlags() )
    {
        AddMenuItem( PopMenu, ID_POPUP_SCH_END_SHEET, _( "Place" ), KiBitmap( checked_ok_xpm ) );
    }
    else
    {
        msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
        AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( editor_xpm ) );

        AddMenuItem( PopMenu, ID_POPUP_SCH_RESIZE_SHEET, _( "Resize" ),
                     KiBitmap( resize_sheet_xpm ) );
        PopMenu->AppendSeparator();
        AddMenuItem( PopMenu, ID_POPUP_IMPORT_HLABEL_TO_SHEETPIN, _( "Import Sheet Pins" ),
                     KiBitmap( import_hierarchical_label_xpm ) );

        if( Sheet->HasUndefinedPins() )  // Sheet has pin labels, and can be cleaned
            AddMenuItem( PopMenu, ID_POPUP_SCH_CLEANUP_SHEET, _( "Cleanup Sheet Pins" ),
                         KiBitmap( options_pinsheet_xpm ) );

        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_sheet_xpm ) );
    }
}


void AddMenusForSheetPin( wxMenu* PopMenu, SCH_SHEET_PIN* PinSheet )
{
    wxString msg;

    if( !PinSheet->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
    }

    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, _( "Edit..." ), KiBitmap( edit_xpm ) );

    if( !PinSheet->GetFlags() )
        AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE, _( "Delete" ), KiBitmap( delete_xpm ) );
}


void AddMenusForBlock( wxMenu* PopMenu, SCH_EDIT_FRAME* frame )
{
    wxString msg;

    AddMenuItem( PopMenu, ID_POPUP_CANCEL_CURRENT_COMMAND, _( "Cancel Block" ),
                 KiBitmap( cancel_xpm ) );

    PopMenu->AppendSeparator();

    if( frame->GetScreen()->m_BlockLocate.GetCommand() == BLOCK_MOVE )
        AddMenuItem( PopMenu, ID_POPUP_ZOOM_BLOCK, _( "Window Zoom" ), KiBitmap( zoom_area_xpm ) );

    AddMenuItem( PopMenu, ID_POPUP_PLACE_BLOCK, _( "Place Block" ), KiBitmap( checked_ok_xpm ) );

    // After a block move (that is also a block selection) one can reselect
    // a block function.
    if( frame->GetScreen()->m_BlockLocate.GetCommand() == BLOCK_MOVE )
    {
        msg = AddHotkeyName( _( "Cut Block" ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_CUT );
        AddMenuItem( PopMenu, ID_POPUP_CUT_BLOCK, msg, KiBitmap( cut_xpm ) );
        msg = AddHotkeyName( _( "Copy Block" ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COPY );
        AddMenuItem( PopMenu, ID_POPUP_COPY_BLOCK, msg, KiBitmap( copy_xpm ) );
        AddMenuItem( PopMenu, ID_POPUP_DUPLICATE_BLOCK, _( "Duplicate Block" ),
                     KiBitmap( duplicate_xpm ) );
        msg = AddHotkeyName( _( "Drag Block" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVEBLOCK_TO_DRAGBLOCK );
        AddMenuItem( PopMenu, ID_POPUP_DRAG_BLOCK, msg, KiBitmap( drag_xpm ) );
        msg = AddHotkeyName( _( "Delete Block" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( PopMenu, ID_POPUP_DELETE_BLOCK, msg, KiBitmap( delete_xpm ) );
        msg = AddHotkeyName( _( "Mirror Block Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                             HK_MIRROR_Y );
        AddMenuItem( PopMenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );
        msg = AddHotkeyName( _( "Mirror Block Around Horizontal(X) Axis" ),
                             g_Schematic_Hotkeys_Descr, HK_MIRROR_X );
        AddMenuItem( PopMenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
        msg = AddHotkeyName( _( "Rotate Block CCW" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
        AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_ccw_xpm ) );

#if 0
  #ifdef __WINDOWS__
        AddMenuItem( menu_other_block_commands, ID_GEN_COPY_BLOCK_TO_CLIPBOARD,
                     _( "Copy to Clipboard" ), KiBitmap( copy_xpm ) );
  #endif
#endif
    }
}


void AddMenusForMarkers( wxMenu* aPopMenu, SCH_MARKER* aMarker, SCH_EDIT_FRAME* aFrame )
{
    AddMenuItem( aPopMenu, ID_POPUP_SCH_DELETE, _( "Delete Marker" ), KiBitmap( delete_xpm ) );
    AddMenuItem( aPopMenu, ID_POPUP_SCH_GETINFO_MARKER, _( "Marker Error Info" ),
                 KiBitmap( info_xpm ) );
}


void AddMenusForBitmap( wxMenu* aPopMenu, SCH_BITMAP * aBitmap )
{
    wxString msg;

    if( aBitmap->GetFlags() == 0 )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( aPopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Counterclockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( aPopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_X );
    AddMenuItem( aPopMenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_Y );
    AddMenuItem( aPopMenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );
    msg = AddHotkeyName( _( "Edit Image..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( aPopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( image_xpm ) );

    if( aBitmap->GetFlags() == 0 )
    {
        aPopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( aPopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
    }
}


void AddMenusForBusEntry( wxMenu* aPopMenu, SCH_BUS_ENTRY_BASE* aBusEntry )
{
    wxString msg;

    if( !aBusEntry->GetFlags() )
    {
        msg = AddHotkeyName( _( "Move Bus Entry" ), g_Schematic_Hotkeys_Descr,
                                      HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( aPopMenu, ID_SCH_MOVE_ITEM, msg, KiBitmap( move_xpm ) );
    }

    if( aBusEntry->GetBusEntryShape() == '\\' )
        AddMenuItem( aPopMenu, ID_POPUP_SCH_ENTRY_SELECT_SLASH,
                     _( "Set Bus Entry Shape /" ), KiBitmap( change_entry_orient_xpm ) );
    else
        AddMenuItem( aPopMenu, ID_POPUP_SCH_ENTRY_SELECT_ANTISLASH,
                     _( "Set Bus Entry Shape \\" ), KiBitmap( change_entry_orient_xpm ) );

    msg = AddHotkeyName( _( "Delete Bus Entry" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( aPopMenu, ID_POPUP_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
}
