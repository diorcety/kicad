/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010 Jean-Pierre Charras, jp.charras@wanadoo.fr
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file wxPcbStruct.h
 */

#ifndef  WXPCB_STRUCT_H_
#define  WXPCB_STRUCT_H_

#include <unordered_map>
#include <map>
#include "pcb_base_edit_frame.h"
#include "3d_cache/3d_cache.h"
#include "config_params.h"
#include "undo_redo_container.h"
#include "zones.h"

/*  Forward declarations of classes. */
class ACTION_PLUGIN;
class PCB_SCREEN;
class BOARD;
class BOARD_COMMIT;
class BOARD_ITEM_CONTAINER;
class TEXTE_PCB;
class MODULE;
class TRACK;
class SEGZONE;
class VIA;
class D_PAD;
class TEXTE_MODULE;
class PCB_TARGET;
class DIMENSION;
class EDGE_MODULE;
class DRC;
class DIALOG_PLOT;
class ZONE_CONTAINER;
class DRAWSEGMENT;
class GENERAL_COLLECTOR;
class GENERAL_COLLECTORS_GUIDE;
class PCB_LAYER_WIDGET;
class MARKER_PCB;
class BOARD_ITEM;
class PCB_LAYER_BOX_SELECTOR;
class NETLIST;
class REPORTER;
struct PARSE_ERROR;
class IO_ERROR;
class FP_LIB_TABLE;

namespace PCB { struct IFACE; }     // KIFACE_I is in pcbnew.cpp

/**
 * Enum to signify the result of editing tracks and vias
 */
enum TRACK_ACTION_RESULT
{
    TRACK_ACTION_DRC_ERROR = -1,//!< TRACK_ACTION_DRC_ERROR - Track not changed to to DRC
    TRACK_ACTION_SUCCESS,       //!< TRACK_ACTION_SUCCESS - Track changed successfully
    TRACK_ACTION_NONE           //!< TRACK_ACTION_NONE - Nothing to change
};

/**
 * Class PCB_EDIT_FRAME
 * is the main frame for Pcbnew.
 *
 * See also class PCB_BASE_FRAME(): Basic class for Pcbnew and GerbView.
 */


class PCB_EDIT_FRAME : public PCB_BASE_EDIT_FRAME
{
    friend struct PCB::IFACE;
    friend class PCB_LAYER_WIDGET;

    /// The auxiliary right vertical tool bar used to access the microwave tools.
    wxAuiToolBar* m_microWaveToolBar;

protected:
    PCB_LAYER_WIDGET* m_Layers;

    DRC* m_drc;                                 ///< the DRC controller, see drc.cpp

    PARAM_CFG_ARRAY   m_configParams;         ///< List of Pcbnew configuration settings.

    wxString          m_lastNetListRead;        ///< Last net list read with relative path.

    // For PrepareLayerIndicator():
    int     m_previous_requested_scale;
    COLOR4D m_previous_active_layer_color;
    COLOR4D m_previous_Route_Layer_TOP_color;
    COLOR4D m_previous_Route_Layer_BOTTOM_color;
    COLOR4D m_previous_via_color;
    COLOR4D m_previous_background_color;

    // The Tool Framework initalization
    void setupTools();

    // we'll use lower case function names for private member functions.
    void createPopUpMenuForZones( ZONE_CONTAINER* edge_zone, wxMenu* aPopMenu );
    void createPopUpMenuForFootprints( MODULE* aModule, wxMenu* aPopMenu );
    void createPopUpMenuForFpTexts( TEXTE_MODULE* aText, wxMenu* aPopMenu );
    void createPopUpMenuForFpPads( D_PAD* aPad, wxMenu* aPopMenu );
    void createPopupMenuForTracks( TRACK* aTrack, wxMenu* aPopMenu );
    void createPopUpMenuForTexts( TEXTE_PCB* Text, wxMenu* menu );
    void createPopUpBlockMenu( wxMenu* menu );
    void createPopUpMenuForMarkers( MARKER_PCB* aMarker, wxMenu* aPopMenu );

    wxString createBackupFile( const wxString& aFileName );

    /**
     * an helper function to enable some menus only active when the display
     * is switched to GAL mode and which do nothing in legacy mode
     */
    void enableGALSpecificMenus();

    /**
     * switches currently used canvas (default / Cairo / OpenGL).
     * It also reinit the layers manager that slightly changes with canvases
     */
    virtual void OnSwitchCanvas( wxCommandEvent& aEvent ) override;

#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)
    /**
     * Function RebuildActionPluginMenus
     * Fill action menu with all registered action plugins
     */
    void RebuildActionPluginMenus();

    /**
     * Function AddActionPluginTools
     * Append action plugin buttons to main toolbar
     */
    void AddActionPluginTools();

	/**
	 * Function RunActionPlugin
	 * Executes action plugin's Run() method and updates undo buffer
	 * @param aActionPlugin action plugin
	 */
	void RunActionPlugin( ACTION_PLUGIN* aActionPlugin );

    /**
     * Function OnActionPluginMenu
     * Launched by the menu when an action is called
     * @param aEvent sent by wx
     */
    void OnActionPluginMenu( wxCommandEvent& aEvent);

    /**
     * Function OnActionPluginButton
     * Launched by the button when an action is called
     * @param aEvent sent by wx
     */
    void OnActionPluginButton( wxCommandEvent& aEvent );

    /**
     * Function OnActionPluginRefresh
     * Refresh plugin list (reload Python plugins)
     * @param aEvent sent by wx
     */
    void OnActionPluginRefresh( wxCommandEvent& aEvent)
    {
       PythonPluginsReload();
    }
#endif

    /** Has meaning only if KICAD_SCRIPTING_WXPYTHON option is
     * not defined
     * @return the frame name identifier for the python console frame
     */
    static const wxChar * pythonConsoleNameId()
    {
        return wxT( "PythonConsole" );
    }

    /**
     * @return a pointer to the python console frame, or NULL if not exist
     */
    static wxWindow * findPythonConsole()
    {
       return FindWindowByName( pythonConsoleNameId() );
    }

    /**
     * Updates the state of the GUI after a new board is loaded or created
     */
    void onBoardLoaded();

    /**
     * Function syncLayerWidgetLayer
     * updates the currently layer "selection" within the PCB_LAYER_WIDGET.
     * The currently selected layer is defined by the return value of GetActiveLayer().
     * <p>
     * This function cannot be inline without including layer_widget.h in
     * here and we do not want to do that.
     * </p>
     */
    void syncLayerWidgetLayer();

    /**
     * Function syncRenderStates
     * updates the "Render" checkboxes in the layer widget according
     * to current toggle values determined by IsElementVisible(), and is helpful
     * immediately after loading a BOARD which may have state information in it.
     */
    void syncRenderStates();

    /**
     * Function syncLayerVisibilities
     * updates each "Layer" checkbox in the layer widget according
     * to each layer's current visibility determined by IsLayerVisible(), and is
     * helpful immediately after loading a BOARD which may have state information in it.
     */
    void syncLayerVisibilities();

    /**
     * Function doAutoSave
     * performs auto save when the board has been modified and not saved within the
     * auto save interval.
     *
     * @return true if the auto save was successful.
     */
    virtual bool doAutoSave() override;

    /**
     * Function isautoSaveRequired
     * returns true if the board has been modified.
     */
    virtual bool isAutoSaveRequired() const override;

    /**
     * Function duplicateZone
     * duplicates the given zone.
     * @param aDC is the current Device Context.
     * @param aZone is the zone to duplicate
     */
    void duplicateZone( wxDC* aDC, ZONE_CONTAINER* aZone );

    /**
     * Function moveExact
     * Move the selected item exactly
     */
    void moveExact();

    /**
     * Function duplicateItems
     * Duplicate selected item if possible and start a move
     * @param aIncrement increment the item number if appropriate
     */
    void duplicateItems( bool aIncrement ) override;

    /**
     * Load the given filename but sets the path to the current project path.
     * @param full filepath of file to be imported.
     * @param aFileType PCB_FILE_T value for filetype
     */
    bool importFile( const wxString& aFileName, int aFileType );

    /**
     * Rematch orphaned zones and vias to schematic nets.
     */
    bool fixEagleNets( const std::unordered_map<wxString, wxString>& aRemap );

    // protected so that PCB::IFACE::CreateWindow() is the only factory.
    PCB_EDIT_FRAME( KIWAY* aKiway, wxWindow* aParent );

public:
    PCB_LAYER_BOX_SELECTOR* m_SelLayerBox;  // a combo box to display and select active layer
    wxChoice* m_SelTrackWidthBox;           // a choice box to display and select current track width
    wxChoice* m_SelViaSizeBox;              // a choice box to display and select current via diameter

    bool m_show_microwave_tools;
    bool m_show_layer_manager_tools;

    bool m_ZoneFillsDirty;                  // Board has been modified since last zone fill.

    virtual ~PCB_EDIT_FRAME();

    /**
     * Function loadFootprints
     * loads the footprints for each #COMPONENT in \a aNetlist from the list of libraries.
     *
     * @param aNetlist is the netlist of components to load the footprints into.
     * @param aReporter is the #REPORTER object to report to.
     * @throw IO_ERROR if an I/O error occurs or a #PARSE_ERROR if a file parsing error
     *           occurs while reading footprint library files.
     */
    void LoadFootprints( NETLIST& aNetlist, REPORTER& aReporter );

    void OnQuit( wxCommandEvent& event );

    /**
     * Reload the Python plugins if they are newer than
     * the already loaded, and load new plugins if any
     * Do nothing if KICAD_SCRIPTING is not defined
     */
    void PythonPluginsReload();

    /**
     * Synchronize the environment variables from KiCad's environment into the Python interpreter.
     * Do nothing if KICAD_SCRIPTING is not defined.
     */
    void PythonSyncEnvironmentVariables();

    /**
     * Synchronize the project name from KiCad's environment into the Python interpreter.
     * Do nothing if KICAD_SCRIPTING is not defined.
     */
    void PythonSyncProjectName();

    /**
     * Update the layer manager and other widgets from the board setup
     * (layer and items visibility, colors ...)
     */
    void UpdateUserInterface();

    /**
     * Execute a remote command send by Eeschema via a socket,
     * port KICAD_PCB_PORT_SERVICE_NUMBER (currently 4242)
     * this is a virtual function called by EDA_DRAW_FRAME::OnSockRequest().
     * @param cmdline = received command from socket
     */
    virtual void ExecuteRemoteCommand( const char* cmdline ) override;

    void KiwayMailIn( KIWAY_EXPRESS& aEvent ) override;

    /**
     * Function ToPlotter
     * Open a dialog frame to create plot and drill files relative to the current board.
     */
    void ToPlotter( wxCommandEvent& event );

    /**
     * Function ToPrinter
     * Install the print dialog.
     */
    void ToPrinter( wxCommandEvent& event );

    /**
     * Function SVG_Print
     * Shows the Export to SVG file dialog.
     */
    void ExportSVG( wxCommandEvent& event );

    // User interface update command event handlers.
    void OnUpdateSave( wxUpdateUIEvent& aEvent );
    void OnUpdateLayerPair( wxUpdateUIEvent& aEvent );
    void OnUpdateLayerSelectBox( wxUpdateUIEvent& aEvent );
    void OnUpdateDrcEnable( wxUpdateUIEvent& aEvent );
    void OnUpdateShowBoardRatsnest( wxUpdateUIEvent& aEvent );
    void OnUpdateViaDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateTraceDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateHighContrastDisplayMode( wxUpdateUIEvent& aEvent );
    void OnUpdateShowLayerManager( wxUpdateUIEvent& aEvent );
    void OnUpdateShowMicrowaveToolbar( wxUpdateUIEvent& aEvent );
    void OnUpdateVerticalToolbar( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectViaSize( wxUpdateUIEvent& aEvent );
    void OnUpdateZoneDisplayStyle( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectTrackWidth( wxUpdateUIEvent& aEvent );
    void OnUpdateMuWaveToolbar( wxUpdateUIEvent& aEvent );
    void OnLayerColorChange( wxCommandEvent& aEvent );
    void OnConfigurePaths( wxCommandEvent& aEvent );
    void OnUpdatePCBFromSch( wxCommandEvent& event );
    void OnRunEeschema( wxCommandEvent& event );

    void UpdateTrackWidthSelectBox( wxChoice* aTrackWidthSelectBox, const bool aEdit = true );
    void UpdateViaSizeSelectBox( wxChoice* aViaSizeSelectBox, const bool aEdit = true );

    void GetKicadAbout( wxCommandEvent& event );

    /**
     * Function IsGridVisible() , virtual
     * @return true if the grid must be shown
     */
    virtual bool IsGridVisible() const override;

    /**
     * Function SetGridVisibility() , virtual
     * It may be overloaded by derived classes
     * if you want to store/retrieve the grid visibility in configuration.
     * @param aVisible = true if the grid must be shown
     */
    virtual void SetGridVisibility( bool aVisible ) override;

    /**
     * Function GetGridColor() , virtual
     * @return the color of the grid
     */
    virtual COLOR4D GetGridColor() override;

    /**
     * Function SetGridColor() , virtual
     * @param aColor = the new color of the grid
     */
    virtual void SetGridColor( COLOR4D aColor ) override;

    // Configurations:
    void Process_Config( wxCommandEvent& event );

#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)

    /**
     * Function SetActionPluginSettings
     * Set a set of plugins that have visible buttons on toolbar
     * Plugins are identified by their module path
     */
    void SetActionPluginSettings( const std::vector< std::pair<wxString, wxString> >& aPluginsWithButtons );

    /**
     * Function GetActionPluginSettings
     * Get a set of plugins that have visible buttons on toolbar
     */
    std::vector< std::pair<wxString, wxString> > GetActionPluginSettings();

    /**
     * Function GetActionPluginButtonVisible
     * Returns true if button visibility action plugin setting was set to true
     * or it is unset and plugin defaults to true.
     */
    bool GetActionPluginButtonVisible( const wxString& aPluginPath, bool aPluginDefault );

    /**
     * Function GetOrderedActionPlugins
     * Returns ordered list of plugins in sequence in which they should appear on toolbar or in settings
     */
    std::vector<ACTION_PLUGIN*> GetOrderedActionPlugins();

#endif

    /**
     * Function GetProjectFileParameters
     * returns a project file parameter list for Pcbnew.
     * <p>
     * Populate a project file parameter array specific to Pcbnew.
     * Creating the parameter list at run time has the advantage of being able
     * to define local variables.  The old method of statically building the array
     * at compile time requiring global variable definitions by design.
     * </p>
     * @return PARAM_CFG_ARRAY - it is only good until SetBoard() is called, so
     *   don't keep it around past that event.
     */
    PARAM_CFG_ARRAY GetProjectFileParameters();

    /**
     * Function SaveProjectSettings
     * saves changes to the project settings to the project (.pro) file.
     * @param aAskForSave = true to open a dialog before saving the settings
     */
    void SaveProjectSettings( bool aAskForSave ) override;

    /**
     * Load the current project's file configuration settings which are pertinent
     * to this PCB_EDIT_FRAME instance.
     *
     * @return always returns true.
     */
    bool LoadProjectSettings();

    /**
     * Function GetConfigurationSettings
     * returns the Pcbnew applications settings list.
     *
     * This replaces the old statically defined list that had the project
     * file settings and the application settings mixed together.  This
     * was confusing and caused some settings to get saved and loaded
     * incorrectly.  Currently, only the settings that are needed at start
     * up by the main window are defined here.  There are other locally used
     * settings that are scattered throughout the Pcbnew source code.  If you need
     * to define a configuration setting that needs to be loaded at run time,
     * this is the place to define it.
     *
     * @return - Reference to the list of applications settings.
     */
    PARAM_CFG_ARRAY& GetConfigurationSettings();

    void LoadSettings( wxConfigBase* aCfg ) override;

    void SaveSettings( wxConfigBase* aCfg ) override;

    wxConfigBase* GetSettings() { return config(); };

    /**
     * Get the last net list read with the net list dialog box.
     * @return - Absolute path and file name of the last net list file successfully read.
     */
    wxString GetLastNetListRead();

    /**
     * Set the last net list successfully read by the net list dialog box.
     *
     * Note: the file path is converted to a path relative to the project file path.  If
     *       the path cannot be made relative, than m_lastNetListRead is set to and empty
     *       string.  This could happen when the net list file is on a different drive than
     *       the project file.  The advantage of relative paths is that is more likely to
     *       work when opening the same project from both Windows and Linux.
     *
     * @param aNetListFile - The last net list file with full path successfully read.
     */
    void SetLastNetListRead( const wxString& aNetListFile );

    ///> @copydoc EDA_DRAW_FRAME::GetHotKeyDescription()
    EDA_HOTKEY* GetHotKeyDescription( int aCommand ) const override;

    /**
     * Function OnHotKey.
     *  ** Commands are case insensitive **
     *  Some commands are relatives to the item under the mouse cursor
     * @param aDC = current device context
     * @param aHotkeyCode = hotkey code (ascii or wxWidget code for special keys)
     * @param aPosition The cursor position in logical (drawing) units.
     * @param aItem = NULL or pointer on a EDA_ITEM under the mouse cursor
     */
    bool OnHotKey( wxDC* aDC, int aHotkeyCode, const wxPoint& aPosition, EDA_ITEM* aItem = NULL ) override;

    /**
     * Function OnHotkeyDeleteItem
     * Delete the item found under the mouse cursor
     *  Depending on the current active tool::
     *      Tool track
     *          if a track is in progress: Delete the last segment
     *          else delete the entire track
     *      Tool module (footprint):
     *          Delete the module.
     * @param aDC = current device context
     * @return true if an item was deleted
     */
    bool OnHotkeyDeleteItem( wxDC* aDC );

    /**
     * Function OnHotkeyPlaceItem
     * Place the item (footprint, track, text .. ) found under the mouse cursor
     * An item can be placed only if there is this item currently edited
     * Only a footprint, a pad or a track can be placed
     * @param aDC = current device context
     * @return true if an item was placed
     */
    bool OnHotkeyPlaceItem( wxDC* aDC );

    bool OnHotkeyEditItem( int aIdCommand );

    /**
     * Function OnHotkeyCopyItem
     * returns the copy event id for copyable items.
     * @return Event id of a suitable copy event, zero when no copyable item found.
     */
    int OnHotkeyCopyItem();

    /**
     * Function OnHotkeyDuplicateOrArrayItem
     * Duplicate an item (optionally incrementing if necessary and possible)
     * or invoke array dialog and create an array
     * @param aIdCommand = the hotkey command id
     * @return true if item duplicated or arrayed
     */
    bool OnHotkeyDuplicateOrArrayItem( int aIdCommand );

    /**
     * Function OnHotkeyMoveItem
     * Moves or drag the item (footprint, track, text .. ) found under the mouse cursor
     * Only a footprint or a track can be dragged
     * @param aIdCommand = the hotkey command id
     * @return true if an item was moved
     */
    bool OnHotkeyMoveItem( int aIdCommand );

    /**
     * Function OnHotkeyRotateItem
     * Rotate the item (text or footprint) found under the mouse cursor
     * @note This command can be used with an item currently in edit.
     *       Only some items can be rotated (footprints and texts).
     * @param aIdCommand = the hotkey command id
     * @return true if an item was moved
     */
    bool OnHotkeyRotateItem( int aIdCommand );

    /**
     * Function OnHotkeyFlipItem
     * Flip the item (text or footprint) found under the mouse cursor
     * @note This command can be used with an item currently in edit.
     *       Only some items can be rotated (footprints and texts).
     * @param aIdCommand = the hotkey command id
     * @return true if an item was moved
     */
    bool OnHotkeyFlipItem( int aIdCommand );

    /**
     * Function OnHotkeyBeginRoute
     * If the current active layer is a copper layer,
     * and if no item currently edited, start a new track segmenton
     * the current copper layer.
     * If a new track is in progress, terminate the current segment and
     * start a new one.
     * @param aDC = current device context
     * @return a reference to the track if a track is created, or NULL
     */
    TRACK * OnHotkeyBeginRoute( wxDC* aDC );

    void OnCloseWindow( wxCloseEvent& Event ) override;
    void Process_Special_Functions( wxCommandEvent& event );
    void Tracks_and_Vias_Size_Event( wxCommandEvent& event );
    void OnSelectTool( wxCommandEvent& aEvent );

    /**
     * Function OnEditTextAndGraphics
     * Dialog for editing properties of text and graphic items, selected by type, layer,
     * and/or parent footprint.
     */
    void OnEditTextAndGraphics( wxCommandEvent& event );

    /**
     * Function OnEditTracksAndVias
     * Dialog for editing the properties of tracks and vias, selected by net, netclass,
     * and/or layer.
     */
    void OnEditTracksAndVias( wxCommandEvent& event );

    void ProcessMuWaveFunctions( wxCommandEvent& event );
    void MuWaveCommand( wxDC* DC, const wxPoint& MousePos );

    void RedrawActiveWindow( wxDC* DC, bool EraseBg ) override;
    void ReCreateHToolbar() override;
    void ReCreateAuxiliaryToolbar() override;
    void ReCreateVToolbar() override;
    void ReCreateMicrowaveVToolbar();
    void ReCreateOptToolbar() override;
    void ReCreateMenuBar() override;

    /**
     * Re create the layer Box by clearing the old list, and building
     * le new one, from the new layers names and cole layers
     * @param aForceResizeToolbar = true to resize the parent toolbar
     * false if not needed (mainly in parent toolbar creation,
     * or when the layers names are not modified)
     */
    void ReCreateLayerBox( bool aForceResizeToolbar = true );


    /**
     * Function SetCurrentNetClass
     * Must be called after a netclass selection (or after a netclass parameter change
     * calls BOARD_DESIGN_SETTINGS::SetCurrentNetClass() and update trace width and via size
     * combo boxes on main toolbar
     * Initialize vias and tracks values displayed in comb boxes of the auxiliary toolbar
     * and some others parameters (netclass name ....)
     * @param aNetClassName = the new netclass name
     * @return true if lists of tracks and vias sizes are modified
     */
    bool SetCurrentNetClass( const wxString& aNetClassName );

    /**
     * Function OnModify
     * must be called after a board change to set the modified flag.
     * <p>
     * Reloads the 3D view if required and calls the base PCB_BASE_FRAME::OnModify function
     * to update auxiliary information.
     * </p>
     */
    virtual void OnModify() override;

    /**
     * Function SetActiveLayer
     * will change the currently active layer to \a aLayer and also
     * update the PCB_LAYER_WIDGET.
     */
    virtual void SetActiveLayer( PCB_LAYER_ID aLayer ) override;

    PCB_LAYER_WIDGET* GetLayerManager() { return m_Layers; }

    /**
     * Update the UI to reflect changes to the current layer's transparency.
     */
    void OnUpdateLayerAlpha( wxUpdateUIEvent& aEvent ) override;

    /**
     * Function IsElementVisible
     * tests whether a given element category is visible. Keep this as an
     * inline function.
     * @param aElement is from the enum by the same name
     * @return bool - true if the element is visible.
     * @see enum GAL_LAYER_ID
     */
    bool IsElementVisible( GAL_LAYER_ID aElement ) const;

    /**
     * Function SetElementVisibility
     * changes the visibility of an element category
     * @param aElement is from the enum by the same name
     * @param aNewState The new visibility state of the element category
     * @see enum PCB_LAYER_ID
     */
    void SetElementVisibility( GAL_LAYER_ID aElement, bool aNewState );

    /**
     * Function SetVisibleAlls
     * Set the status of all visible element categories and layers to VISIBLE
     */
    void SetVisibleAlls();

    /**
     * Function ReFillLayerWidget
     * changes out all the layers in m_Layers and may be called upon
     * loading a new BOARD.
     */
    void ReFillLayerWidget();

    /**
     * Function Show3D_Frame
     * displays the 3D view of current printed circuit board.
     */
    void Show3D_Frame( wxCommandEvent& event ) override;

    ///> @copydoc EDA_DRAW_FRAME::UseGalCanvas()
    void UseGalCanvas( bool aEnable ) override;

    bool GeneralControl( wxDC* aDC, const wxPoint& aPosition, EDA_KEY aHotKey = 0 ) override;

    /**
     * Function ShowBoardSetupDialog
     */
    void ShowBoardSetupDialog( wxCommandEvent& event );
    void DoShowBoardSetupDialog( const wxString& aInitialPage = wxEmptyString,
                                 const wxString& aInitialParentPage = wxEmptyString );

    /* toolbars update UI functions: */

    void PrepareLayerIndicator();

    /* mouse functions events: */
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos ) override;
    void OnLeftDClick( wxDC* DC, const wxPoint& MousePos ) override;

    /**
     * Function OnRightClick
     * populates a popup menu with the choices appropriate for the current context.
     * The caller will add the ZOOM menu choices afterward.
     * @param aMousePos The current mouse position
     * @param aPopMenu The menu to add to.
     */
    bool OnRightClick( const wxPoint& aMousePos, wxMenu* aPopMenu ) override;

    void OnSelectOptionToolbar( wxCommandEvent& event );
    void OnFlipPcbView( wxCommandEvent& event );
    void ToolOnRightClick( wxCommandEvent& event ) override;

    /* Block operations: */

    /**
     * Function BlockCommand
     * Returns the block command internat code (BLOCK_MOVE, BLOCK_COPY...)
     * corresponding to the keys pressed (ALT, SHIFT, SHIFT ALT ..) when
     * block command is started by dragging the mouse.
     * @param aKey = the key modifiers (Alt, Shift ...)
     * @return the block command id (BLOCK_MOVE, BLOCK_COPY...)
     */
    virtual int BlockCommand( EDA_KEY aKey ) override;

    /**
     * Function HandleBlockPlace()
     * Called after HandleBlockEnd, when a block command needs to be
     * executed after the block is moved to its new place
     * (bloc move, drag, copy .. )
     * Parameters must be initialized in GetScreen()->m_BlockLocate
     */
    virtual void HandleBlockPlace( wxDC* DC ) override;

    /**
     * Function HandleBlockEnd()
     * Handle the "end"  of a block command,
     * i.e. is called at the end of the definition of the area of a block.
     * depending on the current block command, this command is executed
     * or parameters are initialized to prepare a call to HandleBlockPlace
     * in GetScreen()->m_BlockLocate
     * @return false if no item selected, or command finished,
     * true if some items found and HandleBlockPlace must be called later
     */
    virtual bool HandleBlockEnd( wxDC* DC ) override;

    /**
     * Function Block_SelectItems
     * Uses  GetScreen()->m_BlockLocate
     * select items within the selected block.
     * selected items are put in the pick list
     */
    void Block_SelectItems();

    /**
     * Function Block_Delete
     * deletes all items within the selected block.
     */
    void Block_Delete();

    /**
     * Function Block_Rotate
     * Rotate all items within the selected block.
     * The rotation center is the center of the block
     */
    void Block_Rotate();

    /**
     * Function Block_Flip
     * Flip items within the selected block.
     * The flip center is the center of the block
     */
    void Block_Flip();

    /**
     * Function Block_Move
     * move all items within the selected block.
     * New location is determined by the current offset from the selected
     * block's original location.
     */
    void Block_Move();

    /**
     * Function Block_Duplicate
     * Duplicate all items within the selected block.
     * New location is determined by the current offset from the selected
     * block's original location.
     */
    void Block_Duplicate( bool aIncrement );

    void InstallPcbGlobalDeleteFrame( const wxPoint& pos );

    /**
     * Function GenFootprintsPositionFile
     * Calls DoGenFootprintsPositionFile to create a footprint position file
     * See DoGenFootprintsPositionFile for options and file format
     */
    void GenFootprintsPositionFile( wxCommandEvent& event );

    /**
     * Function DoGenFootprintsPositionFile
     * Creates an ascii footprint position file
     * @param aFullFileName = the full file name of the file to create
     * @param aUnitsMM = false to use inches, true to use mm in coordinates
     * @param aForceSmdItems = true to force all footprints with smd pads in list
     *                       = false to put only footprints with option "INSERT" in list
     * @param aSide = 0 to list footprints on BACK side,
     *                1 to list footprints on FRONT side
     *                2 to list footprints on both sides
     * @param aFormatCSV = true to use a comma separated file (CSV) format; default = false
     * @param aNegateBottomX = true to negate the x coordinates of the parts on the bottom layer
     * @return the number of footprints found on aSide side,
     *    or -1 if the file could not be created
     */
    int DoGenFootprintsPositionFile( const wxString& aFullFileName, bool aUnitsMM,
                                     bool aForceSmdItems, int aSide, bool aFormatCSV = false,
                                     bool aNegateBottomX = false );

    /**
     * Function GenFootprintsReport
     * Calls DoGenFootprintsReport to create a footprint reprot file
     * See DoGenFootprintsReport for file format
     */
    void GenFootprintsReport( wxCommandEvent& event );

    /**
     * Function DoGenFootprintsReport
     * Creates an ascii footprint report file giving some infos on footprints
     * and board outlines
     * @param aFullFilename = the full file name of the file to create
     * @param aUnitsMM = false to use inches, true to use mm in coordinates
     * @return true if OK, false if error
     */
    bool DoGenFootprintsReport( const wxString& aFullFilename, bool aUnitsMM );

    void InstallDrillFrame( wxCommandEvent& event );
    void GenD356File( wxCommandEvent& event );

    void OnFileHistory( wxCommandEvent& event );

    /**
     * Function Files_io
     * @param event is the command event handler.
     * do nothing else than call Files_io_from_id with the
     * wxCommandEvent id
     */
    void Files_io( wxCommandEvent& event );

    /**
     * Function Files_io_from_id
     * Read and write board files
     * @param aId is an event ID ciming from file command events:
     * ID_LOAD_FILE
     * ID_MENU_READ_BOARD_BACKUP_FILE
     * ID_MENU_RECOVER_BOARD_AUTOSAVE
     * ID_APPEND_FILE
     * ID_NEW_BOARD
     * ID_SAVE_BOARD
     * ID_COPY_BOARD_AS
     * ID_SAVE_BOARD_AS
     * Files_io_from_id prepare parameters and calls the specialized function
     */
    bool Files_io_from_id( int aId );

    /**
     * Function OpenProjectFiles    (was LoadOnePcbFile)
     * loads a KiCad board (.kicad_pcb) from \a aFileName.
     *
     * @param aFileSet - hold the BOARD file to load, a vector of one element.
     *
     * @param aCtl      - KICTL_ bits, one to indicate that an append of the board file
     *                      aFileName to the currently loaded file is desired.
     *                    @see #KIWAY_PLAYER for bit defines.
     *
     * @return bool - false if file load fails, otherwise true.
    bool LoadOnePcbFile( const wxString& aFileName, bool aAppend = false,
                         bool aForceFileDialog = false );
     */
    bool OpenProjectFiles( const std::vector<wxString>& aFileSet, int aCtl = 0 ) override;

    /**
     * Function AppendBoardFile
     * appends a board file onto the current one, creating God knows what.
     * the main purpose is only to allow panelizing boards.
     */
    bool AppendBoardFile( const wxString& aFullFileName, int aCtl );

    /**
     * Function SavePcbFile
     * writes the board data structures to \a a aFileName
     * Creates backup when requested and update flags (modified and saved flgs)
     *
     * @param aFileName The file name to write or wxEmptyString to prompt user for
     *                  file name.
     * @param aCreateBackupFile Creates a back of \a aFileName if true.  Helper
     *                          definitions #CREATE_BACKUP_FILE and #NO_BACKUP_FILE
     *                          are defined for improved code readability.
     * @return True if file was saved successfully.
     */
    bool SavePcbFile( const wxString& aFileName, bool aCreateBackupFile = CREATE_BACKUP_FILE );

    /**
     * Function SavePcbCopy
     * writes the board data structures to \a a aFileName
     * but unlike SavePcbFile, does not make anything else
     * (no backup, borad fliename change, no flag changes ...)
     * Used under a project mgr to save under a new name the current board
     *
     * When not under a project mgr, the full SavePcbFile is used.
     * @param aFileName The file name to write.
     * @return True if file was saved successfully.
     */
    bool SavePcbCopy( const wxString& aFileName );

    // BOARD handling

    /**
     * Function Clear_Pcb
     * delete all and reinitialize the current board
     * @param aQuery = true to prompt user for confirmation, false to initialize silently
     */
    bool Clear_Pcb( bool aQuery );

    ///> @copydoc PCB_BASE_FRAME::SetBoard()
    void SetBoard( BOARD* aBoard ) override;

    ///> @copydoc PCB_BASE_EDIT_FRAME::GetModel()
    BOARD_ITEM_CONTAINER* GetModel() const override;

    ///> @copydoc PCB_BASE_FRAME::SetPageSettings()
    void SetPageSettings( const PAGE_INFO& aPageSettings ) override;

    /**
     * Function GetDrcController
     * @return the DRC controller
     */
    DRC* GetDrcController() { return m_drc; }

    /**
     * Function RecreateBOMFileFromBoard
     * Recreates a .cmp file from the current loaded board
     * this is the same as created by CvPcb.
     * can be used if this file is lost
     */
    void RecreateCmpFileFromBoard( wxCommandEvent& aEvent );

    /**
     * Function ArchiveModulesOnBoard
     * Save modules in a library:
     * @param aStoreInNewLib:
     *              true : save modules in a existing lib. Existing footprints will be kept
     *              or updated.
     *              This lib should be in fp lib table, and is type is .pretty
     *              false: save modules in a new lib. It it is an existing lib,
     *              previous footprints will be removed
     *
     * @param aLibName:
     *              optional library name to create, stops dialog call.
     *              must be called with aStoreInNewLib as true
     */
    void ArchiveModulesOnBoard( bool aStoreInNewLib, const wxString& aLibName = wxEmptyString,
                                wxString* aLibPath = NULL );

    /**
     * Function RecreateBOMFileFromBoard
     * Creates a BOM file from the current loaded board
     */
    void RecreateBOMFileFromBoard( wxCommandEvent& aEvent );

    /**
     * Function ExportToGenCAD
     * creates a file in  GenCAD 1.4 format from the current board.
     */
    void ExportToGenCAD( wxCommandEvent& event );

    /**
     * Function OnExportVRML
     * will export the current BOARD to a VRML file.
     */
    void OnExportVRML( wxCommandEvent& event );

    /**
     * Function ExportVRML_File
     * Creates the file(s) exporting current BOARD to a VRML file.
     *
     * @note When copying 3D shapes files, the new filename is build from the full path
     *       name, changing the separators by underscore.  This is needed because files
     *       with the same shortname can exist in different directories
     * @note ExportVRML_File generates coordinates in board units (BIU) inside the file.
     * @todo Use mm inside the file.  A general scale transform is applied to the whole
     *       file (1.0 to have the actual WRML unit im mm, 0.001 to have the actual WRML
     *       unit in meters.
     * @note For 3D models built by a 3D modeler, the unit is 0,1 inches.  A specific scale
     *       is applied to 3D models to convert them to internal units.
     *
     * @param aFullFileName = the full filename of the file to create
     * @param aMMtoWRMLunit = the VRML scaling factor:
     *      1.0 to export in mm. 0.001 for meters
     * @param aExport3DFiles = true to copy 3D shapes in the subir a3D_Subdir
     * @param aUseRelativePaths set to true to use relative paths instead of absolute paths
     *                          in the board VRML file URLs.
     * @param aUsePlainPCB set to true to export a board with no copper or silkskreen;
     *                          this is useful for generating a VRML file which can be
     *                          converted to a STEP model.
     * @param a3D_Subdir = sub directory where 3D shapes files are copied.  This is only used
     *                     when aExport3DFiles == true
     * @param aXRef = X value of PCB (0,0) reference point
     * @param aYRef = Y value of PCB (0,0) reference point
     * @return true if Ok.
     */
    static bool ExportVRML_File( BOARD *pcb, S3D_CACHE* aCache, const wxString & aFullFileName, double aMMtoWRMLunit,
                          bool aExport3DFiles, bool aUseRelativePaths,
                          bool aUsePlainPCB, const wxString & a3D_Subdir,
                          double aXRef, double aYRef );

    /**
     * Function OnExportIDF3
     * will export the current BOARD to a IDFv3 board and lib files.
     */
    void OnExportIDF3( wxCommandEvent& event );

    /**
     * Function Export_IDF3
     * Creates an IDF3 compliant BOARD (*.emn) and LIBRARY (*.emp) file.
     *
     * @param aPcb = a pointer to the board to be exported to IDF
     * @param aFullFileName = the full filename of the export file
     * @param aUseThou = set to true if the desired IDF unit is thou (mil)
     * @param aXRef = the board Reference Point in mm, X value
     * @param aYRef = the board Reference Point in mm, Y value
     * @return true if OK
     */
    bool Export_IDF3( BOARD* aPcb, const wxString& aFullFileName,
                      bool aUseThou, double aXRef, double aYRef );

    /**
     * Function OnExportSTEP
     * Exports the current BOARD to a STEP assembly.
     */
    void OnExportSTEP( wxCommandEvent& event );

    /**
     * Function ExporttoSPECCTRA
     * Ask for a filename and call ExportSpecctraFile to export the current BOARD
     * to a specctra dsn file.
     */
    void ExportToSpecctra( wxCommandEvent& event );

    /**
     * Function ExportSpecctraFile
     * will export the current BOARD to a specctra dsn file.
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     * @return true if OK
     */
    bool ExportSpecctraFile( const wxString& aFullFilename );

    /**
     * Function ImportSpecctraSession
     * will import a specctra *.ses file and use it to relocate MODULEs and
     * to replace all vias and tracks in an existing and loaded BOARD.
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     */
    void ImportSpecctraSession( wxCommandEvent& event );

    /**
     * Function ImportSpecctraDesign
     * will import a specctra *.dsn file and use it to replace an entire BOARD.
     * The new board will not have any graphics, only components, tracks and
     * vias.
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     */
    void ImportSpecctraDesign( wxCommandEvent& event );

    /**
     * Function ListAndSelectModuleName
     * builds and shows a list of existing modules on board that the user can select.
     * @return a pointer to the selected module or NULL.
     */
    MODULE* ListAndSelectModuleName();

    /**
     * Function ListNetsAndSelect
     * called by a command event
     * displays the sorted list of nets in a dialog frame
     * If a net is selected, it is highlighted
     */
    void ListNetsAndSelect( wxCommandEvent& event );

    void Swap_Layers( wxCommandEvent& event );

    // Handling texts on the board
    void Rotate_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );
    void FlipTextePcb( TEXTE_PCB* aTextePcb, wxDC* aDC );
    TEXTE_PCB* CreateTextePcb( wxDC* aDC, TEXTE_PCB* aText = NULL );
    void Delete_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );
    void StartMoveTextePcb( TEXTE_PCB* aTextePcb, wxDC* aDC, bool aErase = true );
    void Place_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );

    // Graphic Segments type DRAWSEGMENT
    void Start_Move_DrawItem( DRAWSEGMENT* drawitem, wxDC* DC );
    void Place_DrawItem( DRAWSEGMENT* drawitem, wxDC* DC );

    // Footprint editing (see also PCB_BASE_FRAME)
    void InstallFootprintPropertiesDialog( MODULE* Module, wxDC* DC );

    /**
     * Function StartMoveModule
     * Initialize a drag or move pad command
     * @param aModule = the module to move or drag
     * @param aDC = the current device context
     * @param aDragConnectedTracks = true to drag connected tracks,
     *                               false to just move the module
     */
    void StartMoveModule( MODULE* aModule, wxDC* aDC, bool aDragConnectedTracks );

    /**
     * Function PushPadProperties
     * Function to change pad caracteristics for the given footprint
     * or all footprints which look like the given footprint
     * Options are set by the opened dialog.
     * @param aPad is the pattern. The given footprint is the parent of this pad
     */
    void PushPadProperties( D_PAD* aPad );

    /**
     * Function Delete Module
     * Remove a footprint from m_Modules linked list and put it in undelete buffer
     * The ratsnest and pad list are recalculated
     * @param aModule = footprint to delete
     * @param aDC = currentDevice Context. if NULL: do not redraw new ratsnest
     */
    bool Delete_Module( MODULE* aModule, wxDC* aDC );

    /**
     * Function Change_Side_Module
     * Flip a footprint (switch layer from component or component to copper)
     * The mirroring is made from X axis
     * if a footprint is not on copper or component layer it is not flipped
     * (it could be on an adhesive layer, not supported at this time)
     * @param Module the footprint to flip
     * @param  DC Current Device Context. if NULL, no redraw
     */
    void Change_Side_Module( MODULE* Module, wxDC* DC );

    int InstallExchangeModuleFrame( MODULE* aModule, bool updateMode, bool selectedMode );

    /**
     * Function Exchange_Module
     * Replaces OldModule by NewModule, using OldModule settings:
     * position, orientation, pad netnames ...)
     * OldModule is deleted or put in undo list.
     * @param aSrc = footprint to replace
     * @param aDest = footprint to put
     * @param aCommit = commit that should store the changes
     */
    void Exchange_Module( MODULE* aSrc, MODULE* aDest, BOARD_COMMIT& aCommit,
                          bool deleteExtraTexts = true,
                          bool resetTextLayers = true, bool resetTextEffects = true );

    // loading modules: see PCB_BASE_FRAME

    // Board handling
    void RemoveStruct( BOARD_ITEM* Item, wxDC* DC );

    /**
     * Function OnEditItemRequest
     * Install the corresponding dialog editor for the given item
     * @param aDC = the current device context
     * @param aItem = a pointer to the BOARD_ITEM to edit
     */
    void OnEditItemRequest( wxDC* aDC, BOARD_ITEM* aItem ) override;

    /**
     * Locate track or pad and highlight the corresponding net.
     * @return The Netcode, or -1 if no net located.
     */
    int SelectHighLight( wxDC* DC );

    /**
     * Function HighLight.
     * highlights the net at the current cursor position.
     */
    void HighLight( wxDC* DC );

    /**
     * Function IsMicroViaAcceptable
     * return true if a microvia can be placed on the board.
     * <p>
     * A microvia is a small via restricted to 2 near neighbor layers
     * because its is hole is made by laser which can penetrate only one layer
     * It is mainly used to connect BGA to the first inner layer
     * And it is allowed from an external layer to the first inner layer
     * </p>
     */
    bool IsMicroViaAcceptable();

    /**
     * Function Other_Layer_Route
     * operates in one of two ways.  If argument track is NULL, then swap the
     * active layer between m_Route_Layer_TOP and m_Route_Layer_BOTTOM.  If a
     * track is in progress (track is not NULL), and if DRC allows it, place
     * a via on the end of the current track, and then swap the current active
     * layer and start a new segment on the new layer.
     * @param track A TRACK* to append the via to or NULL.
     * @param DC A device context to draw on.
     * @return bool - true if the operation was successful, else false such as
     *                the case where DRC would not allow a via.
     */
    bool Other_Layer_Route( TRACK* track, wxDC* DC );

    /**
     * Function Delete_Segment
     * removes a track segment.
     * If a new track is in progress: delete the current new segment.
     * Otherwise, delete segment under the mouse cursor.
     */
    TRACK* Delete_Segment( wxDC* DC, TRACK* Track );

    void Delete_Track( wxDC* DC, TRACK* Track );
    void Delete_net( wxDC* DC, TRACK* Track );

    /**
     * Function Remove_One_Track
     * removes 1 track/
     * The leading segment is removed and all adjacent segments
     * until a pad or a junction point of more than 2 segments is found
     */
    void Remove_One_Track( wxDC* DC, TRACK* pt_segm );

    /**
     * Function Edit_Track_Width
     * Modify a full track width (using DRC control).
     * a full track is the set of track segments between 2 ends: pads or a
     * point that has more than 2 segments ends connected
     * @param  aDC = the curred device context (can be NULL)
     * @param aTrackSegment = a segment or via on the track to change
     */
    void Edit_Track_Width( wxDC* aDC, TRACK* aTrackSegment );

    /**
     * Function Edit_TrackSegm_Width
     *  Modify one track segment width or one via diameter (using DRC control).
     * @param  aDC = the current device context (can be NULL)
     * @param aTrackItem = the track segment or via to modify
     */
    void Edit_TrackSegm_Width( wxDC* aDC, TRACK* aTrackItem );

    /**
     * Function Begin_Route
     * Starts a new track and/or establish of a new track point.
     *
     * For a new track:
     * - Search the netname of the new track from the starting point
     * if it is on a pad or an existing track
     * - Highlight all this net
     * If a track is in progress:
     * - Call DRC
     * - If DRC is OK: finish the track segment and starts a new one.
     * @param aTrack = the current track segment, or NULL to start a new track
     * @param aDC = the current device context
     * @return a pointer to the new track segment or null if not created (DRC error)
     */
    TRACK* Begin_Route( TRACK* aTrack, wxDC* aDC );

    /**
     * Function End_Route
     * Terminates a track currently being created
     * @param aTrack = the current track segment in progress
     * @param aDC = the current device context
     * @return true if the track was created, false if not (due to a DRC error)
     */
    bool End_Route( TRACK* aTrack, wxDC* aDC );

    void Attribut_Segment( TRACK* track, wxDC* DC, bool Flag_On );
    void Attribut_Track( TRACK* track, wxDC* DC, bool Flag_On );
    void Attribut_net( wxDC* DC, int net_code, bool Flag_On );

    /**
     * Function StartMoveOneNodeOrSegment
     * initializes the parameters to move one  via or/and a terminal point of a track segment
     * The terminal point of other connected segments (if any) are moved too.
     */
    void StartMoveOneNodeOrSegment( TRACK* aTrack, wxDC* aDC, int aCommand );

    bool PlaceDraggedOrMovedTrackSegment( TRACK* Track, wxDC* DC );

    /**
     * @todo This function is broken, because it merge segments having different
     *       widths or without any connectivity test.
     * 2 collinear segments can be merged only if no other segment or via is
     * connected to the common point and if they have the same width. See
     * cleanup.cpp for merge functions and consider MarkTrace() to locate segments
     * that can be merged
     */
    bool MergeCollinearTracks( TRACK* track, wxDC* DC, int end );

    void Start_DragTrackSegmentAndKeepSlope( TRACK* track, wxDC* DC );
    void SwitchLayer( wxDC* DC, PCB_LAYER_ID layer ) override;

    /**
     * Function Add45DegreeSegment
     * adds a track segment between 2 tracks segments if these 2 segments
     * make a 90 deg angle, in order to have 45 deg track segments
     * Its only works on horizontal or vertical segments.
     *
     * @param aDC The wxDC device context to draw on.
     * @return A bool value true if ok or false if not.
     */
    bool Add45DegreeSegment( wxDC* aDC );

    /**
     * Function EraseRedundantTrack
     * Called after creating a track
     * Remove (if exists) the old track that have the same starting and the
     * same ending point as the new created track
     * (this is the redunding track)
     * @param aDC = the current device context (can be NULL)
     * @param aNewTrack = the new created track (a pointer to a segment of the
     *                    track list)
     * @param aNewTrackSegmentsCount = number of segments in this new track
     * @param aItemsListPicker = the list picker to use for an undo command
     *                           (can be NULL)
     */
    int EraseRedundantTrack( wxDC*              aDC,
                             TRACK*             aNewTrack,
                             int                aNewTrackSegmentsCount,
                             PICKED_ITEMS_LIST* aItemsListPicker );

    /**
     * Function SetTrackSegmentWidth
     *  Modify one track segment width or one via diameter (using DRC control).
     *  Basic routine used by other routines when editing tracks or vias.
     *  Note that casting this to boolean will allow you to determine whether any action
     *  happened.
     * @param aTrackItem = the track segment or via to modify
     * @param aItemsListPicker = the list picker to use for an undo command
     *                           (can be NULL)
     * @param aUseNetclassValue = true to use NetClass value, false to use
     *                            current designSettings value
     * @return  0 if items successfully changed,
     *          -1 if there was a DRC error,
     *          1 if items were changed successfully
     */
    int SetTrackSegmentWidth( TRACK*             aTrackItem,
                               PICKED_ITEMS_LIST* aItemsListPicker,
                               bool               aUseNetclassValue );


    // zone handling

    /**
     * Function Delete_OldZone_Fill (obsolete)
     * Used for compatibility with old boards
     * Remove the zone filling which include the segment aZone, or the zone
     * which have the given time stamp.
     * For old boards, a zone is a group of SEGZONE segments which have the same TimeStamp
     * @param aZone = zone segment within the zone to delete. Can be NULL
     * @param aTimestamp = Timestamp for the zone to delete, used if aZone ==
     *                     NULL
     */
    void Delete_OldZone_Fill( SEGZONE* aZone, timestamp_t aTimestamp = 0 );

    /**
     * Function Delete_LastCreatedCorner
     * Used only while creating a new zone outline
     * Remove and delete the current outline segment in progress
     * @return 0 if no corner in list, or corner number
     */
    int Delete_LastCreatedCorner( wxDC* DC );

    /**
     * Function Begin_Zone
     * either initializes the first segment of a new zone, or adds an
     * intermediate segment.
     * A new zone can be:
     * created from scratch: the user will be prompted to define parameters (layer, clearence ...)
     * created from a similar zone (s_CurrentZone is used): parameters are copied from
     * s_CurrentZone
     * created as a cutout (an hole) inside s_CurrentZone
     */
    int Begin_Zone( wxDC* DC );

    /**
     * Function End_Zone
     * terminates (if no DRC error ) the zone edge creation process
     * @param DC = current Device Context
     * @return true if Ok, false if DRC error
     */
    bool End_Zone( wxDC* DC );

    /**
     * Function Fill_Zone
     *  Calculate the zone filling for the outline zone_container
     *  The zone outline is a frontier, and can be complex (with holes)
     *  The filling starts from starting points like pads, tracks.
     * If exists the old filling is removed
     * @param aZone = zone to fill
     * @return error level (0 = no error)
     */
    int Fill_Zone( ZONE_CONTAINER* aZone );

    /**
     * Function Fill_All_Zones
     *  Fill all zones on the board
     * The old fillings are removed
     * @param aActiveWindow = the current active window, if a progress bar is shown.
     * the progress bar will be on top of aActiveWindow
     * aActiveWindow = NULL to do not display a progress bar
     */
    int Fill_All_Zones( wxWindow * aActiveWindow );

    /**
     * Function Check_All_Zones
     *  Checks for out-of-date fills and fills them if requested by the user.
     * @param aActiveWindow
     */
    void Check_All_Zones( wxWindow* aActiveWindow );


    /**
     * Function Add_Zone_Cutout
     * Add a cutout zone to a given zone outline
     * @param DC = current Device Context
     * @param zone_container = parent zone outline
     */
    void Add_Zone_Cutout( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Add_Similar_Zone
     * Add a zone to a given zone outline.
     * if the zones are overlapping they will be merged
     * @param DC = current Device Context
     * @param zone_container = parent zone outline
     */
    void Add_Similar_Zone( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Edit_Zone_Params
     * Edit params (layer, clearance, ...) for a zone outline
     */
    void Edit_Zone_Params( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Start_Move_Zone_Corner
     * Prepares a move corner in a zone outline,
     * called from a move corner command (IsNewCorner = false),
     * or a create new cornet command (IsNewCorner = true )
     */
    void Start_Move_Zone_Corner( wxDC*           DC,
                                 ZONE_CONTAINER* zone_container,
                                 int             corner_id,
                                 bool            IsNewCorner );

    /**
     * Function Start_Move_Zone_Corner
     * Prepares a drag edge in an existing zone outline,
     */
    void Start_Move_Zone_Drag_Outline_Edge( wxDC*            DC,
                                            ZONE_CONTAINER* zone_container,
                                            int             corner_id );

    /**
     * Function End_Move_Zone_Corner_Or_Outlines
     * Terminates a move corner in a zone outline, or a move zone outlines
     * @param DC = current Device Context (can be NULL)
     * @param zone_container: the given zone
     */
    void End_Move_Zone_Corner_Or_Outlines( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function End_Move_Zone_Corner_Or_Outlines
     * Remove the currently selected corner in a zone outline
     * the .m_CornerSelection is used as corner selection
     */
    void Remove_Zone_Corner( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Delete_Zone
     * Remove the zone which include the segment aZone, or the zone which have
     * the given time stamp.  A zone is a group of segments which have the
     * same TimeStamp
     * @param DC = current Device Context (can be NULL)
     * @param zone_container = zone to modify
     *  the member .m_CornerSelection is used to find the outline to remove.
     * if the outline is the main outline, all the zone is removed
     * otherwise, the hole is deleted
     */
    void Delete_Zone_Contour( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Start_Move_Zone_Outlines
     * Initialize parameters to move an existing zone outlines.
     * @param DC = current Device Context (can be NULL)
     * @param zone_container: the given zone to move
     */
    void Start_Move_Zone_Outlines( wxDC* DC, ZONE_CONTAINER* zone_container );

    // Target handling
    PCB_TARGET* CreateTarget( wxDC* DC );
    void DeleteTarget( PCB_TARGET* aTarget, wxDC* DC );
    void BeginMoveTarget( PCB_TARGET* aTarget, wxDC* DC );
    void PlaceTarget( PCB_TARGET* aTarget, wxDC* DC );
    void ShowTargetOptionsDialog( PCB_TARGET* aTarget, wxDC* DC );

    // Graphic segments type DRAWSEGMENT handling:
    DRAWSEGMENT* Begin_DrawSegment( DRAWSEGMENT* Segment, STROKE_T shape, wxDC* DC );
    void End_Edge( DRAWSEGMENT* Segment, wxDC* DC );
    void Delete_Segment_Edge( DRAWSEGMENT* Segment, wxDC* DC );
    void Delete_Drawings_All_Layer( PCB_LAYER_ID aLayer );

    // Dimension handling:
    void ShowDimensionPropertyDialog( DIMENSION* aDimension, wxDC* aDC );
    DIMENSION* EditDimension( DIMENSION* aDimension, wxDC* aDC );
    void DeleteDimension( DIMENSION* aDimension, wxDC* aDC );
    void BeginMoveDimensionText( DIMENSION* aItem, wxDC* DC );
    void PlaceDimensionText( DIMENSION* aItem, wxDC* DC );


    // netlist  handling:
    void InstallNetlistFrame( wxDC* DC );

    /**
     * Function ReadPcbNetlist
     * reads \a aNetlistFileName and updates the footprints (load missing footprints and
     * delete on demand extra footprints) on the board.
     * Update connectivity info, references, values and "TIME STAMP"
     *
     * @param aNetlistFileName = netlist file name (*.net)
     * @param aCmpFileName = cmp/footprint link file name (*.cmp).
     *                       if not found or empty, only the netlist will be used
     * @param aReporter a #REPORTER object to write display messages.
     * @param aChangeFootprint if true, footprints that have changed in netlist will be changed
     * @param aDeleteBadTracks if true, erroneous tracks will be deleted
     * @param aDeleteExtraFootprints if true, remove unlocked footprints that are not in netlist
     * @param aSelectByTimestamp if true, use timestamp instead of reference to identify
     *                           footprints from components (use after reannotation of the
     *                           schematic)
     * @param aDeleteSinglePadNets if true, remove nets counting only one pad
     *                             and set net code to 0 for these pads
     * @param aIsDryRun performs a dry run without making any changes if true.
     * @param runDragCommand indicates that a selection was created which should be dragged.
     */
    void ReadPcbNetlist( const wxString&  aNetlistFileName,
                         const wxString&  aCmpFileName,
                         REPORTER&        aReporter,
                         bool             aChangeFootprint,
                         bool             aDeleteBadTracks,
                         bool             aDeleteExtraFootprints,
                         bool             aSelectByTimestamp,
                         bool             aDeleteSinglePadNets,
                         bool             aIsDryRun,
                         bool*            runDragCommand );

    /**
     * Function RemoveMisConnectedTracks
     * finds all track segments which are mis-connected (to more than one net).
     * When such a bad segment is found, it is flagged to be removed.
     * All tracks having at least one flagged segment are removed.
     * @return true if any change is made
     */
    bool RemoveMisConnectedTracks();


    // Autoplacement:
    void OnPlaceOrRouteFootprints( wxCommandEvent& event );

#if defined( KICAD_SCRIPTING_WXPYTHON )

    /**
     * Function ScriptingConsoleEnableDisable
     * enables or disabled the scripting console
     */
    void ScriptingConsoleEnableDisable( wxCommandEvent& aEvent );

    void OnUpdateScriptingConsoleState( wxUpdateUIEvent& aEvent );

#endif

    void LockModule( MODULE* aModule, bool aLocked );

    /**
     * Function SpreadFootprints
     * Footprints (after loaded by reading a netlist for instance) are moved
     * to be in a small free area (outside the current board) without overlapping.
     * @param aFootprints: a list of footprints to be spread out.
     * @param aMoveFootprintsOutsideBoardOnly: true to move only
     *        footprints outside the board outlines
     *        (they are outside if the position of a footprint anchor is outside
     *        the board outlines bounding box). It imply the board outlines exist
     * @param aCheckForBoardEdges: true to try to place footprints outside of
     *        board edges, if aSpreadAreaPosition is incorrectly chosen.
     * @param aSpreadAreaPosition the position of the upper left corner of the
     *        area used to spread footprints
     * @param aPrepareUndoCommand = true (defualt) to commit a undo command for the
     * spread footprints, false to do just the spread command
     * (no undo specific to this move command)
     */
    void SpreadFootprints( std::vector<MODULE*>* aFootprints,
                           bool                  aMoveFootprintsOutsideBoardOnly,
                           bool                  aCheckForBoardEdges,
                           wxPoint               aSpreadAreaPosition,
                           bool                  aPrepareUndoCommand = true );


    /**
     * Function Show_1_Ratsnest
     * draw ratsnest.
     *
     * The net edge pad with mouse or module locates the mouse.
     * Delete the ratsnest if no module or pad is selected.
     */
    void Show_1_Ratsnest( EDA_ITEM* item, wxDC* DC );

    /**
     * Function Clean_Pcb
     * Clean up the board (remove redundant vias, not connected tracks
     * and merges collinear track segments)
     * Install the cleanup dialog frame to know what should be cleaned
     * and run the cleanup function
     */
    void Clean_Pcb();

    void InstallFindFrame();

    /**
     * Function SendMessageToEESCHEMA
     * sends a message to the schematic editor so that it may move its cursor
     * to a part with the same reference as the objectToSync
     * @param objectToSync The object whose reference is used to synchronize Eeschema.
     */
    void SendMessageToEESCHEMA( BOARD_ITEM* objectToSync );

    /**
     * Sends a net name to eeschema for highlighting
     *
     * @param aNetName is the name of a net, or empty string to clear highlight
     */
    void SendCrossProbeNetName( const wxString& aNetName );

    /**
     * Function Edit_Gap
     * edits the GAP module if it has changed the position and/or size of the pads that
     * form the gap get a new value.
     */
    void Edit_Gap( wxDC* DC, MODULE* Module );

    /**
     * Function CreateMuWaveBaseFootprint
     * create a basic footprint for micro wave applications.
     * @param aValue = the text value
     * @param aTextSize = the size of ref and value texts ( <= 0 to use board default values )
     * @param aPadCount = number of pads
     * Pads settings are:
     *  PAD_ATTRIB_SMD, rectangular, H size = V size = current track width.
     */
    MODULE* CreateMuWaveBaseFootprint( const wxString& aValue, int aTextSize, int aPadCount );

    /**
     * Create_MuWaveComponent
     * creates a module "GAP" or "STUB" used in micro wave designs.
     *  This module has 2 pads:
     *  PAD_ATTRIB_SMD, rectangular, H size = V size = current track width.
     *  the "gap" is isolation created between this 2 pads
     */
    MODULE* Create_MuWaveComponent( int shape_type );

    MODULE* Create_MuWavePolygonShape();

    void Begin_Self( wxDC* DC );

    void ShowChangedLanguage() override;

    /**
     * Function UpdateTitle
     * sets the main window title bar text.
     * <p>
     * If file name defined by PCB_SCREEN::m_FileName is not set, the title is set to the
     * application name appended with no file.  Otherwise, the title is set to the full path
     * and file name and read only is appended to the title if the user does not have write
     * access to the file.
     * </p>
     */
    void UpdateTitle();

    /**
     * Allows Pcbnew to install its preferences panel into the preferences dialog.
     */
    void InstallPreferences( PAGED_DIALOG* aParent ) override;

    /**
     * Called after the preferences dialog is run.
     */
    void CommonSettingsChanged() override;

    void SyncMenusAndToolbars( wxEvent& aEvent ) override;

    DECLARE_EVENT_TABLE()
};

#endif  // WXPCB_STRUCT_H_
