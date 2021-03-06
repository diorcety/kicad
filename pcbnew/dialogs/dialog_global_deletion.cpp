/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <functional>
using namespace std::placeholders;

#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcbnew.h>
#include <pcb_edit_frame.h>
#include <ratsnest_data.h>
#include <board_commit.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <class_zone.h>

#include <tool/tool_manager.h>
#include <tools/pcb_actions.h>

#include <dialog_global_deletion.h>


DIALOG_GLOBAL_DELETION::DIALOG_GLOBAL_DELETION( PCB_EDIT_FRAME* parent ) :
    DIALOG_GLOBAL_DELETION_BASE( parent )
{
    m_Parent = parent;
    m_currentLayer = F_Cu;
    m_TrackFilterAR->Enable( m_DelTracks->GetValue() );
    m_TrackFilterLocked->Enable( m_DelTracks->GetValue() );
    m_TrackFilterNormal->Enable( m_DelTracks->GetValue() );
    m_TrackFilterVias->Enable( m_DelTracks->GetValue() );
    m_ModuleFilterLocked->Enable( m_DelModules->GetValue() );
    m_ModuleFilterNormal->Enable( m_DelModules->GetValue() );
    m_sdbSizer1OK->SetDefault();
    SetFocus();
    GetSizer()->SetSizeHints( this );
    Centre();
}


void PCB_EDIT_FRAME::InstallPcbGlobalDeleteFrame( const wxPoint& pos )
{
    DIALOG_GLOBAL_DELETION dlg( this );
    dlg.SetCurrentLayer( GetActiveLayer() );

    dlg.ShowModal();
}


void DIALOG_GLOBAL_DELETION::SetCurrentLayer( LAYER_NUM aLayer )
{
    m_currentLayer = aLayer;
    m_textCtrlCurrLayer->SetValue( m_Parent->GetBoard()->GetLayerName( ToLAYER_ID( aLayer ) ) );
}


void DIALOG_GLOBAL_DELETION::OnCheckDeleteTracks( wxCommandEvent& event )
{
    m_TrackFilterAR->Enable( m_DelTracks->GetValue() );
    m_TrackFilterLocked->Enable( m_DelTracks->GetValue() );
    m_TrackFilterNormal->Enable( m_DelTracks->GetValue() );
    m_TrackFilterVias->Enable( m_DelTracks->GetValue() );
}


void DIALOG_GLOBAL_DELETION::OnCheckDeleteModules( wxCommandEvent& event )
{
    m_ModuleFilterLocked->Enable( m_DelModules->GetValue() );
    m_ModuleFilterNormal->Enable( m_DelModules->GetValue() );
}


void DIALOG_GLOBAL_DELETION::AcceptPcbDelete()
{
    bool gen_rastnest = false;

    // Clear selection before removing any items
    m_Parent->GetToolManager()->RunAction( PCB_ACTIONS::selectionClear, true );

    m_Parent->SetCurItem( NULL );

    bool delAll = false;

    if( m_DelAlls->GetValue() )
    {
        if( !IsOK( this, _( "Are you sure you want to delete the entire board?" ) ) )
            return;

        delAll = true;
    }
    else if( !IsOK( this, _( "Are you sure you want to delete the selected items?" ) ) )
            return;

    BOARD*            pcb = m_Parent->GetBoard();
    BOARD_COMMIT      commit( m_Parent );
    BOARD_ITEM*       item;

    LSET layers_filter = LSET().set();

    if( m_rbLayersOption->GetSelection() != 0 )     // Use current layer only
        layers_filter = LSET( ToLAYER_ID( m_currentLayer ) );

    if( delAll || m_DelZones->GetValue() )
    {
        int area_index = 0;
        item = pcb->GetArea( area_index );

        while( item )
        {
            if( delAll || layers_filter[item->GetLayer()] )
            {
                commit.Remove( item );
                gen_rastnest = true;
            }

            area_index++;
            item = pcb->GetArea( area_index );
        }
    }

    bool delDrawings = m_DelDrawings->GetValue() || m_DelBoardEdges->GetValue();
    bool delTexts = m_DelTexts->GetValue();

    if( delAll || delDrawings || delTexts )
    {
        // Layer mask for texts
        LSET del_text_layers = layers_filter;

        // Layer mask for drawings
        LSET masque_layer;

        if( m_DelDrawings->GetValue() )
            masque_layer = LSET::AllNonCuMask().set( Edge_Cuts, false );

        if( m_DelBoardEdges->GetValue() )
            masque_layer.set( Edge_Cuts );

        masque_layer &= layers_filter;

        for( auto dwg : pcb->Drawings() )
        {
            KICAD_T type = dwg->Type();
            LAYER_NUM layer = dwg->GetLayer();

            if( delAll
                || ( type == PCB_LINE_T && delDrawings && masque_layer[layer] )
                || ( type == PCB_TEXT_T && delTexts && del_text_layers[layer] ) )
            {
                commit.Remove( dwg );
            }
        }
    }

    if( delAll || m_DelModules->GetValue() )
    {
        for( item = pcb->m_Modules; item; item = item->Next() )
        {
            bool del_fp = delAll;

            if( layers_filter[item->GetLayer()] &&
                ( ( m_ModuleFilterNormal->GetValue() && !item->IsLocked() ) ||
                  ( m_ModuleFilterLocked->GetValue() && item->IsLocked() ) ) )
                del_fp = true;

            if( del_fp )
            {
                commit.Remove( item );
                gen_rastnest = true;
            }
        }
    }

    if( delAll || m_DelTracks->GetValue() )
    {
        STATUS_FLAGS track_mask_filter = 0;

        if( !m_TrackFilterLocked->GetValue() )
            track_mask_filter |= TRACK_LOCKED;

        if( !m_TrackFilterAR->GetValue() )
            track_mask_filter |= TRACK_AR;

        TRACK* nexttrack;

        for( TRACK *track = pcb->m_Track; track; track = nexttrack )
        {
            nexttrack = track->Next();

            if( !delAll )
            {
                if( ( track->GetState( TRACK_LOCKED | TRACK_AR ) & track_mask_filter ) != 0 )
                    continue;

                if( ( track->Type() == PCB_TRACE_T ) &&
                    ( track->GetState( TRACK_LOCKED | TRACK_AR ) == 0 ) &&
                    !m_TrackFilterNormal->GetValue() )
                    continue;

                if( ( track->Type() == PCB_VIA_T ) && !m_TrackFilterVias->GetValue() )
                    continue;

                if( ( track->GetLayerSet() & layers_filter ) == 0 )
                    continue;
            }

            commit.Remove( track );
            gen_rastnest = true;
        }
    }

    commit.Push( "Global delete" );

    if( m_DelMarkers->GetValue() )
        pcb->DeleteMARKERs();

    if( gen_rastnest )
        m_Parent->Compile_Ratsnest( NULL, true );

    // There is a chance that some of tracks have changed their nets, so rebuild ratsnest from scratch
    m_Parent->GetCanvas()->Refresh();
}
