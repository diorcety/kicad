/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
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
 * http://www.gnu.org/licenses/old-licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


/**
 * @file pcbnew/cross-probing.cpp
 * @brief Cross probing functions to handle communication to andfrom Eeschema.
 */

/**
 * Handle messages between Pcbnew and Eeschema via a socket, the port numbers are
 * KICAD_PCB_PORT_SERVICE_NUMBER (currently 4242) (Eeschema to Pcbnew)
 * KICAD_SCH_PORT_SERVICE_NUMBER (currently 4243) (Pcbnew to Eeschema)
 * Note: these ports must be enabled for firewall protection
 */

#include <fctsys.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <kiway_express.h>
#include <pcb_edit_frame.h>
#include <eda_dde.h>
#include <macros.h>

#include <pcbnew_id.h>
#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <class_zone.h>

#include <collectors.h>
#include <pcbnew.h>
#include <board_netlist_updater.h>
#include <netlist_reader.h>
#include <pcb_netlist.h>
#include <dialogs/dialog_update_pcb.h>

#include <tools/pcb_actions.h>
#include <tool/tool_manager.h>
#include <tools/selection_tool.h>
#include <pcb_draw_panel_gal.h>
#include <pcb_painter.h>

/* Execute a remote command send by Eeschema via a socket,
 * port KICAD_PCB_PORT_SERVICE_NUMBER
 * cmdline = received command from Eeschema
 * Commands are
 * $PART: "reference"   put cursor on component
 * $PIN: "pin name"  $PART: "reference" put cursor on the footprint pin
 * $NET: "net name" highlight the given net (if highlight tool is active)
 * They are a keyword followed by a quoted string.
 */
void PCB_EDIT_FRAME::ExecuteRemoteCommand( const char* cmdline )
{
    char        line[1024];
    wxString    msg;
    wxString    modName;
    char*       idcmd;
    char*       text;
    MODULE*     module = NULL;
    D_PAD*      pad = NULL;
    BOARD*      pcb = GetBoard();
    wxPoint     pos;

    strncpy( line, cmdline, sizeof(line) - 1 );
    line[sizeof(line) - 1] = 0;

    idcmd = strtok( line, " \n\r" );
    text  = strtok( NULL, "\"\n\r" );

    if( idcmd == NULL )
        return;

    if( strcmp( idcmd, "$NET:" ) == 0 )
    {
        if( GetToolId() == ID_PCB_HIGHLIGHT_BUTT )
        {
            wxString net_name = FROM_UTF8( text );
            NETINFO_ITEM* netinfo = pcb->FindNet( net_name );
            int netcode = 0;

            if( netinfo )
                netcode = netinfo->GetNet();

            if( IsGalCanvasActive() )
            {
                auto view = m_toolManager->GetView();
                auto rs = view->GetPainter()->GetSettings();
                rs->SetHighlight( true, netcode );
                view->UpdateAllLayersColor();

                BOX2I bbox;
                bool first = true;

                auto merge_area = [netcode, &bbox, &first]( BOARD_CONNECTED_ITEM* aItem )
                {
                    if( aItem->GetNetCode() == netcode )
                    {
                        if( first )
                        {
                            bbox = aItem->GetBoundingBox();
                            first = false;
                        }
                        else
                        {
                            bbox.Merge( aItem->GetBoundingBox() );
                        }
                    }
                };

                for( auto zone : pcb->Zones() )
                    merge_area( zone );

                for( auto track : pcb->Tracks() )
                    merge_area( track );

                for( auto mod : pcb->Modules() )
                    for ( auto mod_pad : mod->Pads() )
                        merge_area( mod_pad );

                if( netcode > 0 && bbox.GetWidth() > 0 && bbox.GetHeight() > 0 )
                {
                    auto bbSize = bbox.Inflate( bbox.GetWidth() * 0.2f ).GetSize();
                    auto screenSize = view->ToWorld( GetGalCanvas()->GetClientSize(), false );
                    double ratio = std::max( fabs( bbSize.x / screenSize.x ),
                                             fabs( bbSize.y / screenSize.y ) );
                    double scale = view->GetScale() / ratio;

                    view->SetScale( scale );
                    view->SetCenter( bbox.Centre() );
                }

                GetGalCanvas()->Refresh();
            }
            else
            {
                if( netcode > 0 )
                {
                    pcb->HighLightON();
                    pcb->SetHighLightNet( netcode );
                }
                else
                {
                    pcb->HighLightOFF();
                    pcb->SetHighLightNet( -1 );
                }
            }
        }

        return;
    }

    if( text == NULL )
        return;

    if( strcmp( idcmd, "$PART:" ) == 0 )
    {
        modName = FROM_UTF8( text );

        module = pcb->FindModuleByReference( modName );

        if( module )
            msg.Printf( _( "%s found" ), modName );
        else
            msg.Printf( _( "%s not found" ), modName );

        SetStatusText( msg );

        if( module )
            pos = module->GetPosition();
    }
    else if( strcmp( idcmd, "$SHEET:" ) == 0 )
    {
        msg.Printf( _( "Selecting all from sheet \"%s\"" ), FROM_UTF8( text ) );
        wxString sheetStamp( FROM_UTF8( text ) );
        SetStatusText( msg );
        GetToolManager()->RunAction( PCB_ACTIONS::selectOnSheetFromEeschema, true,
                                     static_cast<void*>( &sheetStamp ) );
        return;
    }
    else if( strcmp( idcmd, "$PIN:" ) == 0 )
    {
        wxString pinName;
        int      netcode = -1;

        pinName = FROM_UTF8( text );

        text = strtok( NULL, " \n\r" );

        if( text && strcmp( text, "$PART:" ) == 0 )
            text = strtok( NULL, "\"\n\r" );

        modName = FROM_UTF8( text );

        module = pcb->FindModuleByReference( modName );

        if( module )
            pad = module->FindPadByName( pinName );

        if( pad )
        {
            netcode = pad->GetNetCode();

            // put cursor on the pad:
            pos = pad->GetPosition();
        }

        if( netcode > 0 )               // highlight the pad net
        {
            pcb->HighLightON();
            pcb->SetHighLightNet( netcode );
        }
        else
        {
            pcb->HighLightOFF();
            pcb->SetHighLightNet( -1 );
        }

        if( module == NULL )
        {
            msg.Printf( _( "%s not found" ), modName );
        }
        else if( pad == NULL )
        {
            msg.Printf( _( "%s pin %s not found" ), modName, pinName );
            SetCurItem( module );
        }
        else
        {
            msg.Printf( _( "%s pin %s found" ), modName, pinName );
            SetCurItem( pad );
        }

        SetStatusText( msg );
    }

    if( module )  // if found, center the module on screen, and redraw the screen.
    {
        if( IsGalCanvasActive() )
        {
            GetToolManager()->RunAction( PCB_ACTIONS::crossProbeSchToPcb,
                true,
                pad ?
                    static_cast<BOARD_ITEM*>( pad ) :
                    static_cast<BOARD_ITEM*>( module )
                );
        }
        else
        {
            SetCrossHairPosition( pos );
            RedrawScreen( pos, false );
        }
    }
}


std::string FormatProbeItem( BOARD_ITEM* aItem )
{
    MODULE*     module;

    if( !aItem )
        return "$CLEAR: \"HIGHLIGHTED\""; // message to clear highlight state

    switch( aItem->Type() )
    {
    case PCB_MODULE_T:
        module = (MODULE*) aItem;
        return StrPrintf( "$PART: \"%s\"", TO_UTF8( module->GetReference() ) );

    case PCB_PAD_T:
        {
            module = (MODULE*) aItem->GetParent();
            wxString pad = ((D_PAD*)aItem)->GetName();

            return StrPrintf( "$PART: \"%s\" $PAD: \"%s\"",
                     TO_UTF8( module->GetReference() ),
                     TO_UTF8( pad ) );
        }

    case PCB_MODULE_TEXT_T:
        {
            module = static_cast<MODULE*>( aItem->GetParent() );

            TEXTE_MODULE*   text_mod = static_cast<TEXTE_MODULE*>( aItem );

            const char*     text_key;

            /* This can't be a switch since the break need to pull out
             * from the outer switch! */
            if( text_mod->GetType() == TEXTE_MODULE::TEXT_is_REFERENCE )
                text_key = "$REF:";
            else if( text_mod->GetType() == TEXTE_MODULE::TEXT_is_VALUE )
                text_key = "$VAL:";
            else
                break;

            return StrPrintf( "$PART: \"%s\" %s \"%s\"",
                     TO_UTF8( module->GetReference() ),
                     text_key,
                     TO_UTF8( text_mod->GetText() ) );
        }

    default:
        break;
    }

    return "";
}


/* Send a remote command to Eeschema via a socket,
 * aSyncItem = item to be located on schematic (module, pin or text)
 * Commands are
 * $PART: "reference"   put cursor on component anchor
 * $PART: "reference" $PAD: "pad number" put cursor on the component pin
 * $PART: "reference" $REF: "reference" put cursor on the component ref
 * $PART: "reference" $VAL: "value" put cursor on the component value
 */
void PCB_EDIT_FRAME::SendMessageToEESCHEMA( BOARD_ITEM* aSyncItem )
{
    std::string packet = FormatProbeItem( aSyncItem );

    if( packet.size() )
    {
        if( Kiface().IsSingle() )
            SendCommand( MSG_TO_SCH, packet.c_str() );
        else
        {
            // Typically ExpressMail is going to be s-expression packets, but since
            // we have existing interpreter of the cross probe packet on the other
            // side in place, we use that here.
            Kiway().ExpressMail( FRAME_SCH, MAIL_CROSS_PROBE, packet, this );
        }
    }
}


void PCB_EDIT_FRAME::SendCrossProbeNetName( const wxString& aNetName )
{
    std::string packet = StrPrintf( "$NET: \"%s\"", TO_UTF8( aNetName ) );

    if( packet.size() )
    {
        if( Kiface().IsSingle() )
            SendCommand( MSG_TO_SCH, packet.c_str() );
        else
        {
            // Typically ExpressMail is going to be s-expression packets, but since
            // we have existing interpreter of the cross probe packet on the other
            // side in place, we use that here.
            Kiway().ExpressMail( FRAME_SCH, MAIL_CROSS_PROBE, packet, this );
        }
    }
}


void PCB_EDIT_FRAME::KiwayMailIn( KIWAY_EXPRESS& mail )
{
    const std::string& payload = mail.GetPayload();

    switch( mail.Command() )
    {
    case MAIL_CROSS_PROBE:
        ExecuteRemoteCommand( payload.c_str() );
        break;

    case MAIL_SCH_PCB_UPDATE:
    {
        NETLIST netlist;
        size_t split = payload.find( '\n' );
        wxCHECK( split != std::string::npos, /*void*/ );

        // Extract options and netlist
        std::string options = payload.substr( 0, split );
        std::string netlistData = payload.substr( split + 1 );

        // Quiet update options
        bool by_reference = options.find( "by-reference" ) != std::string::npos;
        bool by_timestamp = options.find( "by-timestamp" ) != std::string::npos;
        wxASSERT( !( by_reference && by_timestamp ) );  // only one at a time please

        try
        {
            STRING_LINE_READER* lineReader = new STRING_LINE_READER( netlistData, _( "Eeschema netlist" ) );
            KICAD_NETLIST_READER netlistReader( lineReader, &netlist );
            netlistReader.LoadNetlist();
        }
        catch( const IO_ERROR& )
        {
            assert( false ); // should never happen
        }

        if( by_reference || by_timestamp )
        {
            netlist.SetDeleteExtraFootprints( false );
            netlist.SetFindByTimeStamp( by_timestamp );
            netlist.SetReplaceFootprints( true );

            BOARD_NETLIST_UPDATER updater( this, GetBoard() );
            updater.SetLookupByTimestamp( by_timestamp );
            updater.SetDeleteUnusedComponents( false );
            updater.SetReplaceFootprints( true );
            updater.SetDeleteSinglePadNets( false );
            updater.UpdateNetlist( netlist );
        }
        else
        {
            DIALOG_UPDATE_PCB updateDialog( this, &netlist );
            updateDialog.ShowModal();
        }

        break;
    }

    case MAIL_IMPORT_FILE:
    {
        // Extract file format type and path (plugin type and path separated with \n)
        size_t split = payload.find( '\n' );
        wxCHECK( split != std::string::npos, /*void*/ );
        int importFormat;

        try
        {
            importFormat = std::stoi( payload.substr( 0, split ) );
        }
        catch( std::invalid_argument& )
        {
            wxFAIL;
            importFormat = -1;
        }

        std::string path = payload.substr( split + 1 );
        wxASSERT( !path.empty() );

        if( importFormat >= 0 )
            importFile( path, importFormat );
    }

    // many many others.
    default:
        ;
    }
}

