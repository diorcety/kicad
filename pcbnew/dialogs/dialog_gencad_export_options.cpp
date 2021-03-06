/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
 * Copyright (C) 2018 KiCad Developers, see change_log.txt for contributors.
 *
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#include "dialog_gencad_export_options.h"

#include <pcb_edit_frame.h>
#include <class_board.h>
#include <project.h>
#include <confirm.h>
#include <wildcards_and_files_ext.h>

#include <wx/statline.h>
#include <wx/button.h>

DIALOG_GENCAD_EXPORT_OPTIONS::DIALOG_GENCAD_EXPORT_OPTIONS( PCB_EDIT_FRAME* aParent )
    : DIALOG_SHIM( aParent, wxID_ANY, _( "Export to GenCAD settings" ), wxDefaultPosition,
                   wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
    // Obtain a potential filename for the exported file
    wxFileName fn = aParent->GetBoard()->GetFileName();
    fn.SetExt( "cad" );

    // Create widgets
    SetSizeHints( wxSize( 500, 200 ), wxDefaultSize );

    wxBoxSizer* m_mainSizer= new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* m_fileSizer = new wxBoxSizer( wxHORIZONTAL );

    m_filePath = new wxTextCtrl( this, wxID_ANY );
    m_fileSizer->Add( m_filePath, 1, wxEXPAND | wxRIGHT, 5 );

    wxButton* m_browseBtn = new wxButton( this, wxID_ANY, _( "Browse" ) );
    m_browseBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( DIALOG_GENCAD_EXPORT_OPTIONS::onBrowse ), NULL, this );
    m_fileSizer->Add( m_browseBtn, 0 );

    m_mainSizer->Add( m_fileSizer, 0, wxEXPAND | wxALL, 5 );


    m_optsSizer = new wxGridSizer( 0, 1, 3, 3 );
    createOptCheckboxes();
    m_mainSizer->Add( m_optsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5 );


    wxSizer* stdButtons = CreateSeparatedButtonSizer( wxOK | wxCANCEL );
    m_mainSizer->Add( stdButtons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    SetSizer( m_mainSizer );
    Layout();
    m_mainSizer->Fit( this );

    // Now the widgets sizes are fixed, we can initialize the m_filePath value.
    // It will be correctly displayed
    m_filePath->SetValue( fn.GetFullPath() );

    Centre( wxBOTH );
}


DIALOG_GENCAD_EXPORT_OPTIONS::~DIALOG_GENCAD_EXPORT_OPTIONS()
{
}


bool DIALOG_GENCAD_EXPORT_OPTIONS::GetOption( GENCAD_EXPORT_OPT aOption ) const
{
    auto it = m_options.find( aOption );

    if( it == m_options.end() )
    {
        wxASSERT_MSG( false, "Missing checkbox for an option" );
        return false;
    }

    return it->second->IsChecked();
}


std::map<GENCAD_EXPORT_OPT, bool> DIALOG_GENCAD_EXPORT_OPTIONS::GetAllOptions() const
{
    std::map<GENCAD_EXPORT_OPT, bool> retVal;

    for( const auto& option : m_options )
        retVal[option.first] = option.second->IsChecked();

    return retVal;
}


wxString DIALOG_GENCAD_EXPORT_OPTIONS::GetFileName() const
{
    return m_filePath->GetValue();
}


bool DIALOG_GENCAD_EXPORT_OPTIONS::TransferDataFromWindow()
{
    if( !wxDialog::TransferDataFromWindow() )
        return false;

    wxString fn = GetFileName();

    if( wxFile::Exists( fn ) )
    {
        wxString msg = wxString::Format( _( "File %s already exists." ), fn );
        KIDIALOG dlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        dlg.SetOKLabel( _( "Overwrite" ) );
        dlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        return ( dlg.ShowModal() == wxID_OK );
    }

    return true;
}


void DIALOG_GENCAD_EXPORT_OPTIONS::createOptCheckboxes()
{
    std::map<GENCAD_EXPORT_OPT, wxString> opts =
    {
        { FLIP_BOTTOM_PADS,         _( "Flip bottom footprint padstacks" ) },
        { UNIQUE_PIN_NAMES,         _( "Generate unique pin names" ) },
        { INDIVIDUAL_SHAPES,        _( "Generate a new shape for each footprint instance (do not reuse shapes)" ) },
        { USE_AUX_ORIGIN,           _( "Use auxiliary axis as origin" ) },
        { STORE_ORIGIN_COORDS,      _( "Save the origin coordinates in the file" ) }
    };

    for( const auto& option : opts )
    {
        wxCheckBox* chkbox = new wxCheckBox( this, wxID_ANY, option.second );
        m_options[option.first] = chkbox;
        m_optsSizer->Add( chkbox );
    }
}


void DIALOG_GENCAD_EXPORT_OPTIONS::onBrowse( wxCommandEvent& aEvent )
{
    wxFileDialog dlg( this, _( "Save GenCAD Board File" ),
                      wxPathOnly( Prj().GetProjectFullName() ),
                      m_filePath->GetValue(),
                      GencadFileWildcard(),
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    m_filePath->SetValue( dlg.GetPath() );
}
