/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file basepcbframe.cpp
 */

#include <fctsys.h>
#include <kiface_i.h>
#include <eda_base_frame.h>
#include <confirm.h>
#include <kiface_i.h>
#include <dialog_helpers.h>
#include <kicad_device_context.h>
#include <pcb_base_frame.h>
#include <base_units.h>
#include <msgpanel.h>
#include <pgm_base.h>
#include <3d_viewer/eda_3d_viewer.h>                                            // To include VIEWER3D_FRAMENAME

#include <pcbnew.h>
#include <fp_lib_table.h>
#include <pcbnew_id.h>
#include <class_board.h>
#include <class_track.h>
#include <class_module.h>
#include <class_drawsegment.h>

#include <collectors.h>
#include <class_drawpanel.h>
#include <pcb_draw_panel_gal.h>
#include <pcb_view.h>
#include <math/vector2d.h>
#include <trigo.h>
#include <pcb_painter.h>

#include <tool/tool_manager.h>
#include <tool/tool_dispatcher.h>
#include <tools/pcb_actions.h>

const wxChar PCB_BASE_FRAME::AUTO_ZOOM_KEY[] = wxT( "AutoZoom" );
const wxChar PCB_BASE_FRAME::ZOOM_KEY[] = wxT( "Zoom" );

// Configuration entry names.
static const wxChar UserGridSizeXEntry[] = wxT( "PcbUserGrid_X" );
static const wxChar UserGridSizeYEntry[] = wxT( "PcbUserGrid_Y" );
static const wxChar UserGridUnitsEntry[] = wxT( "PcbUserGrid_Unit" );
static const wxChar DisplayPadFillEntry[] = wxT( "DiPadFi" );
static const wxChar DisplayViaFillEntry[] = wxT( "DiViaFi" );
static const wxChar DisplayPadNumberEntry[] = wxT( "DiPadNu" );
static const wxChar DisplayModuleEdgeEntry[] = wxT( "DiModEd" );
static const wxChar DisplayModuleTextEntry[] = wxT( "DiModTx" );
static const wxChar FastGrid1Entry[] = wxT( "FastGrid1" );
static const wxChar FastGrid2Entry[] = wxT( "FastGrid2" );


BEGIN_EVENT_TABLE( PCB_BASE_FRAME, EDA_DRAW_FRAME )
    EVT_MENU_RANGE( ID_POPUP_PCB_ITEM_SELECTION_START, ID_POPUP_PCB_ITEM_SELECTION_END,
                    PCB_BASE_FRAME::ProcessItemSelection )

    EVT_TOOL( ID_TB_OPTIONS_SHOW_POLAR_COORD, PCB_BASE_FRAME::OnTogglePolarCoords )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_PADS_SKETCH, PCB_BASE_FRAME::OnTogglePadDrawMode )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_GRAPHIC_SKETCH, PCB_BASE_FRAME::OnToggleGraphicDrawMode )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_MODULE_EDGE_SKETCH, PCB_BASE_FRAME::OnToggleEdgeDrawMode )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_MODULE_TEXT_SKETCH, PCB_BASE_FRAME::OnToggleTextDrawMode )

    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_POLAR_COORD, PCB_BASE_FRAME::OnUpdateCoordType )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_PADS_SKETCH, PCB_BASE_FRAME::OnUpdatePadDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_GRAPHIC_SKETCH, PCB_BASE_FRAME::OnUpdateGraphicDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_MODULE_EDGE_SKETCH, PCB_BASE_FRAME::OnUpdateEdgeDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_MODULE_TEXT_SKETCH, PCB_BASE_FRAME::OnUpdateTextDrawMode )
    EVT_UPDATE_UI( ID_ON_GRID_SELECT, PCB_BASE_FRAME::OnUpdateSelectGrid )
    EVT_UPDATE_UI( ID_ON_ZOOM_SELECT, PCB_BASE_FRAME::OnUpdateSelectZoom )
    // Switching canvases
    EVT_UPDATE_UI( ID_MENU_CANVAS_LEGACY, PCB_BASE_FRAME::OnUpdateSwitchCanvas )
    EVT_UPDATE_UI( ID_MENU_CANVAS_CAIRO, PCB_BASE_FRAME::OnUpdateSwitchCanvas )
    EVT_UPDATE_UI( ID_MENU_CANVAS_OPENGL, PCB_BASE_FRAME::OnUpdateSwitchCanvas )

    EVT_UPDATE_UI_RANGE( ID_ZOOM_IN, ID_ZOOM_PAGE, PCB_BASE_FRAME::OnUpdateSelectZoom )
END_EVENT_TABLE()


PCB_BASE_FRAME::PCB_BASE_FRAME( KIWAY* aKiway, wxWindow* aParent, FRAME_T aFrameType,
        const wxString& aTitle, const wxPoint& aPos, const wxSize& aSize,
        long aStyle, const wxString & aFrameName ) :
    EDA_DRAW_FRAME( aKiway, aParent, aFrameType, aTitle, aPos, aSize, aStyle, aFrameName ),
    m_Pcb( nullptr ),
    m_configSettings( aFrameType )
{
    m_UserGridSize        = wxPoint( (int) 10 * IU_PER_MILS, (int) 10 * IU_PER_MILS );
    m_Collector           = new GENERAL_COLLECTOR();

    m_FastGrid1           = 0;
    m_FastGrid2           = 0;

    m_zoomLevelCoeff      = 11.0 * IU_PER_MILS;  // Adjusted to roughly displays zoom level = 1
                                        // when the screen shows a 1:1 image
                                        // obviously depends on the monitor,
                                        // but this is an acceptable value
}


PCB_BASE_FRAME::~PCB_BASE_FRAME()
{
    // Ensure m_canvasType is up to date, to save it in config
    if( !IsGalCanvasActive() )
        m_canvasType = EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE;
    else
        m_canvasType = GetGalCanvas()->GetBackend();

    delete m_Collector;
    delete m_Pcb;
}


EDA_3D_VIEWER* PCB_BASE_FRAME::Get3DViewerFrame()
{
    // return the 3D viewer frame, when exists, or NULL
    return static_cast<EDA_3D_VIEWER*>
        ( wxWindow::FindWindowByName( VIEWER3D_FRAMENAME ) );
}


bool PCB_BASE_FRAME::Update3DView( const wxString* aTitle )
{
    // Update the 3D view only if the viewer is opened by this frame
    EDA_3D_VIEWER* draw3DFrame = Get3DViewerFrame();

    if( draw3DFrame == NULL )
        return false;

    // Ensure the viewer was created by me, and not by another editor:
    PCB_BASE_FRAME* owner = draw3DFrame->Parent();

    // if I am not the owner, do not use the current viewer instance
    if( this != owner )
        return false;

    if( aTitle )
        draw3DFrame->SetTitle( *aTitle );

    // The 3D view update can be time consumming to rebuild a board 3D view.
    // So do not use a immediate update in the board editor
    bool immediate_update = true;

    if( IsType( FRAME_PCB ) )
        immediate_update = false;

    draw3DFrame->NewDisplay( immediate_update );

    return true;
}


FP_LIB_TABLE* PROJECT::PcbFootprintLibs()
{
    // This is a lazy loading function, it loads the project specific table when
    // that table is asked for, not before.

    FP_LIB_TABLE*   tbl = (FP_LIB_TABLE*) GetElem( ELEM_FPTBL );

    // its gotta be NULL or a FP_LIB_TABLE, or a bug.
    wxASSERT( !tbl || tbl->Type() == FP_LIB_TABLE_T );

    if( !tbl )
    {
        // Stack the project specific FP_LIB_TABLE overlay on top of the global table.
        // ~FP_LIB_TABLE() will not touch the fallback table, so multiple projects may
        // stack this way, all using the same global fallback table.
        tbl = new FP_LIB_TABLE( &GFootprintTable );

        SetElem( ELEM_FPTBL, tbl );

        wxString projectFpLibTableFileName = FootprintLibTblName();

        try
        {
            tbl->Load( projectFpLibTableFileName );
        }
        catch( const IO_ERROR& ioe )
        {
            DisplayErrorMessage( nullptr,
                                 _( "Error loading project footprint libraries" ),
                                 ioe.What() );
        }
    }

    return tbl;
}


void PCB_BASE_FRAME::SetBoard( BOARD* aBoard )
{
    if( m_Pcb != aBoard )
    {
        delete m_Pcb;
        m_Pcb = aBoard;
        m_Pcb->SetColorsSettings( &Settings().Colors() );
    }
}


void PCB_BASE_FRAME::AddModuleToBoard( MODULE* module )
{
    if( module )
    {
        GetBoard()->Add( module, ADD_APPEND );

        module->SetFlags( IS_NEW );

        if( IsGalCanvasActive() )
            module->SetPosition( wxPoint( 0, 0 ) ); // cursor in GAL may not be initialized at the moment
        else
            module->SetPosition( GetCrossHairPosition() );

        module->SetTimeStamp( GetNewTimeStamp() );
        GetBoard()->m_Status_Pcb = 0;

        // Put it on FRONT layer,
        // (Can be stored flipped if the lib is an archive built from a board)
        if( module->IsFlipped() )
            module->Flip( module->GetPosition() );

        // Place it in orientation 0,
        // even if it is not saved with orientation 0 in lib
        // (Can happen if the lib is an archive built from a board)
        module->SetOrientation( 0 );
    }
}


void PCB_BASE_FRAME::SetPageSettings( const PAGE_INFO& aPageSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetPageSettings( aPageSettings );

    if( GetScreen() )
        GetScreen()->InitDataPoints( aPageSettings.GetSizeIU() );
}


const PAGE_INFO& PCB_BASE_FRAME::GetPageSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetPageSettings();
}


const wxSize PCB_BASE_FRAME::GetPageSizeIU() const
{
    wxASSERT( m_Pcb );

    // this function is only needed because EDA_DRAW_FRAME is not compiled
    // with either -DPCBNEW or -DEESCHEMA, so the virtual is used to route
    // into an application specific source file.
    return m_Pcb->GetPageSettings().GetSizeIU();
}


const wxPoint& PCB_BASE_FRAME::GetAuxOrigin() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetAuxOrigin();
}


void PCB_BASE_FRAME::SetAuxOrigin( const wxPoint& aPoint )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetAuxOrigin( aPoint );
}


const wxPoint& PCB_BASE_FRAME::GetGridOrigin() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetGridOrigin();
}


void PCB_BASE_FRAME::SetGridOrigin( const wxPoint& aPoint )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetGridOrigin( aPoint );
}


const TITLE_BLOCK& PCB_BASE_FRAME::GetTitleBlock() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetTitleBlock();
}


void PCB_BASE_FRAME::SetTitleBlock( const TITLE_BLOCK& aTitleBlock )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetTitleBlock( aTitleBlock );
}


BOARD_DESIGN_SETTINGS& PCB_BASE_FRAME::GetDesignSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetDesignSettings();
}


void PCB_BASE_FRAME::SetDesignSettings( const BOARD_DESIGN_SETTINGS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetDesignSettings( aSettings );
}


void PCB_BASE_FRAME::SetDrawBgColor( COLOR4D aColor )
{
    m_drawBgColor= aColor;
    m_auimgr.Update();
}


const ZONE_SETTINGS& PCB_BASE_FRAME::GetZoneSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetZoneSettings();
}


void PCB_BASE_FRAME::SetZoneSettings( const ZONE_SETTINGS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetZoneSettings( aSettings );
}


const PCB_PLOT_PARAMS& PCB_BASE_FRAME::GetPlotSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetPlotOptions();
}


void PCB_BASE_FRAME::SetPlotSettings( const PCB_PLOT_PARAMS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetPlotOptions( aSettings );
}


EDA_RECT PCB_BASE_FRAME::GetBoardBoundingBox( bool aBoardEdgesOnly ) const
{
    wxASSERT( m_Pcb );

    EDA_RECT area = aBoardEdgesOnly ? m_Pcb->GetBoardEdgesBoundingBox() : m_Pcb->GetBoundingBox();

    if( area.GetWidth() == 0 && area.GetHeight() == 0 )
    {
        wxSize pageSize = GetPageSizeIU();

        if( m_showBorderAndTitleBlock )
        {
            area.SetOrigin( 0, 0 );
            area.SetEnd( pageSize.x, pageSize.y );
        }
        else
        {
            area.SetOrigin( -pageSize.x / 2, -pageSize.y / 2 );
            area.SetEnd( pageSize.x / 2, pageSize.y / 2 );
        }
    }

    return area;
}


double PCB_BASE_FRAME::BestZoom()
{
    EDA_RECT    ibbbox  = GetBoardBoundingBox();

    double  sizeX = (double) ibbbox.GetWidth();
    double  sizeY = (double) ibbbox.GetHeight();
    wxPoint centre = ibbbox.Centre();

    // Reserve a 10% margin around board bounding box.
    double margin_scale_factor = 1.1;

    return bestZoom( sizeX, sizeY, margin_scale_factor, centre );
}


// Virtual function
void PCB_BASE_FRAME::ReCreateMenuBar()
{
}


void PCB_BASE_FRAME::ShowChangedLanguage()
{
    // call my base class
    EDA_DRAW_FRAME::ShowChangedLanguage();

    // tooltips in toolbars
    ReCreateHToolbar();
    ReCreateAuxiliaryToolbar();
    ReCreateVToolbar();
    ReCreateOptToolbar();

    // status bar
    UpdateMsgPanel();
}


// Virtual functions: Do nothing for PCB_BASE_FRAME window
void PCB_BASE_FRAME::Show3D_Frame( wxCommandEvent& event )
{
}


bool PCB_BASE_FRAME::CreateAndShow3D_Frame( bool aForceRecreateIfNotOwner )
{
    EDA_3D_VIEWER* draw3DFrame = Get3DViewerFrame();

    // Ensure the viewer was created by me, and not by another editor:
    PCB_BASE_FRAME* owner = draw3DFrame ? draw3DFrame->Parent() : nullptr;

    // if I am not the owner, do not use the current viewer instance
    if( draw3DFrame && this != owner )
    {
        if( aForceRecreateIfNotOwner )
        {
            draw3DFrame->Destroy();
            draw3DFrame = nullptr;
        }
        else
            return false;
    }

    if( !draw3DFrame )
    {
        draw3DFrame = new EDA_3D_VIEWER( &Kiway(), this, _( "3D Viewer" ) );
        draw3DFrame->Raise();     // Needed with some Window Managers
        draw3DFrame->Show( true );
        return true;
    }

    // Raising the window does not show the window on Windows if iconized.
    // This should work on any platform.
    if( draw3DFrame->IsIconized() )
         draw3DFrame->Iconize( false );

    draw3DFrame->Raise();

    // Raising the window does not set the focus on Linux.  This should work on any platform.
    if( wxWindow::FindFocus() != draw3DFrame )
        draw3DFrame->SetFocus();

    return true;
}


// Note: virtual, overridden in PCB_EDIT_FRAME;
void PCB_BASE_FRAME::SwitchLayer( wxDC* DC, PCB_LAYER_ID layer )
{
    PCB_LAYER_ID preslayer = GetActiveLayer();
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    // Check if the specified layer matches the present layer
    if( layer == preslayer )
        return;

    // Copper layers cannot be selected unconditionally; how many
    // of those layers are currently enabled needs to be checked.
    if( IsCopperLayer( layer ) )
    {
        // If only one copper layer is enabled, the only such layer
        // that can be selected to is the "Copper" layer (so the
        // selection of any other copper layer is disregarded).
        if( m_Pcb->GetCopperLayerCount() < 2 )
        {
            if( layer != B_Cu )
            {
                return;
            }
        }

        // If more than one copper layer is enabled, the "Copper"
        // and "Component" layers can be selected, but the total
        // number of copper layers determines which internal
        // layers are also capable of being selected.
        else
        {
            if( ( layer != B_Cu ) && ( layer != F_Cu )
                && ( layer >= m_Pcb->GetCopperLayerCount() - 1 ) )
            {
                return;
            }
        }
    }

    // Is yet more checking required? E.g. when the layer to be selected
    // is a non-copper layer, or when switching between a copper layer
    // and a non-copper layer, or vice-versa?
    // ...

    SetActiveLayer( layer );

    if( displ_opts->m_ContrastModeDisplay )
        m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnTogglePolarCoords( wxCommandEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    SetStatusText( wxEmptyString );

    displ_opts->m_DisplayPolarCood = !displ_opts->m_DisplayPolarCood;

    UpdateStatusBar();
}


void PCB_BASE_FRAME::OnTogglePadDrawMode( wxCommandEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    displ_opts->m_DisplayPadFill = !displ_opts->m_DisplayPadFill;
    EDA_DRAW_PANEL_GAL* gal = GetGalCanvas();

    if( gal )
    {
        // Apply new display options to the GAL canvas
        auto view = static_cast<KIGFX::PCB_VIEW*>( gal->GetView() );
        view->UpdateDisplayOptions( displ_opts );

        // Update pads
        BOARD* board = GetBoard();
        for( MODULE* module = board->m_Modules; module; module = module->Next() )
        {
            for( auto pad : module->Pads() )
                view->Update( pad, KIGFX::GEOMETRY );
        }
    }

    m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnToggleGraphicDrawMode( wxCommandEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    displ_opts->m_DisplayDrawItemsFill = !displ_opts->m_DisplayDrawItemsFill;
    m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnToggleEdgeDrawMode( wxCommandEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    displ_opts->m_DisplayModEdgeFill = !displ_opts->m_DisplayModEdgeFill;
    EDA_DRAW_PANEL_GAL* gal = GetGalCanvas();

    if( gal )
    {
        // Apply new display options to the GAL canvas
        auto view = static_cast<KIGFX::PCB_VIEW*>( gal->GetView() );
        view->UpdateDisplayOptions( displ_opts );
        view->MarkTargetDirty( KIGFX::TARGET_NONCACHED );
    }

    m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnToggleTextDrawMode( wxCommandEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    displ_opts->m_DisplayModTextFill = !displ_opts->m_DisplayModTextFill;
    EDA_DRAW_PANEL_GAL* gal = GetGalCanvas();

    if( gal )
    {
        // Apply new display options to the GAL canvas
        auto view = static_cast<KIGFX::PCB_VIEW*>( gal->GetView() );
        view->UpdateDisplayOptions( displ_opts );
        view->MarkTargetDirty( KIGFX::TARGET_NONCACHED );
    }

    m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnSwitchCanvas( wxCommandEvent& aEvent )
{
    switch( aEvent.GetId() )
    {
    case ID_MENU_CANVAS_LEGACY:
        SwitchCanvas( EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE );
        break;

    case ID_MENU_CANVAS_CAIRO:
        SwitchCanvas( EDA_DRAW_PANEL_GAL::GAL_TYPE_CAIRO );
        break;

    case ID_MENU_CANVAS_OPENGL:
        SwitchCanvas( EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL );
        break;
    }
}


void PCB_BASE_FRAME::OnUpdateCoordType( wxUpdateUIEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    aEvent.Check( displ_opts->m_DisplayPolarCood );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_POLAR_COORD,
                                        displ_opts->m_DisplayPolarCood ?
                                        _( "Display rectangular coordinates" ) :
                                        _( "Display polar coordinates" ) );
}


void PCB_BASE_FRAME::OnUpdatePadDrawMode( wxUpdateUIEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    aEvent.Check( !displ_opts->m_DisplayPadFill );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_PADS_SKETCH,
                                        displ_opts->m_DisplayPadFill ?
                                        _( "Show pads in outline mode" ) :
                                        _( "Show pads in fill mode" ) );
}


void PCB_BASE_FRAME::OnUpdateGraphicDrawMode( wxUpdateUIEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    aEvent.Check( !displ_opts->m_DisplayDrawItemsFill);
}


void PCB_BASE_FRAME::OnUpdateEdgeDrawMode( wxUpdateUIEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    aEvent.Check( !displ_opts->m_DisplayModEdgeFill );

    wxString msgEdgesFill[2] = { _( "Show outlines in filled mode" ),
                                 _( "Show outlines in sketch mode" ) };

    unsigned i = displ_opts->m_DisplayModTextFill == SKETCH ? 0 : 1;
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_MODULE_EDGE_SKETCH, msgEdgesFill[i] );
}


void PCB_BASE_FRAME::OnUpdateTextDrawMode( wxUpdateUIEvent& aEvent )
{
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
    aEvent.Check( !displ_opts->m_DisplayModTextFill );

    wxString msgTextsFill[2] = { _( "Show texts in filled mode" ),
                                 _( "Show texts in sketch mode" ) };
    unsigned i = displ_opts->m_DisplayModTextFill == SKETCH ? 0 : 1;
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_MODULE_TEXT_SKETCH, msgTextsFill[i] );
}


void PCB_BASE_FRAME::OnUpdateSelectZoom( wxUpdateUIEvent& aEvent )
{
    if( m_zoomSelectBox == NULL || m_zoomSelectBox->GetParent() == NULL )
        return;

    int current = 0;    // display Auto if no match found

    // check for a match within 1%
    double zoom = IsGalCanvasActive() ? GetGalCanvas()->GetLegacyZoom() : GetScreen()->GetZoom();

    for( unsigned i = 0; i < GetScreen()->m_ZoomList.size(); i++ )
    {
        if( std::fabs( zoom - GetScreen()->m_ZoomList[i] ) < ( zoom / 100.0 ) )
        {
            current = i + 1;
            break;
        }
    }

    if( current != m_zoomSelectBox->GetSelection() )
        m_zoomSelectBox->SetSelection( current );
}


void PCB_BASE_FRAME::ProcessItemSelection( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();

    // index into the collector list:
    int itemNdx = id - ID_POPUP_PCB_ITEM_SELECTION_START;

    if( id >= ID_POPUP_PCB_ITEM_SELECTION_START && id <= ID_POPUP_PCB_ITEM_SELECTION_END )
    {
        BOARD_ITEM* item = (*m_Collector)[itemNdx];
        m_canvas->SetAbortRequest( false );

#if 0 && defined (DEBUG)
        item->Show( 0, std::cout );
#endif

        SetCurItem( item );
    }
}


void PCB_BASE_FRAME::SetCurItem( BOARD_ITEM* aItem, bool aDisplayInfo )
{
    GetScreen()->SetCurItem( aItem );

    if( aDisplayInfo )
        UpdateMsgPanel();
}


void PCB_BASE_FRAME::UpdateMsgPanel()
{
    BOARD_ITEM* item = GetScreen()->GetCurItem();
    MSG_PANEL_ITEMS items;

    if( item )
    {
        item->GetMsgPanelInfo( m_UserUnits, items );
    }
    else       // show general information about the board
    {
        if( IsGalCanvasActive() )
            GetGalCanvas()->GetMsgPanelInfo( m_UserUnits, items );
        else
            m_Pcb->GetMsgPanelInfo( m_UserUnits, items );
    }

    SetMsgPanel( items );
}


BOARD_ITEM* PCB_BASE_FRAME::GetCurItem()
{
    return GetScreen()->GetCurItem();
}


GENERAL_COLLECTORS_GUIDE PCB_BASE_FRAME::GetCollectorsGuide()
{
    GENERAL_COLLECTORS_GUIDE guide( m_Pcb->GetVisibleLayers(), GetActiveLayer(),
                                    GetGalCanvas()->GetView() );

    // account for the globals
    guide.SetIgnoreMTextsMarkedNoShow( ! m_Pcb->IsElementVisible( LAYER_MOD_TEXT_INVISIBLE ) );
    guide.SetIgnoreMTextsOnBack( ! m_Pcb->IsElementVisible( LAYER_MOD_TEXT_BK ) );
    guide.SetIgnoreMTextsOnFront( ! m_Pcb->IsElementVisible( LAYER_MOD_TEXT_FR ) );
    guide.SetIgnoreModulesOnBack( ! m_Pcb->IsElementVisible( LAYER_MOD_BK ) );
    guide.SetIgnoreModulesOnFront( ! m_Pcb->IsElementVisible( LAYER_MOD_FR ) );
    guide.SetIgnorePadsOnBack( ! m_Pcb->IsElementVisible( LAYER_PAD_BK ) );
    guide.SetIgnorePadsOnFront( ! m_Pcb->IsElementVisible( LAYER_PAD_FR ) );
    guide.SetIgnoreThroughHolePads( ! m_Pcb->IsElementVisible( LAYER_PADS_TH ) );
    guide.SetIgnoreModulesVals( ! m_Pcb->IsElementVisible( LAYER_MOD_VALUES ) );
    guide.SetIgnoreModulesRefs( ! m_Pcb->IsElementVisible( LAYER_MOD_REFERENCES ) );
    guide.SetIgnoreThroughVias( ! m_Pcb->IsElementVisible( LAYER_VIA_THROUGH ) );
    guide.SetIgnoreBlindBuriedVias( ! m_Pcb->IsElementVisible( LAYER_VIA_BBLIND ) );
    guide.SetIgnoreMicroVias( ! m_Pcb->IsElementVisible( LAYER_VIA_MICROVIA ) );
    guide.SetIgnoreTracks( ! m_Pcb->IsElementVisible( LAYER_TRACKS ) );

    return guide;
}

void PCB_BASE_FRAME::SetToolID( int aId, int aCursor, const wxString& aToolMsg )
{
    bool redraw = false;

    EDA_DRAW_FRAME::SetToolID( aId, aCursor, aToolMsg );

    if( aId < 0 )
        return;

    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    // handle color changes for transitions in and out of ID_TRACK_BUTT
    if( ( GetToolId() == ID_TRACK_BUTT && aId != ID_TRACK_BUTT )
        || ( GetToolId() != ID_TRACK_BUTT && aId == ID_TRACK_BUTT ) )
    {
        if( displ_opts->m_ContrastModeDisplay )
            redraw = true;
    }

    // must do this after the tool has been set, otherwise pad::Draw() does
    // not show proper color when GetDisplayOptions().ContrastModeDisplay is true.
    if( redraw && m_canvas )
        m_canvas->Refresh();
}


/*
 * Display the grid status.
 */
void PCB_BASE_FRAME::DisplayGridMsg()
{
    wxString line;
    wxString gridformatter;

    switch( m_UserUnits )
    {
    case INCHES:
        gridformatter = "grid X %.6f  Y %.6f";
        break;

    case MILLIMETRES:
        gridformatter = "grid X %.6f  Y %.6f";
        break;

    default:
        gridformatter = "grid X %f  Y %f";
        break;
    }

    BASE_SCREEN* screen = GetScreen();
    wxArrayString gridsList;

    int icurr = screen->BuildGridsChoiceList( gridsList, m_UserUnits != INCHES );
    GRID_TYPE& grid = screen->GetGrid( icurr );
    double grid_x = To_User_Unit( m_UserUnits, grid.m_Size.x );
    double grid_y = To_User_Unit( m_UserUnits, grid.m_Size.y );
    line.Printf( gridformatter, grid_x, grid_y );

    SetStatusText( line, 4 );
}


/*
 * Update the status bar information.
 */
void PCB_BASE_FRAME::UpdateStatusBar()
{
    PCB_SCREEN* screen = GetScreen();

    if( !screen )
        return;

    wxString line;
    wxString locformatter;
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();

    EDA_DRAW_FRAME::UpdateStatusBar();

    if( displ_opts->m_DisplayPolarCood )  // display polar coordinates
    {
        double dx = (double)GetCrossHairPosition().x - (double)screen->m_O_Curseur.x;
        double dy = (double)GetCrossHairPosition().y - (double)screen->m_O_Curseur.y;

        double theta = ArcTangente( -dy, dx ) / 10;
        double ro = hypot( dx, dy );

        wxString formatter;
        switch( GetUserUnits() )
        {
        case INCHES:
            formatter = "r %.6f  theta %.1f";
            break;

        case MILLIMETRES:
            formatter = "r %.6f  theta %.1f";
            break;

        case UNSCALED_UNITS:
            formatter = "r %f  theta %f";
            break;

        case DEGREES:
            wxASSERT( false );
            break;
        }

        line.Printf( formatter, To_User_Unit( GetUserUnits(), ro ), theta );

        SetStatusText( line, 3 );
    }

    // Display absolute coordinates:
    double dXpos = To_User_Unit( GetUserUnits(), GetCrossHairPosition().x );
    double dYpos = To_User_Unit( GetUserUnits(), GetCrossHairPosition().y );

    // The following sadly is an if Eeschema/if Pcbnew
    wxString absformatter;

    switch( GetUserUnits() )
    {
    case INCHES:
        absformatter = "X %.6f  Y %.6f";
        locformatter = "dx %.6f  dy %.6f  dist %.4f";
        break;

    case MILLIMETRES:
        absformatter = "X %.6f  Y %.6f";
        locformatter = "dx %.6f  dy %.6f  dist %.3f";
        break;

    case UNSCALED_UNITS:
        absformatter = "X %f  Y %f";
        locformatter = "dx %f  dy %f  dist %f";
        break;

    case DEGREES:
        wxASSERT( false );
        break;
    }

    line.Printf( absformatter, dXpos, dYpos );
    SetStatusText( line, 2 );

    if( !displ_opts->m_DisplayPolarCood )  // display relative cartesian coordinates
    {
        // Display relative coordinates:
        double dx = (double)GetCrossHairPosition().x - (double)screen->m_O_Curseur.x;
        double dy = (double)GetCrossHairPosition().y - (double)screen->m_O_Curseur.y;
        dXpos = To_User_Unit( GetUserUnits(), dx );
        dYpos = To_User_Unit( GetUserUnits(), dy );

        // We already decided the formatter above
        line.Printf( locformatter, dXpos, dYpos, hypot( dXpos, dYpos ) );
        SetStatusText( line, 3 );
    }

    DisplayGridMsg();
}


void PCB_BASE_FRAME::unitsChangeRefresh()
{
    EDA_DRAW_FRAME::unitsChangeRefresh();    // Update the status bar.

    UpdateGridSelectBox();
}


void PCB_BASE_FRAME::LoadSettings( wxConfigBase* aCfg )
{
    EDA_DRAW_FRAME::LoadSettings( aCfg );

    // Ensure grid id is an existent grid id:
    if( (m_LastGridSizeId <= 0) ||
        (m_LastGridSizeId > (ID_POPUP_GRID_USER - ID_POPUP_GRID_LEVEL_1000)) )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_500 - ID_POPUP_GRID_LEVEL_1000;

    wxString baseCfgName = GetName();

    EDA_UNITS_T userGridUnits;
    aCfg->Read( baseCfgName + UserGridUnitsEntry, ( int* )&userGridUnits, ( int )INCHES );

    double tmp;
    aCfg->Read( baseCfgName + UserGridSizeXEntry, &tmp, 0.01 );
    m_UserGridSize.x = From_User_Unit( userGridUnits, tmp );

    aCfg->Read( baseCfgName + UserGridSizeYEntry, &tmp, 0.01 );
    m_UserGridSize.y = From_User_Unit( userGridUnits, tmp );

    aCfg->Read( baseCfgName + DisplayPadFillEntry, &m_DisplayOptions.m_DisplayPadFill, true );
    aCfg->Read( baseCfgName + DisplayViaFillEntry, &m_DisplayOptions.m_DisplayViaFill, true );
    aCfg->Read( baseCfgName + DisplayPadNumberEntry, &m_DisplayOptions.m_DisplayPadNum, true );
    aCfg->Read( baseCfgName + DisplayModuleEdgeEntry, &m_DisplayOptions.m_DisplayModEdgeFill, true );

    long itmp;
    aCfg->Read( baseCfgName + FastGrid1Entry, &itmp, ( long )0);
    m_FastGrid1 = itmp;
    aCfg->Read( baseCfgName + FastGrid2Entry, &itmp, ( long )0);
    m_FastGrid2 = itmp;

    aCfg->Read( baseCfgName + DisplayModuleTextEntry, &m_DisplayOptions.m_DisplayModTextFill, true );
}


void PCB_BASE_FRAME::SaveSettings( wxConfigBase* aCfg )
{
    EDA_DRAW_FRAME::SaveSettings( aCfg );

    wxString baseCfgName = GetName();

    aCfg->Write( baseCfgName + UserGridSizeXEntry, To_User_Unit( m_UserUnits, m_UserGridSize.x ) );
    aCfg->Write( baseCfgName + UserGridSizeYEntry, To_User_Unit( m_UserUnits, m_UserGridSize.y ) );
    aCfg->Write( baseCfgName + UserGridUnitsEntry, ( long )m_UserUnits );
    aCfg->Write( baseCfgName + DisplayPadFillEntry, m_DisplayOptions.m_DisplayPadFill );
    aCfg->Write( baseCfgName + DisplayViaFillEntry, m_DisplayOptions.m_DisplayViaFill );
    aCfg->Write( baseCfgName + DisplayPadNumberEntry, m_DisplayOptions.m_DisplayPadNum );
    aCfg->Write( baseCfgName + DisplayModuleEdgeEntry, m_DisplayOptions.m_DisplayModEdgeFill );
    aCfg->Write( baseCfgName + DisplayModuleTextEntry, m_DisplayOptions.m_DisplayModTextFill );
    aCfg->Write( baseCfgName + FastGrid1Entry, ( long )m_FastGrid1 );
    aCfg->Write( baseCfgName + FastGrid2Entry, ( long )m_FastGrid2 );
}


void PCB_BASE_FRAME::CommonSettingsChanged()
{
    EDA_DRAW_FRAME::CommonSettingsChanged();

    ReCreateHToolbar();
    ReCreateAuxiliaryToolbar();
    ReCreateVToolbar();
    ReCreateOptToolbar();

    EDA_3D_VIEWER* viewer = Get3DViewerFrame();

    if( viewer )
    {
        // The 3D viewer isn't in the Kiway, so send its update manually
        viewer->CommonSettingsChanged();
    }
}


void PCB_BASE_FRAME::OnModify()
{
    GetScreen()->SetModify();
    GetScreen()->SetSave();

    if( IsGalCanvasActive() )
    {
        UpdateStatusBar();
        UpdateMsgPanel();
    }
}


const wxString PCB_BASE_FRAME::GetZoomLevelIndicator() const
{
    return EDA_DRAW_FRAME::GetZoomLevelIndicator();
}


void PCB_BASE_FRAME::UpdateGridSelectBox()
{
    UpdateStatusBar();
    DisplayUnitsMsg();

    if( m_gridSelectBox == NULL )
        return;

    // Update grid values with the current units setting.
    m_gridSelectBox->Clear();
    wxArrayString gridsList;
    int icurr = GetScreen()->BuildGridsChoiceList( gridsList, GetUserUnits() != INCHES );

    for( size_t i = 0; i < GetScreen()->GetGridCount(); i++ )
    {
        GRID_TYPE& grid = GetScreen()->GetGrid( i );
        m_gridSelectBox->Append( gridsList[i], (void*) &grid.m_CmdId );
    }

    m_gridSelectBox->Append( wxT( "---" ) );
    m_gridSelectBox->Append( _( "Edit user grid..." ) );

    m_gridSelectBox->SetSelection( icurr );
}


void PCB_BASE_FRAME::updateZoomSelectBox()
{
    if( m_zoomSelectBox == NULL )
        return;

    wxString msg;

    m_zoomSelectBox->Clear();
    m_zoomSelectBox->Append( _( "Zoom Auto" ) );
    m_zoomSelectBox->SetSelection( 0 );

    for( unsigned i = 0;  i < GetScreen()->m_ZoomList.size();  ++i )
    {
        msg = _( "Zoom " );

        double level =  m_zoomLevelCoeff / (double)GetScreen()->m_ZoomList[i];
        wxString value = wxString::Format( wxT( "%.2f" ), level );
        msg += value;

        m_zoomSelectBox->Append( msg );

        if( GetScreen()->GetZoom() == GetScreen()->m_ZoomList[i] )
            m_zoomSelectBox->SetSelection( i + 1 );
    }
}


void PCB_BASE_FRAME::SetFastGrid1()
{
    if( m_FastGrid1 >= (int)GetScreen()->GetGridCount() )
        return;

    int cmdId = GetScreen()->GetGrids()[m_FastGrid1].m_CmdId;
    SetPresetGrid( cmdId - ID_POPUP_GRID_LEVEL_1000 );

    if( m_gridSelectBox )
    {
        wxCommandEvent cmd( wxEVT_CHOICE );
        cmd.SetEventObject( this );
        OnSelectGrid( cmd );
    }
    else
        GetCanvas()->Refresh();
}


void PCB_BASE_FRAME::SetFastGrid2()
{
    if( m_FastGrid2 >= (int)GetScreen()->GetGridCount() )
        return;

    int cmdId = GetScreen()->GetGrids()[m_FastGrid2].m_CmdId;
    SetPresetGrid( cmdId - ID_POPUP_GRID_LEVEL_1000 );

    if( m_gridSelectBox )
    {
        wxCommandEvent cmd( wxEVT_CHOICE );
        cmd.SetEventObject( this );
        OnSelectGrid( cmd );
    }
    else
        GetCanvas()->Refresh();
}

void PCB_BASE_FRAME::SetNextGrid()
{
    EDA_DRAW_FRAME::SetNextGrid();

    if( m_gridSelectBox )
    {
        wxCommandEvent cmd( wxEVT_CHOICE );
        cmd.SetEventObject( this );
        OnSelectGrid( cmd );
    }
    else
        GetCanvas()->Refresh();
}


void PCB_BASE_FRAME::SetPrevGrid()
{
    EDA_DRAW_FRAME::SetPrevGrid();

    if( m_gridSelectBox )
    {
        wxCommandEvent cmd( wxEVT_CHOICE );
        cmd.SetEventObject( this );
        OnSelectGrid( cmd );
    }
    else
        GetCanvas()->Refresh();
}


void PCB_BASE_FRAME::UseGalCanvas( bool aEnable )
{
    EDA_DRAW_FRAME::UseGalCanvas( aEnable );

    EDA_DRAW_PANEL_GAL* galCanvas = GetGalCanvas();

    if( m_toolManager )
        m_toolManager->SetEnvironment( m_Pcb, GetGalCanvas()->GetView(),
                                    GetGalCanvas()->GetViewControls(), this );

    if( aEnable )
    {
        SetBoard( m_Pcb );

        if( m_toolManager )
            m_toolManager->ResetTools( TOOL_BASE::GAL_SWITCH );

        // Transfer latest current display options from legacy to GAL canvas:
        auto painter = static_cast<KIGFX::PCB_PAINTER*>( galCanvas->GetView()->GetPainter() );
        auto settings = painter->GetSettings();
        auto displ_opts = (PCB_DISPLAY_OPTIONS*)GetDisplayOptions();
        settings->LoadDisplayOptions( displ_opts, ShowPageLimits() );

        galCanvas->GetView()->RecacheAllItems();
        galCanvas->SetEventDispatcher( m_toolDispatcher );
        galCanvas->StartDrawing();
    }
    else
    {
        if( m_toolManager )
            m_toolManager->ResetTools( TOOL_BASE::GAL_SWITCH );

        // Redirect all events to the legacy canvas
        galCanvas->SetEventDispatcher( NULL );
    }
}


void PCB_BASE_FRAME::OnUpdateSwitchCanvas( wxUpdateUIEvent& aEvent )
{
    wxMenuBar* menuBar = GetMenuBar();
    EDA_DRAW_PANEL_GAL* gal_canvas = GetGalCanvas();
    EDA_DRAW_PANEL_GAL::GAL_TYPE canvasType = EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE;

    if( IsGalCanvasActive() && gal_canvas )
        canvasType = gal_canvas->GetBackend();

    struct { int menuId; int galType; } menuList[] =
    {
        { ID_MENU_CANVAS_LEGACY,    EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE },
        { ID_MENU_CANVAS_OPENGL,    EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL },
        { ID_MENU_CANVAS_CAIRO,     EDA_DRAW_PANEL_GAL::GAL_TYPE_CAIRO },
    };

    for( auto ii: menuList )
    {
        wxMenuItem* item = menuBar->FindItem( ii.menuId );
        if( item && ii.galType == canvasType )
            item->Check( true );
    }
}
