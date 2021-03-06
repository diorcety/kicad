/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file kicad.cpp
 * @brief Main KiCad Project manager file
 */


#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include <common.h>
#include <hotkeys_basic.h>
#include <kiway.h>
#include <richio.h>
#include <wildcards_and_files_ext.h>
#include <systemdirsappend.h>

#include <stdexcept>

#include "pgm_kicad.h"

#include "kicad.h"


// a dummy to quiet linking with EDA_BASE_FRAME::config();
#include <kiface_i.h>
KIFACE_I& Kiface()
{
    // This function should never be called.  It is only referenced from
    // EDA_BASE_FRAME::config() and this is only provided to satisfy the linker,
    // not to be actually called.
    wxLogFatalError( wxT( "Unexpected call to Kiface() in kicad/kicad.cpp" ) );

    throw std::logic_error( "Unexpected call to Kiface() in kicad/kicad.cpp" );
}


static PGM_KICAD program;


PGM_BASE& Pgm()
{
    return program;
}


PGM_KICAD& PgmTop()
{
    return program;
}


bool PGM_KICAD::OnPgmInit()
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

    m_bm.Init();

    // Add search paths to feed the PGM_KICAD::SysSearch() function,
    // currenly limited in support to only look for project templates
    {
        SEARCH_STACK bases;

        SystemDirsAppend( &bases );

        for( unsigned i = 0; i < bases.GetCount(); ++i )
        {
            wxFileName fn( bases[i], wxEmptyString );

            // Add KiCad template file path to search path list.
            fn.AppendDir( wxT( "template" ) );

            // Only add path if exists and can be read by the user.
            if( fn.DirExists() && fn.IsDirReadable() )
                m_bm.m_search.AddPaths( fn.GetPath() );
        }

        // The KICAD_TEMPLATE_DIR takes precedence over the search stack template path.
        ENV_VAR_MAP_CITER it = GetLocalEnvVariables().find( "KICAD_TEMPLATE_DIR" );

        if( it != GetLocalEnvVariables().end() && it->second.GetValue() != wxEmptyString )
            m_bm.m_search.Insert( it->second.GetValue(), 0 );

        // The KICAD_USER_TEMPLATE_DIR takes precedence over KICAD_TEMPLATE_DIR and the search
        // stack template path.
        it = GetLocalEnvVariables().find( "KICAD_USER_TEMPLATE_DIR" );

        if( it != GetLocalEnvVariables().end() && it->second.GetValue() != wxEmptyString )
            m_bm.m_search.Insert( it->second.GetValue(), 0 );
    }

    // Must be called before creating the main frame in order to
    // display the real hotkeys in menus or tool tips
    extern struct EDA_HOTKEY_CONFIG kicad_Manager_Hotkeys_Descr[];
    ReadHotkeyConfig( KICAD_MANAGER_FRAME_NAME, kicad_Manager_Hotkeys_Descr );

    KICAD_MANAGER_FRAME* frame = new KICAD_MANAGER_FRAME( NULL, wxT( "KiCad" ),
                                                          wxDefaultPosition, wxSize( 775, -1 ) );
    App().SetTopWindow( frame );

    Kiway.SetTop( frame );

    wxString projToLoad;

    if( App().argc > 1 )
        projToLoad = App().argv[1];

    if( projToLoad.IsEmpty() && GetFileHistory().GetCount() )
    {
        wxString last_pro = GetFileHistory().GetHistoryFile( 0 );

        if( !wxFileExists( last_pro ) )
        {
            GetFileHistory().RemoveFileFromHistory( 0 );
        }
        else
        {
            // Try to open the last opened project,
            // if a project name is not given when starting Kicad
            projToLoad = last_pro;
        }
    }

    // Do not attempt to load a non-existent project file.
    if( !projToLoad.empty() )
    {
        wxFileName fn( projToLoad );
        if( fn.Exists() )
        {
            fn.MakeAbsolute();
            frame->LoadProject( fn );
        }
    }

    frame->Show( true );
    frame->Raise();

    return true;
}


void PGM_KICAD::OnPgmExit()
{
    Kiway.OnKiwayEnd();

    SaveCommonSettings();

    // write common settings to disk, and destroy everything in PGM_KICAD,
    // especially wxSingleInstanceCheckerImpl earlier than wxApp and earlier
    // than static destruction would.
    Destroy();
}


void PGM_KICAD::MacOpenFile( const wxString& aFileName )
{
#if defined(__WXMAC__)

    KICAD_MANAGER_FRAME* frame = (KICAD_MANAGER_FRAME*) App().GetTopWindow();

    if( !aFileName.empty() && wxFileExists( aFileName ) )
        frame->LoadProject( wxFileName( aFileName ) );

#endif
}


void PGM_KICAD::Destroy()
{
    // unlike a normal destructor, this is designed to be called more
    // than once safely:

    m_bm.End();

    PGM_BASE::Destroy();
}


KIWAY  Kiway( &Pgm(), KFCTL_CPP_PROJECT_SUITE );


/**
 * Struct APP_KICAD
 * is not publicly visible because most of the action is in PGM_KICAD these days.
 */
struct APP_KICAD : public wxApp
{
#if defined (__LINUX__)
    APP_KICAD(): wxApp()
    {
        // Disable proxy menu in Unity window manager. Only usual menubar works with
        // wxWidgets (at least <= 3.1).  When the proxy menu menubar is enable, some
        // important things for us do not work: menuitems UI events and shortcuts.
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

    bool OnInit()           override
    {
        return program.OnPgmInit();
    }

    int  OnExit()           override
    {
        program.OnPgmExit();

#if defined(__FreeBSD__)
        /* Avoid wxLog crashing when used in destructors. */
        wxLog::EnableLogging( false );
#endif

        return wxApp::OnExit();
    }

    int OnRun()             override
    {
        try
        {
            return wxApp::OnRun();
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

        return -1;
    }

    /**
     * Set MacOS file associations.
     *
     * @see http://wiki.wxwidgets.org/WxMac-specific_topics
     */
    void MacOpenFile( const wxString& aFileName )
    {
        Pgm().MacOpenFile( aFileName );
    }
};

IMPLEMENT_APP( APP_KICAD )


// The C++ project manager supports one open PROJECT, so Prj() calls within
// this link image need this function.
PROJECT& Prj()
{
    return Kiway.Prj();
}

