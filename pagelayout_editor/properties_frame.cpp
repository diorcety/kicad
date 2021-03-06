/**
 * @file properties_frame.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Jean-Pierre Charras, jp.charras at wanadoo.fr
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

#include <fctsys.h>
#include <class_drawpanel.h>
#include <worksheet_shape_builder.h>
#include <worksheet_dataitem.h>
#include <properties_frame.h>

PROPERTIES_FRAME::PROPERTIES_FRAME( PL_EDITOR_FRAME* aParent ):
    PANEL_PROPERTIES_BASE( aParent )
{
    m_parent = aParent;
}


PROPERTIES_FRAME::~PROPERTIES_FRAME()
{
}

wxSize PROPERTIES_FRAME::GetMinSize() const
{
    return wxSize( 150, -1 );
}


// Data transfert from general properties to widgets
void PROPERTIES_FRAME::CopyPrmsFromGeneralToPanel()
{
    wxString msg;

    // Set default parameters
    msg.Printf( wxT("%.3f"),  WORKSHEET_DATAITEM::m_DefaultLineWidth );
    m_textCtrlDefaultLineWidth->SetValue( msg );

    msg.Printf( wxT("%.3f"), WORKSHEET_DATAITEM::m_DefaultTextSize.x );
    m_textCtrlDefaultTextSizeX->SetValue( msg );
    msg.Printf( wxT("%.3f"),  WORKSHEET_DATAITEM::m_DefaultTextSize.y );
    m_textCtrlDefaultTextSizeY->SetValue( msg );

    msg.Printf( wxT("%.3f"),  WORKSHEET_DATAITEM::m_DefaultTextThickness );
    m_textCtrlDefaultTextThickness->SetValue( msg );

    // Set page margins values
    WORKSHEET_LAYOUT& pglayout = WORKSHEET_LAYOUT::GetTheInstance();
    msg.Printf( wxT("%.3f"),  pglayout.GetRightMargin() );
    m_textCtrlRightMargin->SetValue( msg );
    msg.Printf( wxT("%.3f"),  pglayout.GetBottomMargin() );
    m_textCtrlDefaultBottomMargin->SetValue( msg );

    msg.Printf( wxT("%.3f"),  pglayout.GetLeftMargin() );
    m_textCtrlLeftMargin->SetValue( msg );
    msg.Printf( wxT("%.3f"),  pglayout.GetTopMargin() );
    m_textCtrlTopMargin->SetValue( msg );
}

// Data transfert from widgets to general properties
bool PROPERTIES_FRAME::CopyPrmsFromPanelToGeneral()
{
    wxString msg;

    // Import default parameters from widgets
    msg = m_textCtrlDefaultLineWidth->GetValue();
    WORKSHEET_DATAITEM::m_DefaultLineWidth = DoubleValueFromString( UNSCALED_UNITS, msg );

    msg = m_textCtrlDefaultTextSizeX->GetValue();
    WORKSHEET_DATAITEM::m_DefaultTextSize.x = DoubleValueFromString( UNSCALED_UNITS, msg );
    msg = m_textCtrlDefaultTextSizeY->GetValue();
    WORKSHEET_DATAITEM::m_DefaultTextSize.y = DoubleValueFromString( UNSCALED_UNITS, msg );

    msg = m_textCtrlDefaultTextThickness->GetValue();
    WORKSHEET_DATAITEM::m_DefaultTextThickness = DoubleValueFromString( UNSCALED_UNITS, msg );

    // Get page margins values
    WORKSHEET_LAYOUT& pglayout = WORKSHEET_LAYOUT::GetTheInstance();

    msg = m_textCtrlRightMargin->GetValue();
    pglayout.SetRightMargin( DoubleValueFromString( UNSCALED_UNITS, msg ) );
    msg = m_textCtrlDefaultBottomMargin->GetValue();
    pglayout.SetBottomMargin( DoubleValueFromString( UNSCALED_UNITS, msg ) );

    // cordinates of the left top corner are the left and top margins
    msg = m_textCtrlLeftMargin->GetValue();
    pglayout.SetLeftMargin( DoubleValueFromString( UNSCALED_UNITS, msg ) );
    msg = m_textCtrlTopMargin->GetValue();
    pglayout.SetTopMargin( DoubleValueFromString( UNSCALED_UNITS, msg ) );

    return true;
}

// Data transfert from item to widgets in properties frame
void PROPERTIES_FRAME::CopyPrmsFromItemToPanel( WORKSHEET_DATAITEM* aItem )
{
    wxString msg;

    // Set parameters common to all WORKSHEET_DATAITEM types
    m_textCtrlType->SetValue( aItem->GetClassName() );
    m_textCtrlComment->SetValue( aItem->m_Info );

    switch( aItem->GetPage1Option() )
    {
       default:
        case 0:
            m_choicePageOpt->SetSelection( 0 );
            break;

        case 1:
            m_choicePageOpt->SetSelection( 1 );
            break;

        case -1:
            m_choicePageOpt->SetSelection( 2 );
            break;
    }

    // Position/ start point
    msg.Printf( wxT("%.3f"), aItem->m_Pos.m_Pos.x );
    m_textCtrlPosX->SetValue( msg );
    msg.Printf( wxT("%.3f"), aItem->m_Pos.m_Pos.y );
    m_textCtrlPosY->SetValue( msg );

    switch(  aItem->m_Pos.m_Anchor )
    {
        case RB_CORNER:      // right bottom corner
            m_comboBoxCornerPos->SetSelection( 2 ); break;
        case RT_CORNER:      // right top corner
            m_comboBoxCornerPos->SetSelection( 0 ); break;
        case LB_CORNER:      // left bottom corner
            m_comboBoxCornerPos->SetSelection( 3 ); break;
        case LT_CORNER:      // left top corner
            m_comboBoxCornerPos->SetSelection( 1 ); break;
    }

    // End point
    msg.Printf( wxT("%.3f"), aItem->m_End.m_Pos.x );
    m_textCtrlEndX->SetValue( msg );
    msg.Printf( wxT("%.3f"), aItem->m_End.m_Pos.y );
    m_textCtrlEndY->SetValue( msg );

    switch( aItem->m_End.m_Anchor )
    {
        case RB_CORNER:      // right bottom corner
            m_comboBoxCornerEnd->SetSelection( 2 ); break;
        case RT_CORNER:      // right top corner
            m_comboBoxCornerEnd->SetSelection( 0 ); break;
        case LB_CORNER:      // left bottom corner
            m_comboBoxCornerEnd->SetSelection( 3 ); break;
        case LT_CORNER:      // left top corner
            m_comboBoxCornerEnd->SetSelection( 1 ); break;
    }

    msg.Printf( wxT("%.3f"), aItem->m_LineWidth );
    m_textCtrlThickness->SetValue( msg );

    // Now, set prms more specific to WORKSHEET_DATAITEM types
    // For a given type, disable widgets which are not relevant,
    // and be sure widgets which are relevant are enabled
    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_TEXT )
    {
        m_SizerTextOptions->Show( true );
        m_staticTextInclabel->Show( true );
        m_textCtrlTextIncrement->Show( true );

        WORKSHEET_DATAITEM_TEXT* item = (WORKSHEET_DATAITEM_TEXT*) aItem;
        item->m_FullText = item->m_TextBase;
        // Replace our '\' 'n' sequence by the EOL char
        item->ReplaceAntiSlashSequence();
        m_textCtrlText->SetValue( item->m_FullText );

        msg.Printf( wxT("%d"), item->m_IncrementLabel );
        m_textCtrlTextIncrement->SetValue( msg );

        // Rotation (poly and text)
        msg.Printf( wxT("%.3f"), item->m_Orient );
        m_textCtrlRotation->SetValue( msg );

        // Constraints:
        msg.Printf( wxT("%.3f"), item->m_BoundingBoxSize.x );
        m_textCtrlConstraintX->SetValue( msg );
        msg.Printf( wxT("%.3f"), item->m_BoundingBoxSize.y );
        m_textCtrlConstraintY->SetValue( msg );

        // Font Options
		m_checkBoxBold->SetValue( item->IsBold() );
		m_checkBoxItalic->SetValue( item->IsItalic() );
        switch( item->m_Hjustify )
        {
            case GR_TEXT_HJUSTIFY_LEFT: m_choiceHjustify->SetSelection( 0 ); break;
            case GR_TEXT_HJUSTIFY_CENTER: m_choiceHjustify->SetSelection( 1 ); break;
            case GR_TEXT_HJUSTIFY_RIGHT: m_choiceHjustify->SetSelection( 2 ); break;
        }
        switch( item->m_Vjustify )
        {
            case GR_TEXT_VJUSTIFY_TOP: m_choiceVjustify->SetSelection( 0 ); break;
            case GR_TEXT_VJUSTIFY_CENTER: m_choiceVjustify->SetSelection( 1 ); break;
            case GR_TEXT_VJUSTIFY_BOTTOM: m_choiceVjustify->SetSelection( 2 ); break;
        }

        // Text size
        msg.Printf( wxT("%.3f"), item->m_TextSize.x );
        m_textCtrlTextSizeX->SetValue( msg );
        msg.Printf( wxT("%.3f"), item->m_TextSize.y );
        m_textCtrlTextSizeY->SetValue( msg );
    }
    else
    {
        m_SizerTextOptions->Show( false );
        m_staticTextInclabel->Show( false );
        m_textCtrlTextIncrement->Show( false );
    }

    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_POLYPOLYGON )
    {
        WORKSHEET_DATAITEM_POLYPOLYGON* item = (WORKSHEET_DATAITEM_POLYPOLYGON*) aItem;
        // Rotation (poly and text)
        msg.Printf( wxT("%.3f"), item->m_Orient );
        m_textCtrlRotation->SetValue( msg );
    }

    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_BITMAP )
    {
        WORKSHEET_DATAITEM_BITMAP* item = (WORKSHEET_DATAITEM_BITMAP*) aItem;
        // select definition in PPI
        msg.Printf( wxT("%d"), item->GetPPI() );
        m_textCtrlBitmapPPI->SetValue( msg );
    }

    switch( aItem->GetType() )
    {
        case WORKSHEET_DATAITEM::WS_SEGMENT:
        case WORKSHEET_DATAITEM::WS_RECT:
            m_SizerRotAndPPI->Show( false );
            m_SizerLineThickness->Show( true );
            m_staticTextInfoThickness->Show( true );
            m_SizerEndPosition->Show(true);
            break;

        case WORKSHEET_DATAITEM::WS_TEXT:
            m_SizerRotAndPPI->Show( true );
            m_staticTextRot->Show( true );
            m_textCtrlRotation->Show( true );
            m_staticTextBitmapPPI->Show( false );
            m_textCtrlBitmapPPI->Show( false );
            m_SizerLineThickness->Show( true );
            m_staticTextInfoThickness->Show( true );
            m_SizerEndPosition->Show(false);
            break;

        case WORKSHEET_DATAITEM::WS_POLYPOLYGON:
            m_SizerRotAndPPI->Show( true );
            m_staticTextRot->Show( true );
            m_textCtrlRotation->Show( true );
            m_staticTextBitmapPPI->Show( false );
            m_textCtrlBitmapPPI->Show( false );
            m_SizerLineThickness->Show( true );
            m_staticTextInfoThickness->Show( false );   // No defaut value for thickness
            m_SizerEndPosition->Show(false);
            break;

        case WORKSHEET_DATAITEM::WS_BITMAP:
            m_SizerRotAndPPI->Show( true );
            m_staticTextRot->Show( false );
            m_textCtrlRotation->Show( false );
            m_staticTextBitmapPPI->Show( true );
            m_textCtrlBitmapPPI->Show( true );
            m_SizerLineThickness->Show( false );
            m_SizerEndPosition->Show(false);
            break;
    }

    // Repeat parameters
    msg.Printf( wxT("%d"), aItem->m_RepeatCount );
    m_textCtrlRepeatCount->SetValue( msg );
    msg.Printf( wxT("%.3f"), aItem->m_IncrementVector.x );
    m_textCtrlStepX->SetValue( msg );
    msg.Printf( wxT("%.3f"), aItem->m_IncrementVector.y );
    m_textCtrlStepY->SetValue( msg );

    // The number of widgets was modified
    m_swItemProperties->Layout();
#ifdef __WXGTK__
    // This call is mandatory on wxGTK to initialize the right virtual size and therefore
    // scrollbars, but for some reason, create issues on Windows (incorrect disply
    // until the frame is resized). Joys of multiplatform dev.
    m_swItemProperties->Fit();
#endif
    // send a size event to be sure scrollbars will be added/removed as needed
    m_swItemProperties->PostSizeEvent();
    m_swItemProperties->Refresh();
}

// Event function called by clicking on the OK button
void PROPERTIES_FRAME::OnAcceptPrms( wxCommandEvent& event )
{
    m_parent->SaveCopyInUndoList();

    WORKSHEET_DATAITEM* item = m_parent->GetSelectedItem();
    if( item )
    {
        CopyPrmsFromPanelToItem( item );
        // Be sure what is displayed is what is set for item
        // (mainly, texts can be modified if they contain "\n")
        CopyPrmsFromItemToPanel( item );
    }

    CopyPrmsFromPanelToGeneral();

    // Refresh values, exactly as they are converted, to avoid any mistake
    CopyPrmsFromGeneralToPanel();

    m_parent->OnModify();
    m_parent->GetCanvas()->Refresh();
}

void PROPERTIES_FRAME::OnSetDefaultValues( wxCommandEvent& event )
{
    WORKSHEET_DATAITEM::m_DefaultTextSize =
            DSIZE( TB_DEFAULT_TEXTSIZE, TB_DEFAULT_TEXTSIZE );
    // default thickness in mm
    WORKSHEET_DATAITEM::m_DefaultLineWidth = 0.15;
    WORKSHEET_DATAITEM::m_DefaultTextThickness = 0.15;

    CopyPrmsFromGeneralToPanel();
    m_parent->GetCanvas()->Refresh();
}


// Data transfert from  properties frame to item parameters
bool PROPERTIES_FRAME::CopyPrmsFromPanelToItem( WORKSHEET_DATAITEM* aItem )
{
    if( aItem == NULL )
        return false;

    wxString msg;

    // Import common parameters:
    aItem->m_Info = m_textCtrlComment->GetValue();

    switch( m_choicePageOpt->GetSelection() )
    {
        default:
        case 0:
            aItem->SetPage1Option( 0 );
            break;

        case 1:
            aItem->SetPage1Option( 1 );
            break;

        case 2:
            aItem->SetPage1Option( -1 );
            break;
    }

    // Import thickness
    msg = m_textCtrlThickness->GetValue();
    aItem->m_LineWidth = DoubleValueFromString( UNSCALED_UNITS, msg );

    // Import Start point
    msg = m_textCtrlPosX->GetValue();
    aItem->m_Pos.m_Pos.x = DoubleValueFromString( UNSCALED_UNITS, msg );

    msg = m_textCtrlPosY->GetValue();
    aItem->m_Pos.m_Pos.y = DoubleValueFromString( UNSCALED_UNITS, msg );

    switch( m_comboBoxCornerPos->GetSelection() )
    {
        case 2: aItem->m_Pos.m_Anchor = RB_CORNER; break;
        case 0: aItem->m_Pos.m_Anchor = RT_CORNER; break;
        case 3: aItem->m_Pos.m_Anchor = LB_CORNER; break;
        case 1: aItem->m_Pos.m_Anchor = LT_CORNER; break;
    }

    // Import End point
    msg = m_textCtrlEndX->GetValue();
    aItem->m_End.m_Pos.x = DoubleValueFromString( UNSCALED_UNITS, msg );

    msg = m_textCtrlEndY->GetValue();
    aItem->m_End.m_Pos.y = DoubleValueFromString( UNSCALED_UNITS, msg );

    switch( m_comboBoxCornerEnd->GetSelection() )
    {
        case 2: aItem->m_End.m_Anchor = RB_CORNER; break;
        case 0: aItem->m_End.m_Anchor = RT_CORNER; break;
        case 3: aItem->m_End.m_Anchor = LB_CORNER; break;
        case 1: aItem->m_End.m_Anchor = LT_CORNER; break;
    }

    // Import Repeat prms
    long itmp;
    msg = m_textCtrlRepeatCount->GetValue();
    msg.ToLong( &itmp );
    aItem->m_RepeatCount = itmp;

    msg = m_textCtrlStepX->GetValue();
    aItem->m_IncrementVector.x = DoubleValueFromString( UNSCALED_UNITS, msg );

    msg = m_textCtrlStepY->GetValue();
    aItem->m_IncrementVector.y = DoubleValueFromString( UNSCALED_UNITS, msg );

    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_TEXT )
    {
        WORKSHEET_DATAITEM_TEXT* item = (WORKSHEET_DATAITEM_TEXT*) aItem;

        item->m_TextBase = m_textCtrlText->GetValue();

        msg = m_textCtrlTextIncrement->GetValue();
        msg.ToLong( &itmp );
        item->m_IncrementLabel = itmp;

        item->SetBold( m_checkBoxBold->IsChecked() );
        item->SetItalic( m_checkBoxItalic->IsChecked() );

        switch( m_choiceHjustify->GetSelection() )
        {
            case 0: item->m_Hjustify = GR_TEXT_HJUSTIFY_LEFT; break;
            case 1: item->m_Hjustify = GR_TEXT_HJUSTIFY_CENTER; break;
            case 2: item->m_Hjustify = GR_TEXT_HJUSTIFY_RIGHT; break;
        }
        switch( m_choiceVjustify->GetSelection() )
        {
            case 0: item->m_Vjustify = GR_TEXT_VJUSTIFY_TOP; break;
            case 1: item->m_Vjustify = GR_TEXT_VJUSTIFY_CENTER; break;
            case 2: item->m_Vjustify = GR_TEXT_VJUSTIFY_BOTTOM; break;
        }

        msg = m_textCtrlRotation->GetValue();
        item->m_Orient = DoubleValueFromString( UNSCALED_UNITS, msg );

        // Import text size
        msg = m_textCtrlTextSizeX->GetValue();
        item->m_TextSize.x = DoubleValueFromString( UNSCALED_UNITS, msg );

        msg = m_textCtrlTextSizeY->GetValue();
        item->m_TextSize.y = DoubleValueFromString( UNSCALED_UNITS, msg );

        // Import constraints:
        msg = m_textCtrlConstraintX->GetValue();
        item->m_BoundingBoxSize.x = DoubleValueFromString( UNSCALED_UNITS, msg );

        msg = m_textCtrlConstraintY->GetValue();
        item->m_BoundingBoxSize.y = DoubleValueFromString( UNSCALED_UNITS, msg );
    }

    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_POLYPOLYGON )
    {
        WORKSHEET_DATAITEM_POLYPOLYGON* item = (WORKSHEET_DATAITEM_POLYPOLYGON*) aItem;

        msg = m_textCtrlRotation->GetValue();
        item->m_Orient = DoubleValueFromString( UNSCALED_UNITS, msg );
    }

    if( aItem->GetType() == WORKSHEET_DATAITEM::WS_BITMAP )
    {
        WORKSHEET_DATAITEM_BITMAP* item = (WORKSHEET_DATAITEM_BITMAP*) aItem;
        // Set definition in PPI
        long value;
        msg = m_textCtrlBitmapPPI->GetValue();
        if( msg.ToLong( &value ) )
            item->SetPPI( (int)value );
    }

    return true;
}

