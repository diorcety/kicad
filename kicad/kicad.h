/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.txt for contributors.
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
 * @file kicad/kicad.h
 * @brief KICAD_MANAGER_FRAME is the KiCad main frame.
 */

#ifndef KICAD_H
#define KICAD_H


#include <wx/process.h>

#include <id.h>
#include <eda_base_frame.h>
#include <kiway_player.h>
#include <mutex>

#define KICAD_MANAGER_FRAME_NAME   wxT( "KicadFrame" )

class LAUNCHER_PANEL;
class TREEPROJECTFILES;
class TREE_PROJECT_FRAME;

// Identify the type of files handled by Kicad manager
//
// When changing this enum  please verify (and perhaps update)
// TREE_PROJECT_FRAME::GetFileExt(),
// s_AllowedExtensionsToList[]

enum TreeFileType {
    TREE_PROJECT = 1,
    TREE_SCHEMA,            // Schematic file (.sch)
    TREE_LEGACY_PCB,        // board file (.brd) legacy format
    TREE_SEXP_PCB,          // board file (.kicad_brd) new s expression format
    TREE_GERBER,            // Gerber  file (.pho, .g*)
    TREE_GERBER_JOB_FILE,   // Gerber  file (.gbrjob)
    TREE_HTML,              // HTML file (.htm, *.html)
    TREE_PDF,               // PDF file (.pdf)
    TREE_TXT,               // ascii text file (.txt)
    TREE_NET,               // netlist file (.net)
    TREE_UNKNOWN,
    TREE_DIRECTORY,
    TREE_CMP_LINK,          // cmp/footprint link file (.cmp)
    TREE_REPORT,            // report file (.rpt)
    TREE_FP_PLACE,          // fooprints position (place) file (.pos)
    TREE_DRILL,             // Excellon drill file (.drl)
    TREE_DRILL_NC,          // Similar Excellon drill file (.nc)
    TREE_DRILL_XNC,         // Similar Excellon drill file (.xnc)
    TREE_SVG,               // SVG file (.svg)
    TREE_PAGE_LAYOUT_DESCR, // Page layout and title block descr file (.kicad_wks)
    TREE_FOOTPRINT_FILE,    // footprint file (.kicad_mod)
    TREE_SCHEMATIC_LIBFILE, // schematic library file (.lib)
    TREE_MAX
};


/**
 * Command IDs for KiCad.
 *
 * Please add IDs that are unique to Kicad here and not in the global id.h file.
 * This will prevent the entire project from being rebuilt when adding
 * new commands to KiCad.
 *
 * However, now the Kicad manager and other sub applications are running inside
 * the same application, these IDs are kept unique inside the whole Kicad code
 * See the global id.h which reserves room for the Kicad manager IDs
 * and expand this room if needed
 *
 * We have experienced issues with duplicate menus IDs between frames
 * because wxUpdateUIEvent events are sent to parent frames, when a wxUpdateUIEvent
 * event function does not exists for some menuitems ID, and therefore
 * with duplicate menuitems IDs in different frames, the wrong menuitem can be used
 * by a function called by the wxUpdateUIEvent event loop.
 *
 * The number of items in this list should be less than ROOM_FOR_KICADMANAGER (see id.h)
 */

enum id_kicad_frm {
    ID_LEFT_FRAME = ID_KICAD_MANAGER_START,
    ID_PROJECT_TREE,
    ID_PROJECT_TXTEDIT,
    ID_PROJECT_TREE_REFRESH,
    ID_PROJECT_SWITCH_TO_OTHER,
    ID_PROJECT_NEWDIR,
    ID_PROJECT_DELETE,
    ID_PROJECT_RENAME,
    ID_PROJECT_OPEN_FILE_WITH_TEXT_EDITOR,

    ID_TO_SCH,
    ID_TO_SCH_LIB_EDITOR,
    ID_TO_PCB,
    ID_TO_PCB_FP_EDITOR,
    ID_TO_CVPCB,
    ID_TO_GERBVIEW,
    ID_TO_BITMAP_CONVERTER,
    ID_TO_PCB_CALCULATOR,
    ID_TO_PL_EDITOR,

    ID_TO_TEXT_EDITOR,
    ID_BROWSE_AN_SELECT_FILE,
    ID_BROWSE_IN_FILE_EXPLORER,
    ID_SAVE_AND_ZIP_FILES,
    ID_READ_ZIP_ARCHIVE,
    ID_INIT_WATCHED_PATHS,
    ID_IMPORT_EAGLE_PROJECT,

    // Please, verify: the number of items in this list should be
    // less than ROOM_FOR_KICADMANAGER (see id.h)
    ID_KICADMANAGER_END_LIST
};


/**
 * The main KiCad project manager frame.  It is not a KIWAY_PLAYER.
 */
class KICAD_MANAGER_FRAME : public EDA_BASE_FRAME, public KIWAY_HOLDER
{
public:
    KICAD_MANAGER_FRAME( wxWindow* parent, const wxString& title,
                         const wxPoint& pos, const wxSize& size );

    ~KICAD_MANAGER_FRAME();

    void OnCloseWindow( wxCloseEvent& Event );
    void OnSize( wxSizeEvent& event );

    /**
     * Load an exiting project (.pro) file.
     */
    void OnLoadProject( wxCommandEvent& event );

    /**
     * Creates a new project folder, copy a template into this new folder.
     * and open this new projrct as working project
     */
    void OnCreateProjectFromTemplate( wxCommandEvent& event );

    void OnNewProject( wxCommandEvent& aEvent );

    /**
     * Save the project (.pro) file containing the top level configuration parameters.
     */
    void OnSaveProject( wxCommandEvent& event );

    void OnArchiveFiles( wxCommandEvent& event );
    void OnUnarchiveFiles( wxCommandEvent& event );

    void OnRunEeschema( wxCommandEvent& event );
    void OnRunSchLibEditor( wxCommandEvent& event );
    void OnRunPcbNew( wxCommandEvent& event );
    void OnRunPcbFpEditor( wxCommandEvent& event );
    void OnRunGerbview( wxCommandEvent& event );
    void OnRunBitmapConverter( wxCommandEvent& event );
    void OnRunPcbCalculator( wxCommandEvent& event );
    void OnRunPageLayoutEditor( wxCommandEvent& event );

    void OnConfigurePaths( wxCommandEvent& aEvent );
    void OnEditSymLibTable( wxCommandEvent& aEvent );
    void OnEditFpLibTable( wxCommandEvent& aEvent );
    void OnPreferences( wxCommandEvent& aEvent );
    void OnOpenTextEditor( wxCommandEvent& event );
    void OnOpenFileInTextEditor( wxCommandEvent& event );
    void OnBrowseInFileExplorer( wxCommandEvent& event );
    void OnShowHotkeys( wxCommandEvent& event );

    void OnFileHistory( wxCommandEvent& event );
    void OnExit( wxCommandEvent& event );

    void ReCreateMenuBar() override;
    void RecreateBaseHToolbar();

    /**
     *  Open dialog to import Eagle schematic and board files.
     */
    void OnImportEagleFiles( wxCommandEvent& event );

    /**
     * Displays \a aText in the text panel.
     *
     * @param aText The text to display.
     */
    void PrintMsg( const wxString& aText );

    /**
     * Prints the current working directory name and the projet name on the text panel.
     */
    void PrintPrjInfo();

    /**
     * Erase the text panel.
     */
    void ClearMsg();

    void OnRefresh( wxCommandEvent& event );
    void OnUpdateRequiresProject( wxUpdateUIEvent& event );

    /**
     * Creates a new project by setting up and initial project, schematic, and board files.
     *
     * The project file is copied from the kicad.pro template file if possible.  Otherwise,
     * a minimal project file is created from an empty project.  A minimal schematic and
     * board file are created to prevent the schematic and board editors from complaining.
     * If any of these files already exist, they are not overwritten.
     *
     * @param aProjectFileName is the absolute path of the project file name.
     */
    void CreateNewProject( const wxFileName& aProjectFileName );
    void LoadProject( const wxFileName& aProjectFileName );


    void LoadSettings( wxConfigBase* aCfg ) override;

    void SaveSettings( wxConfigBase* aCfg ) override;

    void CommonSettingsChanged() override;

    /**
     * Open another KiCad application and logs a message.
     *
     * @param frame = owner frame.
     * @param execFile = name of the executable file.
     * @param param = parameters to be passed to the executable.
     */
    void Execute( wxWindow* frame, const wxString& execFile,
                  wxString param = wxEmptyString );

    class TERMINATE_HANDLER : public wxProcess
    {
    private:
        wxString m_appName;

    public:
        TERMINATE_HANDLER( const wxString& appName ) :
            m_appName( appName )
        {
        }

        void OnTerminate( int pid, int status ) override;
    };

    /**
     * Called by sending a event with id = ID_INIT_WATCHED_PATHS
     * rebuild the list of wahtched paths
     */
    void OnChangeWatchedPaths( wxCommandEvent& aEvent );


    void SetProjectFileName( const wxString& aFullProjectProFileName );
    const wxString GetProjectFileName();

    // read only accessors
    const wxString SchFileName();
    const wxString PcbFileName();
    const wxString PcbLegacyFileName();

    void ReCreateTreePrj();

    /// Call this only for a PCB associated with the current project.  That is,
    /// it must have the same path and name as the project *.pro file.
    void RunPcbNew( const wxString& aProjectBoardFileName );

    /// Call this only for a SCH associated with the current project.  That is,
    /// it must have the same path and name as the project *.pro file.
    void RunEeschema( const wxString& aProjectSchematicFileName );

    DECLARE_EVENT_TABLE()

private:
    wxConfigBase*       config() override;

    const SEARCH_STACK& sys_search() override;

    wxString help_name() override;

    TREE_PROJECT_FRAME* m_LeftWin;
    LAUNCHER_PANEL*     m_Launcher;
    wxTextCtrl*         m_MessagesBox;
    wxAuiToolBar*       m_VToolBar;             // Vertical toolbar (not used)

    int m_leftWinWidth;
    EDA_HOTKEY_CONFIG* m_manager_Hotkeys_Descr;

    void language_change( wxCommandEvent& event );

    bool m_active_project;

    // Mutex to allow only a single KiFace to load at one time (released when loaded)
    std::mutex m_loading;
};


/** class LAUNCHER_PANEL
 */
class LAUNCHER_PANEL : public wxPanel
{
private:
    wxBoxSizer* m_buttonSizer;

    int m_height = 0;
    int m_width  = 0;

public: LAUNCHER_PANEL( wxWindow* parent );
    ~LAUNCHER_PANEL() { };

    int GetPanelHeight() const;
    int GetPanelWidth() const;

private:

    /**
     * Function CreateCommandToolbar
     * creates the main tool bar buttons (fast launch buttons)
     */
    void            CreateCommandToolbar( void );

    void AddButton( wxWindowID aId, const wxBitmap& aBitmap, const wxString& aToolTip );
};

// The C++ project manager includes a single PROJECT in its link image.
class PROJECT;
extern PROJECT& Prj();

#endif
