/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2004-2019 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file edit_label.cpp
 * @brief Label, global label and text creation and editing.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <base_struct.h>
#include <draw_graphic_text.h>
#include <sch_draw_panel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <sch_view.h>
#include <kicad_device_context.h>

#include <general.h>
#include <sch_text.h>
#include <eeschema_id.h>


static PINSHEETLABEL_SHAPE  lastGlobalLabelShape = NET_INPUT;
static int                  lastTextOrientation = 0;
static bool                 lastTextBold = false;
static bool                 lastTextItalic = false;


void SCH_EDIT_FRAME::ChangeTextOrient( SCH_TEXT* aTextItem )
{
    wxCHECK_RET( (aTextItem != NULL) && aTextItem->CanIncrementLabel(),
                 wxT( "Invalid schematic text item." )  );

    int orient = ( aTextItem->GetLabelSpinStyle() + 1 ) & 3;

    // Save current text orientation in undo list if is not already in edit.
    if( aTextItem->GetFlags() == 0 )
        SaveCopyInUndoList( aTextItem, UR_CHANGED );

    aTextItem->SetLabelSpinStyle( orient );

    RefreshItem( aTextItem );
    OnModify();
}


SCH_TEXT* SCH_EDIT_FRAME::CreateNewText( int aType )
{
    SCH_TEXT* textItem = NULL;

    SetRepeatItem( NULL );

    switch( aType )
    {
    case LAYER_NOTES:
        textItem = new SCH_TEXT( GetCrossHairPosition() );
        break;

    case LAYER_LOCLABEL:
        textItem = new SCH_LABEL( GetCrossHairPosition() );
        break;

    case LAYER_HIERLABEL:
        textItem = new SCH_HIERLABEL( GetCrossHairPosition() );
        textItem->SetShape( lastGlobalLabelShape );
        break;

    case LAYER_GLOBLABEL:
        textItem = new SCH_GLOBALLABEL( GetCrossHairPosition() );
        textItem->SetShape( lastGlobalLabelShape );
        break;

    default:
        DisplayError( this, wxT( "SCH_EDIT_FRAME::CreateNewText() Internal error" ) );
        return NULL;
    }

    textItem->SetBold( lastTextBold );
    textItem->SetItalic( lastTextItalic );
    textItem->SetLabelSpinStyle( lastTextOrientation );
    textItem->SetTextSize( wxSize( GetDefaultTextSize(), GetDefaultTextSize() ) );
    textItem->SetFlags( IS_NEW | IS_MOVED );

    EditSchematicText( textItem );

    if( textItem->GetText().IsEmpty() )
    {
        delete textItem;
        return NULL;
    }

    lastTextBold = textItem->IsBold();
    lastTextItalic = textItem->IsItalic();
    lastTextOrientation = textItem->GetLabelSpinStyle();

    if( textItem->Type() == SCH_GLOBAL_LABEL_T || textItem->Type() == SCH_HIERARCHICAL_LABEL_T )
        lastGlobalLabelShape = textItem->GetShape();

    // Prepare display to move the new item
    PrepareMoveItem( textItem );

    return textItem;
}


void SCH_EDIT_FRAME::OnConvertTextType( wxCommandEvent& aEvent )
{
    SCH_SCREEN* screen = GetScreen();
    SCH_TEXT* text = (SCH_TEXT*) screen->GetCurItem();

    wxCHECK_RET( (text != NULL) && text->CanIncrementLabel(), "Cannot convert text type." );

    KICAD_T type;

    switch( aEvent.GetId() )
    {
    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL:
        type = SCH_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL:
        type = SCH_GLOBAL_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL:
        type = SCH_HIERARCHICAL_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT:
        type = SCH_TEXT_T;
        break;

    default:
        wxFAIL_MSG( wxString::Format( "Invalid text type command ID %d.", aEvent.GetId() ) );
        return;
    }

    if( text->Type() == type )
        return;

    SCH_TEXT* newtext = nullptr;
    const wxPoint &position = text->GetPosition();
    wxString txt = text->GetText();

    // There can be characters in a SCH_TEXT object that can break labels so we have to
    // fix them here.
    if( text->Type() == SCH_TEXT_T )
    {
        txt.Replace( "\n", "_" );
        txt.Replace( "\r", "_" );
        txt.Replace( "\t", "_" );
        txt.Replace( " ", "_" );
        txt.Replace( "/", "_" );
    }

    switch( type )
    {
    case SCH_LABEL_T:
        newtext = new SCH_LABEL( position, txt );
        break;

    case SCH_GLOBAL_LABEL_T:
        newtext = new SCH_GLOBALLABEL( position, txt );
        break;

    case SCH_HIERARCHICAL_LABEL_T:
        newtext = new SCH_HIERLABEL( position, txt );
        break;

    case SCH_TEXT_T:
        newtext = new SCH_TEXT( position, txt );
        break;

    default:
        wxASSERT_MSG( false, wxString::Format( "Invalid text type: %d.", type ) );
        return;
    }

    /* Copy the old text item settings to the new one.  Justifications are not copied because
     * they are not used in labels.  Justifications will be set to default value in the new
     * text item type.
     */
    newtext->SetFlags( text->GetFlags() );
    newtext->SetShape( text->GetShape() );
    newtext->SetLabelSpinStyle( text->GetLabelSpinStyle() );
    newtext->SetTextSize( text->GetTextSize() );
    newtext->SetThickness( text->GetThickness() );
    newtext->SetItalic( text->IsItalic() );
    newtext->SetBold( text->IsBold() );
    newtext->SetIsDangling( text->IsDangling() );

    /* Save the new text in undo list if the old text was not itself a "new created text"
     * In this case, the old text is already in undo list as a deleted item.
     * Of course if the old text was a "new created text" the new text will be
     * put in undo list later, at the end of the current command (if not aborted)
     */

    m_canvas->CrossHairOff();   // Erase schematic cursor

    // For an exiting item (i.e. already in list):
    // replace the existing item by the new text in list
    for( SCH_ITEM* item = screen->GetDrawItems(); item != NULL; item = item->Next() )
    {
        if( item == text )
        {
            RemoveFromScreen( text );
            AddToScreen( newtext );
            break;
        }
    }

    SetRepeatItem( NULL );
    OnModify();
    m_canvas->CrossHairOn( );    // redraw schematic cursor

    // if the old item is the current schematic item, replace it by the new text:
    if( screen->GetCurItem() == text )
        screen->SetCurItem( newtext );

    // handle dangling end for the different label/text types
    if( type == SCH_TEXT_T )
    {
        if( newtext->IsDangling() )
        {
            newtext->SetIsDangling( false );
            GetCanvas()->GetView()->Update( newtext, KIGFX::REPAINT );
        }
    }
    else
    {
        newtext->SetIsDangling( true );
    }

    TestDanglingEnds();

    // fix rotation of the converted label, only needed for horizontal labels:
    bool newTypeIsBig = newtext->Type() == SCH_GLOBAL_LABEL_T ||
            newtext->Type() == SCH_HIERARCHICAL_LABEL_T;
    bool oldTypeIsBig = text->Type() == SCH_GLOBAL_LABEL_T ||
            text->Type() == SCH_HIERARCHICAL_LABEL_T;

    if ( ( newTypeIsBig && !oldTypeIsBig ) ||
            ( !newTypeIsBig && oldTypeIsBig ) )
    {
        newtext->MirrorY( newtext->GetPosition().x );
    }

    if( text->IsNew() )
    {
        // if the previous text is new, no undo command to prepare here
        // just delete this previous text.
        delete text;
        return;
    }

    // previous text is not new and we replace text by new text.
    // So this is equivalent to delete text and add newtext
    // If text if being currently edited (i.e. moved)
    // we also save the initial copy of text, and prepare undo command for new text modifications.
    // we must save it as modified text,if it is currently edited, then save as deleted text,
    // and replace text with newtext
    PICKED_ITEMS_LIST pickList;
    ITEM_PICKER picker( text, UR_CHANGED );

    if( text->GetFlags() )
    {
        // text is being edited, save initial text for undo command
        picker.SetLink( GetUndoItem() );
        pickList.PushItem( picker );

        // the owner of undoItem is no more "this", it is now "picker":
        SetUndoItem( NULL );

        // save current newtext copy for undo/abort current command
        SetUndoItem( newtext );
    }

    // Prepare undo command for delete old text
    picker.SetStatus( UR_DELETED );
    picker.SetLink( NULL );
    pickList.PushItem( picker );

    // Prepare undo command for new text
    picker.SetStatus( UR_NEW );
    picker.SetItem(newtext);
    pickList.PushItem( picker );

    SaveCopyInUndoList( pickList, UR_UNSPECIFIED );
}


/* Function to increment bus label members numbers,
 * i.e. when a text is ending with a number, adds
 * aIncrement to this number
 */
void IncrementLabelMember( wxString& name, int aIncrement )
{
    int  ii, nn;
    long number = 0;

    ii = name.Len() - 1; nn = 0;

    if( !wxIsdigit( name.GetChar( ii ) ) )
        return;

    while( (ii >= 0) && wxIsdigit( name.GetChar( ii ) ) )
    {
        ii--; nn++;
    }

    ii++;   /* digits are starting at ii position */
    wxString litt_number = name.Right( nn );

    if( litt_number.ToLong( &number ) )
    {
        number += aIncrement;
        name.Remove( ii ); name << number;
    }
}
