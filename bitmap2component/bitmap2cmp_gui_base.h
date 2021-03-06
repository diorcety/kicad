///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.9.0 Nov  1 2020)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include "kiway_player.h"
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class BM2CMP_FRAME_BASE
///////////////////////////////////////////////////////////////////////////////
class BM2CMP_FRAME_BASE : public KIWAY_PLAYER
{
	private:

	protected:
		wxNotebook* m_Notebook;
		wxScrolledWindow* m_InitialPicturePanel;
		wxScrolledWindow* m_GreyscalePicturePanel;
		wxScrolledWindow* m_BNPicturePanel;
		wxPanel* m_panelRight;
		wxStaticText* m_staticTextSize;
		wxStaticText* m_SizeXValue;
		wxStaticText* m_SizeYValue;
		wxStaticText* m_SizePixUnits;
		wxStaticText* m_staticTextSize1;
		wxStaticText* m_SizeXValue_mm;
		wxStaticText* m_SizeYValue_mm;
		wxStaticText* m_Size_mmxUnits;
		wxStaticText* m_staticTextBPP;
		wxStaticText* m_BPPValue;
		wxStaticText* m_BPPunits;
		wxStaticText* m_staticTextBPI;
		wxTextCtrl* m_DPIValueX;
		wxTextCtrl* m_DPIValueY;
		wxStaticText* m_DPI_Units;
		wxButton* m_buttonLoad;
		wxButton* m_buttonExport;
		wxRadioBox* m_radioBoxFormat;
		wxStaticText* m_ThresholdText;
		wxSlider* m_sliderThreshold;
		wxCheckBox* m_checkNegative;
		wxRadioBox* m_radio_PCBLayer;
		wxStatusBar* m_statusBar;

		// Virtual event handlers, overide them in your derived class
		virtual void OnPaintInit( wxPaintEvent& event ) { event.Skip(); }
		virtual void OnPaintGreyscale( wxPaintEvent& event ) { event.Skip(); }
		virtual void OnPaintBW( wxPaintEvent& event ) { event.Skip(); }
		virtual void UpdatePPITextValueX( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnResolutionChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void UpdatePPITextValueY( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnLoadFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExport( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFormatChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnThresholdChange( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnNegativeClicked( wxCommandEvent& event ) { event.Skip(); }


	public:

		BM2CMP_FRAME_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Bitmap to Component Converter"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );

		~BM2CMP_FRAME_BASE();

};

