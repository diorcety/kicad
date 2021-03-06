/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2016 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file pcb_base_frame.h
 * @brief Classes used in Pcbnew, CvPcb and GerbView.
 */

#ifndef  PCB_BASE_FRAME_H
#define  PCB_BASE_FRAME_H


#include <vector>
#include <boost/interprocess/exceptions.hpp>

#include <draw_frame.h>
#include <base_struct.h>
#include <eda_text.h>                // EDA_DRAW_MODE_T
#include <richio.h>
#include <pcb_screen.h>
#include <pcb_display_options.h>
#include <pcb_general_settings.h>
#include <lib_id.h>

/* Forward declarations of classes. */
class BOARD;
class BOARD_CONNECTED_ITEM;
class MODULE;
class TRACK;
class D_PAD;
class TEXTE_MODULE;
class EDA_3D_VIEWER;
class GENERAL_COLLECTOR;
class GENERAL_COLLECTORS_GUIDE;
class BOARD_DESIGN_SETTINGS;
class ZONE_SETTINGS;
class PCB_PLOT_PARAMS;
class FP_LIB_TABLE;
class PCB_GENERAL_SETTINGS ;

/**
 * class PCB_BASE_FRAME
 * basic PCB main window class for Pcbnew, Gerbview, and CvPcb footprint viewer.
 */
class PCB_BASE_FRAME : public EDA_DRAW_FRAME
{
public:
    PCB_DISPLAY_OPTIONS m_DisplayOptions;
    wxPoint m_UserGridSize;

    int m_FastGrid1;                // 1st fast grid setting (index in EDA_DRAW_FRAME::m_gridSelectBox)
    int m_FastGrid2;                // 2nd fast grid setting (index in EDA_DRAW_FRAME::m_gridSelectBox)

protected:
    BOARD*               m_Pcb;
    GENERAL_COLLECTOR*   m_Collector;

    PCB_GENERAL_SETTINGS m_configSettings;

    void updateZoomSelectBox();
    virtual void unitsChangeRefresh() override;

    /**
     * Function loadFootprint
     * attempts to load \a aFootprintId from the footprint library table.
     *
     * @param aFootprintId is the #LIB_ID of component footprint to load.
     * @return the #MODULE if found or NULL if \a aFootprintId not found in any of the
     *         libraries in the table returned from #Prj().PcbFootprintLibs().
     * @throw IO_ERROR if an I/O error occurs or a #PARSE_ERROR if a file parsing error
     *                 occurs while reading footprint library files.
     */
    MODULE* loadFootprint( const LIB_ID& aFootprintId );

public:
    PCB_BASE_FRAME( KIWAY* aKiway, wxWindow* aParent, FRAME_T aFrameType,
            const wxString& aTitle, const wxPoint& aPos, const wxSize& aSize,
            long aStyle, const wxString& aFrameName );

    ~PCB_BASE_FRAME();

    /**
     * @return a reference to the 3D viewer frame, when exists, or NULL
     */
    EDA_3D_VIEWER* Get3DViewerFrame();

    /**
     * Update the 3D view, if the viewer is opened by this frame
     * @param aTitle = the new title of the 3D frame, or nullptr
     * to do not change the frame title
     * @return false if the 3D view cannot be updated (because the
     * owner of the viewer is not this frame)
     */
    bool Update3DView( const wxString* aTitle = nullptr );

    /**
     * Function LoadFootprint
     * attempts to load \a aFootprintId from the footprint library table.
     *
     * @param aFootprintId is the #LIB_ID of component footprint to load.
     * @return the #MODULE if found or NULL if \a aFootprintId not found in any of the
     *         libraries in table returned from #Prj().PcbFootprintLibs().
     */
    MODULE* LoadFootprint( const LIB_ID& aFootprintId );

    /**
     * Function GetBoardBoundingBox
     * calculates the bounding box containing all board items (or board edge segments).
     * @param aBoardEdgesOnly is true if we are interested in board edge segments only.
     * @return EDA_RECT - the board's bounding box
     */
    EDA_RECT    GetBoardBoundingBox( bool aBoardEdgesOnly = false ) const;

    virtual void SetPageSettings( const PAGE_INFO& aPageSettings ) override;
    const PAGE_INFO& GetPageSettings() const override;
    const wxSize GetPageSizeIU() const override;

    const wxPoint& GetAuxOrigin() const override;
    void SetAuxOrigin( const wxPoint& aPoint ) override;

    const wxPoint& GetGridOrigin() const override;
    void SetGridOrigin( const wxPoint& aPoint ) override;

    const TITLE_BLOCK& GetTitleBlock() const override;
    void SetTitleBlock( const TITLE_BLOCK& aTitleBlock ) override;

    /**
     * Function GetDesignSettings
     * returns the BOARD_DESIGN_SETTINGS for the BOARD owned by this frame.
     * Overloaded in FOOTPRINT_EDIT_FRAME.
     */
    virtual BOARD_DESIGN_SETTINGS& GetDesignSettings() const;
    virtual void SetDesignSettings( const BOARD_DESIGN_SETTINGS& aSettings );

    void SetDrawBgColor( COLOR4D aColor ) override;

    /**
     * Function GetDisplayOptions
     * returns the display options current in use
     * Display options are relative to the way tracks, vias, outlines
     * and other things are shown (for instance solid or sketch mode)
     * Must be overloaded in frames which have display options
     * (board editor and footprint editor)
     */
    void* GetDisplayOptions() override { return &m_DisplayOptions; }

    const ZONE_SETTINGS& GetZoneSettings() const;
    void SetZoneSettings( const ZONE_SETTINGS& aSettings );

    /**
     * Function GetPlotSettings
     * returns the PCB_PLOT_PARAMS for the BOARD owned by this frame.
     * Overloaded in FOOTPRINT_EDIT_FRAME.
     */
    virtual const PCB_PLOT_PARAMS& GetPlotSettings() const;
    virtual void SetPlotSettings( const PCB_PLOT_PARAMS& aSettings );

    /**
     * Function SetBoard
     * sets the m_Pcb member in such as way as to ensure deleting any previous
     * BOARD.
     * @param aBoard The BOARD to put into the frame.
     */
    virtual void SetBoard( BOARD* aBoard );

    BOARD* GetBoard() const
    {
        wxASSERT( m_Pcb );
        return m_Pcb;
    }

    // General
    virtual void OnCloseWindow( wxCloseEvent& Event ) = 0;
    virtual void RedrawActiveWindow( wxDC* DC, bool EraseBg ) override { }
    virtual void ReCreateOptToolbar() { }
    virtual void OnLeftClick( wxDC* DC, const wxPoint& MousePos ) override = 0;
    virtual void OnLeftDClick( wxDC* DC, const wxPoint& MousePos ) override = 0;
    virtual bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu ) override = 0;
    virtual void ShowChangedLanguage() override;
    virtual void ReCreateMenuBar() override;
    virtual void SetToolID( int aId, int aCursor, const wxString& aToolMsg ) override;
    virtual void UpdateStatusBar() override;

    PCB_SCREEN* GetScreen() const override { return (PCB_SCREEN*) EDA_DRAW_FRAME::GetScreen(); }

    void UpdateGridSelectBox();

    /**
     * Function BestZoom
     * @return the "best" zoom to show the entire board or footprint on the screen.
     */
    virtual double BestZoom() override;

    /**
     * Function GetZoomLevelIndicator
     * returns a human readable value which can be displayed as zoom
     * level indicator in dialogs.
     * Virtual from the base class
     */
    const wxString GetZoomLevelIndicator() const override;

    /**
     * Shows the 3D view frame.
     * If it does not exist, it is created.
     * If it exists, and if I am the owner, it is bring to the foreground
     */
    virtual void Show3D_Frame( wxCommandEvent& event );

    /**
     * Shows the 3D view frame.
     * If it does not exist, it is created.
     * If it exists, it is bring to the foreground
     * @param aForceRecreateIfNotOwner = true to recreate the 3D frame viewer,
     * when the owner is not me
     * @return true if it is shown with me as owner
     */
    virtual bool CreateAndShow3D_Frame( bool aForceRecreateIfNotOwner );

    /**
     * Function PcbGeneralLocateAndDisplay
     * searches for an item under the mouse cursor.
     * Items are searched first on the current working layer.
     * If nothing found, an item will be searched without layer restriction.
     * If more than one item is found meeting the current working layer
     * criterion, then a popup menu is shown which allows the user to pick
     * which item he/she is interested in.  Once an item is chosen, then it
     * is make the "current item" and the status window is updated to reflect
     * this.
     *
     * @param aHotKeyCode The hotkey which relates to the caller and determines
     *                    the type of search to be performed.  If zero, then
     *                    the mouse tools will be tested instead.
     */
    BOARD_ITEM* PcbGeneralLocateAndDisplay( int aHotKeyCode = 0 );

    void ProcessItemSelection( wxCommandEvent& event );

    /**
     * Function SetCurItem
     * sets the currently selected item and displays it in the MsgPanel.
     * If the given item is NULL then the MsgPanel is erased and there is no
     * currently selected item. This function is intended to make the process
     * of "selecting" an item more formal, and to indivisibly tie the operation
     * of selecting an item to displaying it using BOARD_ITEM::Display_Infos().
     * @param aItem The BOARD_ITEM to make the selected item or NULL if none.
     * @param aDisplayInfo = true to display item info, false if not (default = true)
     */
    void SetCurItem( BOARD_ITEM* aItem, bool aDisplayInfo = true );

    BOARD_ITEM* GetCurItem();

    ///> @copydoc EDA_DRAW_FRAME::UpdateMsgPanel()
    void UpdateMsgPanel() override;

    /**
     * Function GetCollectorsGuide
     * @return GENERAL_COLLECTORS_GUIDE - that considers the global
     *configuration options.
     */
    GENERAL_COLLECTORS_GUIDE GetCollectorsGuide();

    /**
     * Function SelectLibrary
     * puts up a dialog and allows the user to pick a library, for unspecified use.
     *
     * @param aNicknameExisting is the current choice to highlight
     * @return wxString - the library or wxEmptyString on abort.
     */
    wxString SelectLibrary( const wxString& aNicknameExisting );

    /**
     * Function GetFootprintFromBoardByReference
     * @return a reference to the footprint found by its refence
     * on the curent board. the reference is entered by the user from
     * a dialog (by awxTextCtlr, or a list of available references)
     */
    MODULE* GetFootprintFromBoardByReference();

    /**
     * Function OnModify
     * Virtual
     * Must be called after a change
     * in order to set the "modify" flag of the current screen
     * and update the date in frame reference
     * do not forget to call this basic OnModify function to update info
     * in derived OnModify functions
     */
    virtual void OnModify();

    // Modules (footprints)

    /**
     * Function CreateNewModule
     * Creates a new module or footprint, at position 0,0
     * The new module contains only 2 texts: a reference and a value:
     *  Reference = REF**
     *  Value = "VAL**" or Footprint name in lib
     * Note: they are dummy texts, which will be replaced by the actual texts
     * when the fooprint is placed on a board and a netlist is read
     * @param aModuleName = name of the new footprint in library
     * @return a reference to the new module
     */
    MODULE* CreateNewModule( const wxString& aModuleName );

    void Edit_Module( MODULE* module, wxDC* DC );
    void Rotate_Module( wxDC* DC, MODULE* module, double angle, bool incremental );

    /**
     * Function PlaceModule
     * places \a aModule at the current cursor position and updates module coordinates
     * with the new position.
     *
     * @param aModule A MODULE object point of the module to be placed.
     * @param aDC A wxDC object point of the device context to draw \a aModule on
     *            or  NULL if no display screen need updated.
     * @param aRecreateRatsnest A bool true redraws the module rats nest.
     */
    void PlaceModule( MODULE* aModule, wxDC* aDC, bool aRecreateRatsnest = true );

    // module texts
    void RotateTextModule( TEXTE_MODULE* Text, wxDC* DC );
    void DeleteTextModule( TEXTE_MODULE* Text );
    void PlaceTexteModule( TEXTE_MODULE* Text, wxDC* DC );
    void StartMoveTexteModule( TEXTE_MODULE* Text, wxDC* DC );

    /**
     * Function ResetTextSize
     * resets given field text size and width to current settings in
     * Preferences->Dimensions->Texts and Drawings.
     * @param aItem is the item to be reset, either TEXTE_PCB or TEXTE_MODULE.
     * @param aDC is the drawing context.
     */
    void ResetTextSize( BOARD_ITEM* aItem, wxDC* aDC );

    void InstallPadOptionsFrame( D_PAD* pad );

    void AddPad( MODULE* Module, bool draw );

    /**
     * Function DeletePad
     * Delete the pad aPad.
     * Refresh the modified screen area
     * Refresh modified parameters of the parent module (bounding box, last date)
     * @param aPad = the pad to delete
     * @param aQuery = true to prompt for confirmation, false to delete silently
     */
    void DeletePad( D_PAD* aPad, bool aQuery = true );

    /**
     * Function StartMovePad
     * Initialize a drag or move pad command
     * @param aPad = the pad to move or drag
     * @param aDC = the current device context
     * @param aDragConnectedTracks = true to drag connected tracks,
     *                               false to just move the pad
     */
    void StartMovePad( D_PAD* aPad, wxDC* aDC, bool aDragConnectedTracks );

    void PlacePad( D_PAD* Pad, wxDC* DC );
    void Export_Pad_Settings( D_PAD* aPad );
    void Import_Pad_Settings( D_PAD* aPad, bool aDraw );

    /**
     * Function DoPushPadProperties
     * Function to change pad settings for the given footprint or all identical footprints
     * @param aPad is the pattern. The given footprint is the parent of this pad
     * @param aSameFootprints: if true, make changes on all identical footprints
     * @param aPadShapeFilter: if true, make changes only on pads having the same shape as aPad
     * @param aPadOrientFilter: if true, make changes only on pads having the same orientation as aPad
     * @param aPadLayerFilter: if true, make changes only on pads having the same layers as aPad
     * @param aSaveForUndo: if true: create an entry in the Undo/Redo list
     *        (usually: true in Schematic editor, false in Module editor)
     */
    void DoPushPadProperties( D_PAD* aPad,
                              bool aSameFootprints,
                              bool aPadShapeFilter,
                              bool aPadOrientFilter,
                              bool aPadLayerFilter,
                              bool aSaveForUndo );

    /**
     * Function SelectFootprintFromLibTree
     * opens a dialog to select a footprint.
     *
     * @param aPreslect = if valid, the LIB_ID to select (otherwise the global history is
     *                    used)
     * @param aAllowBroswer = allow selection via the footprint viewer (false when already
     *                        called from footprint viewer)
     */
    MODULE* SelectFootprintFromLibTree( LIB_ID aPreselect = LIB_ID(), bool aAllowBroswer = true );

    /**
     * Adds the given module to the board.
     * @param module
     * @param aDC (can be NULL ) = the current Device Context, to draw the new footprint
     */
    virtual void AddModuleToBoard( MODULE* module );

    /**
     * Function SelectFootprintFromLibBrowser
     * launches the footprint viewer to select the name of a footprint to load.
     *
     * @return the selected footprint name or an empty string if no selection was made.
     */
    wxString SelectFootprintFromLibBrowser();

    //  ratsnest functions
    /**
     * Function Compile_Ratsnest
     *  Create the entire board ratsnest.
     *  Must be called after a board change (changes for
     *  pads, footprints or a read netlist ).
     * @param aDC = the current device context (can be NULL)
     * @param aDisplayStatus : if true, display the computation results
     */
    void Compile_Ratsnest( wxDC* aDC, bool aDisplayStatus );

    /**
     * Function build_ratsnest_module
     * Build a ratsnest relative to one footprint. This is a simplified computation
     * used only in move footprint. It is not optimal, but it is fast and sufficient
     * to help a footprint placement
     * It shows the connections from a pad to the nearest connected pad
     * @param aMoveVector = move vector of the footprint being moved.
     * @param aModule = module to consider.
     */
    void build_ratsnest_module( MODULE *aModule, wxPoint aMoveVector );

    /**
     * Function TraceModuleRatsNest
     * display the rats nest of a moving footprint, computed by
     * build_ratsnest_module()
     */
    void TraceModuleRatsNest( wxDC* aDC );

    /**
     *  function Displays the general ratsnest
     *  Only ratsnest with the status bit CH_VISIBLE is set are displayed
     * @param aDC = the current device context (can be NULL)
     * @param aNetcode if > 0, Display only the ratsnest relative to the
     *                 corresponding net_code
     */
    void DrawGeneralRatsnest( wxDC* aDC, int aNetcode = 0 );

    /**
     * Function TraceAirWiresToTargets
     * This functions shows airwires to nearest connecting points (pads)
     * from the current new track end during track creation
     * Uses data prepared by BuildAirWiresTargetsList()
     * @param aDC = the current device context
     */
    void TraceAirWiresToTargets( wxDC* aDC );

    /**
     * Function BuildAirWiresTargetsList
     * Build a list of candidates that can be a coonection point
     * when a track is started.
     * This functions prepares data to show airwires to nearest connecting points (pads)
     * from the current new track to candidates during track creation
     * @param aItemRef = the item connected to the starting point of the new track (track or pad)
     * @param aPosition = the position of the new track end (usually the mouse cursor on grid)
     * @param aNet = the netcode of the track
     */
    void BuildAirWiresTargetsList( BOARD_CONNECTED_ITEM* aItemRef,
                                   const wxPoint& aPosition,
                                   int aNet );

    /**
     * Function TestNetConnection
     * tests the connections relative to \a aNetCode.  Track segments are assumed to be
     * sorted by net codes.
     * @param aDC Current Device Context
     * @param aNetCode The net code to test
     */
    void TestNetConnection( wxDC* aDC, int aNetCode );

    /* Functions relative to Undo/redo commands:
     */

    /**
     * Function SaveCopyInUndoList (virtual pure)
     * Creates a new entry in undo list of commands.
     * add a picker to handle aItemToCopy
     * @param aItemToCopy = the board item modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation, for
     *                          commands like move
     */
    virtual void SaveCopyInUndoList( BOARD_ITEM* aItemToCopy,
                                     UNDO_REDO_T aTypeCommand,
                                     const wxPoint& aTransformPoint = wxPoint( 0, 0 ) ) = 0;

    /**
     * Function SaveCopyInUndoList (virtual pure, overloaded).
     * Creates a new entry in undo list of commands.
     * add a list of pickers to handle a list of items
     * @param aItemsList = the list of items modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation,
     *                          for commands like move
     */
    virtual void SaveCopyInUndoList( const PICKED_ITEMS_LIST& aItemsList,
                                     UNDO_REDO_T aTypeCommand,
                                     const wxPoint& aTransformPoint = wxPoint( 0, 0 ) ) = 0;


    /** Install the dialog box for layer selection
     * @param aDefaultLayer = Preselection (NB_PCB_LAYERS for "(Deselect)" layer)
     * @param aNotAllowedLayersMask = a layer mask for not allowed layers
     *                            (= 0 to show all layers in use)
     * @param aDlgPosition = position of dialog ( defualt = centered)
     * @return the selected layer id
     */
    PCB_LAYER_ID SelectLayer( PCB_LAYER_ID aDefaultLayer,
                          LSET aNotAllowedLayersMask = LSET(),
                          wxPoint aDlgPosition = wxDefaultPosition );

    /* Display a list of two copper layers to choose a pair of copper layers
     * the layer pair is used to fast switch between copper layers when placing vias
     */
    void SelectCopperLayerPair();

    virtual void SwitchLayer( wxDC* DC, PCB_LAYER_ID layer );

    /**
     * Function SetActiveLayer
     * will change the currently active layer to \a aLayer.
     */
    virtual void SetActiveLayer( PCB_LAYER_ID aLayer )
    {
        GetScreen()->m_Active_Layer = aLayer;
    }

    /**
     * Function GetActiveLayer
     * returns the active layer
     */
    virtual PCB_LAYER_ID GetActiveLayer() const
    {
        return GetScreen()->m_Active_Layer;
    }

    void LoadSettings( wxConfigBase* aCfg ) override;
    void SaveSettings( wxConfigBase* aCfg ) override;

    void CommonSettingsChanged() override;

    void OnTogglePolarCoords( wxCommandEvent& aEvent );
    void OnTogglePadDrawMode( wxCommandEvent& aEvent );
    void OnToggleGraphicDrawMode( wxCommandEvent& aEvent );
    void OnToggleEdgeDrawMode( wxCommandEvent& aEvent );
    void OnToggleTextDrawMode( wxCommandEvent& aEvent );

    virtual void OnSwitchCanvas( wxCommandEvent& aEvent );

    // User interface update event handlers.
    void OnUpdateCoordType( wxUpdateUIEvent& aEvent );
    void OnUpdatePadDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateGraphicDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateEdgeDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateTextDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectZoom( wxUpdateUIEvent& aEvent );

    virtual void OnUpdateLayerAlpha( wxUpdateUIEvent& aEvent ) {}

    /**
     * Function SetFastGrid1()
     *
     * Switches grid settings to the 1st "fast" setting predefined by user.
     */
    void SetFastGrid1();

    /**
     * Function SetFastGrid2()
     *
     * Switches grid settings to the 1st "fast" setting predefined by user.
     */
    void SetFastGrid2();

    /**
     * Virtual function SetNextGrid()
     * changes the grid size settings to the next one available.
     */
    void SetNextGrid() override;

    /**
     * Virtual function SetPrevGrid()
     * changes the grid size settings to the previous one available.
     */
    void SetPrevGrid() override;

    /**
     * Function DisplayGridMsg()
     *
     * Display the current grid pane on the status bar.
     */
    void DisplayGridMsg();

    ///> @copydoc EDA_DRAW_FRAME::UseGalCanvas
    virtual void UseGalCanvas( bool aEnable ) override;

    /**
     * Update UI called when switches currently used canvas (default / Cairo / OpenGL).
     */
    void OnUpdateSwitchCanvas( wxUpdateUIEvent& aEvent );

    PCB_GENERAL_SETTINGS& Settings()
    {
        return m_configSettings;
    }

    ///> Key in KifaceSettings to store the canvas type.
    static const wxChar AUTO_ZOOM_KEY[];
    static const wxChar ZOOM_KEY[];

    DECLARE_EVENT_TABLE()
};

#endif  // PCB_BASE_FRAME_H
