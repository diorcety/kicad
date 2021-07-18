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

#include <pgm_base.h>
#include <confirm.h>
#include <gestfich.h>
#include <wildcards_and_files_ext.h>
#include <build_version.h>
#include <kiway.h>
#include <kiface_i.h>
#include <kiway_player.h>
#include "pcb_edit_frame.h"
#include "io_mgr.h"
#include "class_board.h"
//-----<KIFACE>-----------------------------------------------------------------

namespace KICAD2VRML {

static struct IFACE : public KIFACE_I
{
    bool OnKifaceStart( PGM_BASE* aProgram, int aCtlBits ) override;

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

static PGM_BASE* process;

/*KIFACE_I& Kiface()
{
    return kiface;
}*/


// KIFACE_GETTER's actual spelling is a substitution macro found in kiway.h.
// KIFACE_GETTER will not have name mangling due to declaration in kiway.h.
/*
KIFACE* KIFACE_GETTER( int* aKIFACEversion, int aKIWAYversion, PGM_BASE* aProgram )
{
    process = (PGM_BASE*) aProgram;
    return &kiface;
}
*/

#if defined(BUILD_KIWAY_DLLS)
/*
PGM_BASE& Pgm()
{
    wxASSERT( process );    // KIFACE_GETTER has already been called.
    return *process;
}
*/
#endif


bool IFACE::OnKifaceStart( PGM_BASE* aProgram, int aCtlBits )
{
    return start_common( aCtlBits );
}

// implement a PGM_BASE and a wxApp side by side:

/**
 * Struct PGM_SINGLE_TOP
 * implements PGM_BASE with its own OnPgmInit() and OnPgmExit().
 */
static struct PGM_KICAD2VRML_TOP : public PGM_BASE
{
    bool OnPgmInit();

    void OnPgmExit()
    {
        //Kiway.OnKiwayEnd();

        SaveCommonSettings();

        // Destroy everything in PGM_BASE, especially wxSingleInstanceCheckerImpl
        // earlier than wxApp and earlier than static destruction would.
        PGM_BASE::Destroy();
    }

    int OnRun();

    bool ExportVRML_File( const wxString& aFullFileName, double aMMtoWRMLunit,
	                  bool aExport3DFiles, bool aUseRelativePaths,
	                  bool aUsePlainPCB, const wxString& a3D_Subdir,
	                  double aXRef, double aYRef );

    void MacOpenFile( const wxString& aFileName )   override
    {
    }

} program;


/*
PGM_BASE& Pgm()
{
    return program;
}
*/


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

#if !defined(BUILD_KIWAY_DLL)

    // Only bitmap2component and pcb_calculator use this code currently, as they
    // are not split to use single_top as a link image separate from a *.kiface.
    // i.e. they are single part link images so don't need to load a *.kiface.

    // Get the getter, it is statically linked into this binary image.
    KIFACE_GETTER_FUNC* getter = &KIFACE_GETTER;

    int  kiface_version;

    // Get the KIFACE.
    KIFACE* kiface = getter( &kiface_version, KIFACE_VERSION, this );

    // Trick the KIWAY into thinking it loaded a KIFACE, by recording the KIFACE
    // in the KIWAY.  It needs to be there for KIWAY::OnKiwayEnd() anyways.
    //Kiway.set_kiface( KIWAY::KifaceType( FRAME_KICAD2VRML ), kiface );
#endif
    return true;
}

int PGM_KICAD2VRML_TOP::OnRun()
{
    int argc = App().argc;

    int args_offset = 1;


    if( argc != 3 )
    {
	fprintf(stderr, "pcb_file vrml_file\n");
	return 1;
    }
    PROJECT         project;
    BOARD* brd = IO_MGR::Load( IO_MGR::KICAD_SEXP, App().argv[1] );
    bool ret = false;
    if( brd )
    {
        brd->BuildConnectivity();
        brd->BuildListOfNets();
        brd->SynchronizeNetsAndNetClasses();
        ret = PCB_EDIT_FRAME::ExportVRML_File(brd, project.Get3DCacheManager(), App().argv[2], 1.0, false, false, true, "", 0, 0);
        fprintf(stdout, "CACA\n");
	delete brd;
    }

    return ret?0:-1;
}

/**
 * Struct APP_KICAD2VRML
 * implements a bare naked wxApp (so that we don't become dependent on
 * functionality in a wxApp derivative that we cannot deliver under wxPython).
 */
struct APP_KICAD2VRML : public wxAppConsole
{
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

    bool OnInit() override
    {
        try
        {
            return program.OnPgmInit();
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

        program.OnPgmExit();

        return false;
    }

    int  OnExit() override
    {
        // Fixes segfault when wxPython scripting is enabled.
#if defined( KICAD_SCRIPTING_WXPYTHON )
        program.OnPgmExit();
#endif
        return wxAppConsole::OnExit();
    }

    int OnRun() override
    {
        int ret = -1;

        try
        {
	    ret = program.OnRun();
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
