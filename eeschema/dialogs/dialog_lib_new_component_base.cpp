///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Nov  1 2020)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_lib_new_component_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_LIB_NEW_COMPONENT_BASE::DIALOG_LIB_NEW_COMPONENT_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizerTop;
	bSizerTop = new wxBoxSizer( wxVERTICAL );

	m_staticTextGeneralSettings = new wxStaticText( this, wxID_ANY, _("General Settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextGeneralSettings->Wrap( -1 );
	m_staticTextGeneralSettings->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	bSizerTop->Add( m_staticTextGeneralSettings, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer31->AddGrowableCol( 1 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticTextName = new wxStaticText( this, wxID_ANY, _("Symbol name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextName->Wrap( -1 );
	m_staticTextName->SetToolTip( _("This is the symbol name in library,\nand also the default component value when loaded in the schematic.") );

	fgSizer31->Add( m_staticTextName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );

	m_textName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	fgSizer31->Add( m_textName, 1, wxALL|wxEXPAND, 3 );

	m_staticTextDes = new wxStaticText( this, wxID_ANY, _("Default reference designator:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextDes->Wrap( -1 );
	fgSizer31->Add( m_staticTextDes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_textReference = new wxTextCtrl( this, wxID_ANY, _("U"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer31->Add( m_textReference, 0, wxALL|wxEXPAND, 3 );

	m_staticTextUnits = new wxStaticText( this, wxID_ANY, _("Number of units per package:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextUnits->Wrap( -1 );
	fgSizer31->Add( m_staticTextUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_spinPartCount = new wxSpinCtrl( this, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 64, 0 );
	fgSizer31->Add( m_spinPartCount, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );


	bSizerTop->Add( fgSizer31, 1, wxEXPAND|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );

	m_checkHasConversion = new wxCheckBox( this, wxID_ANY, _("Create symbol with alternate body style (DeMorgan)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( m_checkHasConversion, 0, wxALL, 5 );

	m_checkIsPowerSymbol = new wxCheckBox( this, wxID_ANY, _("Create symbol as power symbol"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( m_checkIsPowerSymbol, 0, wxALL, 5 );

	m_checkLockItems = new wxCheckBox( this, wxID_ANY, _("Units are not interchangeable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkLockItems->SetToolTip( _("Check this option to allow symbols with multiple units to have different\nelements.  Uncheck this option when all symbol units are identical except\nfor pin numbers.") );

	bSizer17->Add( m_checkLockItems, 0, wxALL, 5 );


	bSizerTop->Add( bSizer17, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );


	bSizerMain->Add( bSizerTop, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizerBottom;
	bSizerBottom = new wxBoxSizer( wxVERTICAL );

	m_staticTextPinSettings = new wxStaticText( this, wxID_ANY, _("Pin Settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextPinSettings->Wrap( -1 );
	m_staticTextPinSettings->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	bSizerBottom->Add( m_staticTextPinSettings, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 2, 0, 55 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText12 = new wxStaticText( this, wxID_ANY, _("Pin text position offset:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer4->Add( m_staticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_spinPinTextPosition = new wxSpinCtrl( this, wxID_ANY, wxT("40"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, 40 );
	fgSizer4->Add( m_spinPinTextPosition, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );


	bSizerBottom->Add( fgSizer4, 0, wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );

	m_checkShowPinNumber = new wxCheckBox( this, wxID_ANY, _("Show pin number text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkShowPinNumber->SetValue(true);
	bSizer19->Add( m_checkShowPinNumber, 0, wxALL, 5 );

	m_checkShowPinName = new wxCheckBox( this, wxID_ANY, _("Show pin name text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkShowPinName->SetValue(true);
	bSizer19->Add( m_checkShowPinName, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_checkShowPinNameInside = new wxCheckBox( this, wxID_ANY, _("Pin name inside"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkShowPinNameInside->SetValue(true);
	bSizer19->Add( m_checkShowPinNameInside, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );


	bSizerBottom->Add( bSizer19, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );


	bSizerBottom->Add( 0, 10, 0, wxEXPAND, 5 );


	bSizerMain->Add( bSizerBottom, 1, wxALL|wxEXPAND, 5 );

	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton( this, wxID_OK );
	m_sdbSizer->AddButton( m_sdbSizerOK );
	m_sdbSizerCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer->AddButton( m_sdbSizerCancel );
	m_sdbSizer->Realize();

	bSizerMain->Add( m_sdbSizer, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizerMain );
	this->Layout();
	bSizerMain->Fit( this );

	this->Centre( wxBOTH );
}

DIALOG_LIB_NEW_COMPONENT_BASE::~DIALOG_LIB_NEW_COMPONENT_BASE()
{
}
