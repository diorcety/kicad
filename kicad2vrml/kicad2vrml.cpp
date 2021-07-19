/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 1992-2010 jean-pierre.charras
 * Copyright (C) 1992-2019 Kicad Developers, see change_log.txt for contributors.
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
#include <macros.h>

#include <wx/stdpaths.h>
#include <wx/cmdline.h>
#include <pgm_base.h>
#include <confirm.h>
#include <gestfich.h>
#include <wildcards_and_files_ext.h>
#include <build_version.h>
#include <kiway.h>
#include <kiface_i.h>
#include <kiway_player.h>
#include "export_vrml.h"
#include "io_mgr.h"
#include "3d_viewer/eda_3d_viewer.h"
#include <tools/selection_tool.h>
#include <tool/selection.h>
#include <fp_lib_table.h>
#include "class_board.h"
#include <tools/pcb_actions.h>
#include <hotkeys.h>

//-----<KIFACE>-----------------------------------------------------------------
// Only a single KIWAY is supported in this single_top top level component,
// which is dedicated to loading only a single DSO.
KIWAY    Kiway( &Pgm(), KFCTL_STANDALONE );

namespace KICAD2VRML {

static struct IFACE : public KIFACE_I
{
    bool OnKifaceStart( PGM_BASE* aProgram, int aCtlBits ) override
    {
        return start_common( aCtlBits );
    }

    wxWindow* CreateWindow( wxWindow* aParent, int aClassId, KIWAY* aKiway, int aCtlBits = 0 ) override
    {
        return nullptr;
    }

    /**
     * Function IfaceOrAddress
     * return a pointer to the requested object.  The safest way to use this
     * is to retrieve a pointer to a static instance of an interface, similar to
     * how the KIFACE interface is exported.  But if you know what you are doing
     * use it to retrieve anything you want.
     *
     * @param aDataId identifies which object you want the address of.
     *
     * @return void* - and must be cast into the know type.
     */
    void* IfaceOrAddress( int aDataId ) override
    {
        return NULL;
    }

    IFACE( const char* aDSOname, KIWAY::FACE_T aType ) :
        KIFACE_I( aDSOname, aType )
    {}

} kiface( "KICAD2VRML", KIWAY::FACE_KICAD2VRML );

}   // namespace KICAD2VRML

using namespace KICAD2VRML;

KIFACE_I& Kiface()
{
    return kiface;
}

static struct PGM_KICAD2VRML_TOP : public PGM_BASE
{
    bool OnPgmInit();

    void OnPgmExit()
    {
        Kiway.OnKiwayEnd();

        // Destroy everything in PGM_BASE, especially wxSingleInstanceCheckerImpl
        // earlier than wxApp and earlier than static destruction would.
        PGM_BASE::Destroy();
    }

    void MacOpenFile( const wxString& aFileName )   override
    {
        wxFileName filename( aFileName );

        if( filename.FileExists() )
        {
    #if 0
            // this pulls in EDA_DRAW_FRAME type info, which we don't want in
            // the single_top link image.
            KIWAY_PLAYER* frame = dynamic_cast<KIWAY_PLAYER*>( App().GetTopWindow() );
    #else
            KIWAY_PLAYER* frame = (KIWAY_PLAYER*) App().GetTopWindow();
    #endif

            if( frame )
                frame->OpenProjectFiles( std::vector<wxString>( 1, aFileName ) );
        }
    }

} program;


PGM_BASE& Pgm()
{
    return program;
}


bool PGM_KICAD2VRML_TOP::OnPgmInit()
{
#if defined(DEBUG)
    wxString absoluteArgv0 = wxStandardPaths::Get().GetExecutablePath();

    if( !wxIsAbsolutePath( absoluteArgv0 ) )
    {
        wxLogError( wxT( "No meaningful argv[0]" ) );
        return false;
    }
#endif

    if( !InitPgm() )
        return false;

    return true;
}

static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_PARAM, NULL, NULL, _( "pcb_filename" ).mb_str(),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
        { wxCMD_LINE_OPTION, "o", "output-filename", _( "output filename" ).mb_str(),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_SWITCH, "f", "force", _( "overwrite output file" ).mb_str(),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_OPTION, NULL, "user-origin",
            _( "User-specified output origin ex. 1x1in, 1x1inch, 25.4x25.4mm (default mm)" ).mb_str(),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_NONE }
    };

/**
 * Struct APP_KICAD2VRML
 * implements a bare naked wxApp (so that we don't become dependent on
 * functionality in a wxApp derivative that we cannot deliver under wxPython).
 */
struct APP_KICAD2VRML : public wxAppConsole
{
private:
    bool     m_overwrite;
    wxString m_filename;
    wxString m_outputFile;
    double   m_xOrigin;
    double   m_yOrigin;
public:
#if defined (__LINUX__)
    APP_KICAD2VRML(): wxAppConsole()
    {
        // Disable proxy menu in Unity window manager. Only usual menubar works with wxWidgets (at least <= 3.1)
        // When the proxy menu menubar is enable, some important things for us do not work: menuitems UI events and shortcuts.
        wxString wm;

        if( wxGetEnv( wxT( "XDG_CURRENT_DESKTOP" ), &wm ) && wm.CmpNoCase( wxT( "Unity" ) ) == 0 )
        {
            wxSetEnv ( wxT("UBUNTU_MENUPROXY" ), wxT( "0" ) );
        }

        // Force the use of X11 backend (or wayland-x11 compatibilty layer).  This is required until wxWidgets
        // supports the Wayland compositors
        wxSetEnv( wxT( "GDK_BACKEND" ), wxT( "x11" ) );

        // Disable overlay scrollbars as they mess up wxWidgets window sizing and cause excessive redraw requests
        wxSetEnv( wxT( "GTK_OVERLAY_SCROLLING" ), wxT( "0" ) );

        // Set GTK2-style input instead of xinput2.  This disables touchscreen and smooth scrolling
        // Needed to ensure that we are not getting multiple mouse scroll events
        wxSetEnv( wxT( "GDK_CORE_DEVICE_EVENTS" ), wxT( "1" ) );
    }
#endif

    virtual void OnInitCmdLine(wxCmdLineParser& parser) override
    {
        parser.SetDesc( cmdLineDesc );
        parser.SetSwitchChars( "-" );
        return;
    }

    virtual bool OnCmdLineParsed(wxCmdLineParser& parser) override
    {
        wxString tstr;

        if( parser.Found( "f" ) )
            m_overwrite = true;

        if( parser.Found( "user-origin", &tstr ) )
        {
            std::istringstream istr;
            istr.str( std::string( tstr.ToUTF8() ) );
            istr >> m_xOrigin;

            if( istr.fail() )
            {
                parser.Usage();
                return false;
            }

            char tmpc;
            istr >> tmpc;

            if( istr.fail() || ( tmpc != 'x' && tmpc != 'X' ) )
            {
                parser.Usage();
                return false;
            }

            istr >> m_yOrigin;

            if( istr.fail() )
            {
                parser.Usage();
                return false;
            }

            if( !istr.eof() )
            {
                std::string tunit;
                istr >> tunit;

                if( !tunit.compare( "in" ) || !tunit.compare( "inch" ) )
                {
                    m_xOrigin *= 25.4;
                    m_yOrigin *= 25.4;
                }
                else if( tunit.compare( "mm" ) )
                {
                    parser.Usage();
                    return false;
                }
            }
        }

        if( parser.Found( "o", &tstr ) )
            m_outputFile = tstr;


        if( parser.GetParamCount() < 1 )
        {
            parser.Usage();
            return false;
        }

        m_filename = parser.GetParam( 0 );

        return true;
    }

    wxString getOutputExt() const
    {
        return wxString( "wrl" );
    }

    bool OnInit() override
    {
        try
        {
            program.OnPgmInit();
 
            if( !wxAppConsole::OnInit() )
                return false;

            return true;
        }
        catch( const std::exception& e )
        {
            wxLogError( wxT( "Unhandled exception class: %s  what: %s" ),
                GetChars( FROM_UTF8( typeid(e).name() )),
                GetChars( FROM_UTF8( e.what() ) ) );
        }
        catch( const IO_ERROR& ioe )
        {
            wxLogError( GetChars( ioe.What() ) );
        }
        catch(...)
        {
            wxLogError( wxT( "Unhandled exception of unknown type" ) );
        }

        return false;
    }

    int OnExit() override
    {
        program.OnPgmExit();
        return wxAppConsole::OnExit();
    }

    int OnRun() override
    {
        int ret = -1;

        try
        {
            wxFileName fname( m_filename );

            if( !fname.FileExists() )
            {
                std::ostringstream ostr;
                ostr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                ostr << "  * no such file: '" << m_filename.ToUTF8() << "'\n";
                throw std::runtime_error(ostr.str().c_str());
            }

            wxFileName tfname;

            if( m_outputFile.empty() )
            {
                tfname.Assign( fname.GetFullPath() );
                tfname.SetExt( getOutputExt() );
            }
            else
            {
                tfname.Assign( m_outputFile );

                // Set the file extension if the user's requested
                // file name does not have an extension.
                if( !tfname.HasExt() )
                    tfname.SetExt( getOutputExt() );
            }

            if( tfname.FileExists() && !m_overwrite )
            {
                throw std::runtime_error("** Output already exists. Enable the force overwrite flag to overwrite it.");

            }

            wxString outfile = tfname.GetFullPath();
            wxFileName projdir(  fname.GetPath(), "" );
            wxString m_curProjDir = projdir.GetPath();
            wxSetEnv( "KIPRJMOD", m_curProjDir );

            KIWAY_HOLDER kiway_holder( &::Kiway );
            std::unique_ptr<BOARD> brd(IO_MGR::Load( IO_MGR::KICAD_SEXP, m_filename ));

            if( brd )
            {
                brd->BuildConnectivity();
                brd->BuildListOfNets();
                brd->SynchronizeNetsAndNetClasses();
                if (VRML_WRITER::ExportVRML_File(kiway_holder.Prj(), brd.get(), outfile.mb_str(), 1.0, false, false, true, "", m_xOrigin, m_yOrigin))
                {
                    ret = 0;
                }
            }

        }
        catch( const std::exception& e )
        {
            wxLogError( GetChars( e.what() ) );
        }
        catch( const IO_ERROR& ioe )
        {
            wxLogError( GetChars( ioe.What() ) );
        }
        catch(...)
        {
            wxLogError( wxT( "Unhandled exception of unknown type" ) );
        }

        // Works properly when wxPython scripting is disabled.
#if !defined( KICAD_SCRIPTING_WXPYTHON )
        program.OnPgmExit();
#endif
        return ret;
    }


#if defined( DEBUG )
    /**
     * Override main loop exception handling on debug builds.
     *
     * It can be painfully difficult to debug exceptions that happen in wxUpdateUIEvent
     * handlers.  The override provides a bit more useful information about the exception
     * and a breakpoint can be set to pin point the event where the exception was thrown.
     */
    virtual bool OnExceptionInMainLoop() override
    {
        try
        {
            throw;
        }
        catch( const std::exception& e )
        {
            wxLogError( "Unhandled exception class: %s  what: %s",
                        FROM_UTF8( typeid(e).name() ),
                        FROM_UTF8( e.what() ) );
        }
        catch( const IO_ERROR& ioe )
        {
            wxLogError( ioe.What() );
        }
        catch(...)
        {
            wxLogError( "Unhandled exception of unknown type" );
        }

        return false;   // continue on. Return false to abort program
    }
#endif

#ifdef __WXMAC__

    /**
     * Function MacOpenFile
     * is specific to MacOSX (not used under Linux or Windows).
     * MacOSX requires it for file association.
     * @see http://wiki.wxwidgets.org/WxMac-specific_topics
     */
    void MacOpenFile( const wxString& aFileName ) override
    {
        Pgm().MacOpenFile( aFileName );
    }

#endif
};

IMPLEMENT_APP_CONSOLE( APP_KICAD2VRML )

// Mocks

FP_LIB_TABLE GFootprintTable;

TOOL_ACTION PCB_ACTIONS::flip( "pcbnew.InteractiveEdit.flip",
        AS_GLOBAL, TOOL_ACTION::LegacyHotKey( HK_FLIP_ITEM ),
        _( "Flip" ), _( "Flips selected item(s)" ), nullptr );

TOOL_ACTION PCB_ACTIONS::rotateCw( "pcbnew.InteractiveEdit.rotateCw",
        AS_GLOBAL, MD_SHIFT + 'R',
        _( "Rotate Clockwise" ), _( "Rotates selected item(s) clockwise" ),
        nullptr, AF_NONE, (void*) -1 );

TOOL_ACTION PCB_ACTIONS::rotateCcw( "pcbnew.InteractiveEdit.rotateCcw",
        AS_GLOBAL, TOOL_ACTION::LegacyHotKey( HK_ROTATE_ITEM ),
        _( "Rotate Counterclockwise" ), _( "Rotates selected item(s) counterclockwise" ),
        nullptr, AF_NONE, (void*) 1 );


TOOL_ACTION PCB_ACTIONS::unselectItems( "pcbnew.InteractiveSelection.UnselectItems",
        AS_GLOBAL, 0,
        "", "" );    // No description, it is not supposed to be shown anywhere

TOOL_ACTION PCB_ACTIONS::selectionClear( "pcbnew.InteractiveSelection.Clear",
        AS_GLOBAL, 0,
        "", "" );    // No description, it is not supposed to be shown anywhere

TOOL_ACTION PCB_ACTIONS::properties( "pcbnew.InteractiveEdit.properties",
        AS_GLOBAL, TOOL_ACTION::LegacyHotKey( HK_EDIT_ITEM ),
        _( "Properties..." ), _( "Displays item properties dialog" ), nullptr );

TOOL_ACTION PCB_ACTIONS::highlightNet( "pcbnew.EditorControl.highlightNet",
        AS_GLOBAL, 0,
        "", "" );


void BOARD::Draw( EDA_DRAW_PANEL* aPanel, wxDC* DC, GR_DRAWMODE aDrawMode, const wxPoint& offset )
{
    if (DC == (wxDC*)0xDEADBEAF)
    {
      new EDA_3D_VIEWER(NULL, NULL, "ee");
    }
}


class SELECTION_TOOL::PRIV
{
};

SELECTION_TOOL::SELECTION_TOOL(): 
        PCB_TOOL( "pcbnew.InteractiveSelection" ),
        m_frame( NULL ),
        m_additive( false ),
        m_subtractive( false ),
        m_multiple( false ),
        m_skip_heuristics( false ),
        m_locked( true ),
        m_menu( *this ),
        m_priv( std::make_unique<PRIV>() )
{

}

bool SELECTION_TOOL::Init()
{
    return true;
}

void SELECTION_TOOL::Reset( RESET_REASON aReason )
{
    m_selection.ClearReferencePoint();
}

void SELECTION_TOOL::setTransitions()
{

}

SELECTION& SELECTION_TOOL::GetSelection()
{
    return m_selection;
}


SELECTION_TOOL::~SELECTION_TOOL()
{

}

const KIGFX::VIEW_GROUP::ITEMS SELECTION::updateDrawList() const
{
    std::vector<VIEW_ITEM*> items;
    return items;
}

const BOX2I SELECTION::ViewBBox() const
{
    BOX2I r;
    r.SetMaximum();
    return r;
}
