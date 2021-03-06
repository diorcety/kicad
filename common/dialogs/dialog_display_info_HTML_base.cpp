///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug 15 2019)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_display_info_HTML_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_DISPLAY_HTML_TEXT_BASE::DIALOG_DISPLAY_HTML_TEXT_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );

	m_htmlWindow = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_htmlWindow->SetMinSize( wxSize( 400,250 ) );

	bMainSizer->Add( m_htmlWindow, 1, wxALL|wxEXPAND, 5 );

	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1->Realize();

	bMainSizer->Add( m_sdbSizer1, 0, wxBOTTOM|wxEXPAND, 5 );


	this->SetSizer( bMainSizer );
	this->Layout();
	bMainSizer->Fit( this );

	// Connect Events
	m_htmlWindow->Connect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( DIALOG_DISPLAY_HTML_TEXT_BASE::OnHTMLLinkClicked ), NULL, this );
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_DISPLAY_HTML_TEXT_BASE::OnOKButtonClick ), NULL, this );
}

DIALOG_DISPLAY_HTML_TEXT_BASE::~DIALOG_DISPLAY_HTML_TEXT_BASE()
{
	// Disconnect Events
	m_htmlWindow->Disconnect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( DIALOG_DISPLAY_HTML_TEXT_BASE::OnHTMLLinkClicked ), NULL, this );
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_DISPLAY_HTML_TEXT_BASE::OnOKButtonClick ), NULL, this );

}
