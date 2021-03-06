/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2013 Jean-Pierre Charras <jp.charras at wanadoo.fr>.
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
 *
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
 * @file page_layout_graphic_items.cpp
 * @brief description of graphic items and texts to build a title block
 */

/*
 * the class WORKSHEET_DATAITEM (and WORKSHEET_DATAITEM_TEXT) defines
 * a basic shape of a page layout ( frame references and title block )
 * Basic shapes are line, rect and texts
 * the WORKSHEET_DATAITEM coordinates units is the mm, and are relative to
 * one of 4 page corners.
 *
 * These items cannot be drawn or plot "as this". they should be converted
 * to a "draw list" (WS_DRAW_ITEM_BASE and derived items)

 * The list of these items is stored in a WORKSHEET_LAYOUT instance.
 *
 * When building the draw list:
 * the WORKSHEET_LAYOUT is used to create a WS_DRAW_ITEM_LIST
 *  coordinates are converted to draw/plot coordinates.
 *  texts are expanded if they contain format symbols.
 *  Items with m_RepeatCount > 1 are created m_RepeatCount times
 *
 * the WORKSHEET_LAYOUT is created only once.
 * the WS_DRAW_ITEM_LIST is created each time the page layout is plotted/drawn
 *
 * the WORKSHEET_LAYOUT instance is created from a S expression which
 * describes the page layout (can be the default page layout or a custom file).
 */

#include <fctsys.h>
#include <eda_rect.h>
#include <draw_graphic_text.h>
#include <worksheet.h>
#include <title_block.h>
#include <worksheet_shape_builder.h>
#include <worksheet_dataitem.h>
#include <polygon_test_point_inside.h>

/* a helper function to calculate a marker size scaling factor from zoom level
 * when one need a "constant" size to draw an item whatever the zoom level
 * the scaling factor is clamped between 1.0 to 10.0 to avoid ugly drawings
 * when the factor is ti high (1.0 is the "actual" item size)
 * Note:
 *  Only the page layout editor uses the marker size.
 *  Other editors do not use or draw markers
 */
static double getScaleFromZoom( wxDC* aDC )
{
    double x, y;
    aDC->GetUserScale( &x, &y );

    double scale = (x + y ) / 2;   // should be equal, but if not best we can do is average

    double fscale = WORKSHEET_DATAITEM::m_WSunits2Iu * scale;
    double zscale = 20.0/ fscale;   // The 20.0 factor is chosen for best results
                                    // (fix the zoom level to have a zscale > 1)

    // clamp scaling factor:
    zscale = std::max( 1.0, zscale );   // never smaller than actual size
    zscale = std::min( 10.0, zscale );  // should be enough to make item visible

    return zscale;
}


/* a helper function to draw graphic symbols at start point or end point of
 * an item.
 * The start point symbol is a filled rectangle
 * The start point symbol is a filled circle
 */
inline void drawMarker( EDA_RECT* aClipBox, wxDC* aDC,
                        const wxPoint& aPos, int aSize, bool aEndPointShape = false )
{
    int markerHalfSize = aSize/2;

    if( aEndPointShape )
        GRFilledCircle( aClipBox, aDC, aPos.x, aPos.y, markerHalfSize,
                        0, GREEN, GREEN );
    else
        GRFilledRect( aClipBox, aDC,
                aPos.x - markerHalfSize, aPos.y - markerHalfSize,
                aPos.x + markerHalfSize, aPos.y + markerHalfSize,
                0, GREEN, GREEN );
}


/* Draws the item list created by BuildWorkSheetGraphicList
 * aClipBox = the clipping rect, or NULL if no clipping
 * aDC = the current Device Context
 * The not selected items are drawn first (most of items)
 * The selected items are drawn after (usually 0 or 1)
 * to be sure they are seen, even for overlapping items
 */
void WS_DRAW_ITEM_LIST::Draw( EDA_RECT* aClipBox, wxDC* aDC )
{

    // Draw the bitmaps in the background
    for( WS_DRAW_ITEM_BASE* item = GetFirst(); item; item = GetNext() )
    {
        if( item->GetParent() && item->GetParent()->IsSelected() )
            continue;

        if( item->GetType() == WS_DRAW_ITEM_BASE::wsg_bitmap )
            item->DrawWsItem( aClipBox, aDC );

    }
    // The not selected items are drawn first (most of items)
    for( WS_DRAW_ITEM_BASE* item = GetFirst(); item; item = GetNext() )
    {
        if( item->GetParent() && item->GetParent()->IsSelected() )
            continue;

        if( item->GetType() != WS_DRAW_ITEM_BASE::wsg_bitmap )
            item->DrawWsItem( aClipBox, aDC );
    }

    // The selected items are drawn after (usually 0 or 1)
    int markerSize = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    for( WS_DRAW_ITEM_BASE* item = GetFirst(); item; item = GetNext() )
    {
        if( !item->GetParent() || !item->GetParent()->IsSelected() )
            continue;

        item->DrawWsItem( aClipBox, aDC );

        if( !markerSize )
            continue;

        switch( item->GetType() )
        {
        case WS_DRAW_ITEM_BASE::wsg_line:
            {
                WS_DRAW_ITEM_LINE* line = (WS_DRAW_ITEM_LINE*) item;
                drawMarker( aClipBox, aDC, line->GetStart(), markerSize );
                drawMarker( aClipBox, aDC, line->GetEnd(), markerSize, true );
            }
            break;

        case WS_DRAW_ITEM_BASE::wsg_rect:
            {
                WS_DRAW_ITEM_RECT* rect = (WS_DRAW_ITEM_RECT*) item;
                drawMarker( aClipBox, aDC, rect->GetStart(), markerSize );
                drawMarker( aClipBox, aDC, rect->GetEnd(), markerSize, true );
            }
            break;

        case WS_DRAW_ITEM_BASE::wsg_text:
            {
                WS_DRAW_ITEM_TEXT* text = (WS_DRAW_ITEM_TEXT*) item;
                drawMarker( aClipBox, aDC, text->GetTextPos(), markerSize );
            }
            break;

        case WS_DRAW_ITEM_BASE::wsg_poly:
            {
                WS_DRAW_ITEM_POLYGON* poly = (WS_DRAW_ITEM_POLYGON*) item;
                drawMarker( aClipBox, aDC, poly->GetPosition(), markerSize );
            }
            break;

        case WS_DRAW_ITEM_BASE::wsg_bitmap:
            {
                WS_DRAW_ITEM_BITMAP* bitmap = (WS_DRAW_ITEM_BITMAP*) item;
                drawMarker( aClipBox, aDC, bitmap->GetPosition(), markerSize );
            }
            break;
        }
    }
}


WS_DRAW_ITEM_TEXT::WS_DRAW_ITEM_TEXT( WORKSHEET_DATAITEM* aParent,
                   wxString& aText, wxPoint aPos, wxSize aSize,
                   int aPenWidth, COLOR4D aColor,
                   bool aItalic, bool aBold ) :
    WS_DRAW_ITEM_BASE( aParent, wsg_text, aColor ), EDA_TEXT( aText )
{
    SetTextPos( aPos );
    SetTextSize( aSize );
    SetThickness( aPenWidth );
    SetItalic( aItalic );
    SetBold( aBold );
}


void WS_DRAW_ITEM_TEXT::DrawWsItem( EDA_RECT* aClipBox, wxDC* aDC, const wxPoint& aOffset,
       GR_DRAWMODE aDrawMode, COLOR4D aColor )
{
    Draw( aClipBox, aDC, aOffset,
            aColor == COLOR4D::UNSPECIFIED ? GetColor() : aColor,
            aDrawMode == UNSPECIFIED_DRAWMODE ? GR_COPY : aDrawMode,
            FILLED, COLOR4D::UNSPECIFIED );
}


bool WS_DRAW_ITEM_TEXT::HitTest( const wxPoint& aPosition) const
{
    return EDA_TEXT::TextHitTest( aPosition, 0 );
}


bool WS_DRAW_ITEM_TEXT::HitTest( const EDA_RECT& aRect ) const
{
    return EDA_TEXT::TextHitTest( aRect, 0, 0 );
}


bool WS_DRAW_ITEM_TEXT::HitTestStartPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint pos = GetTextPos();
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( pos.x - aPosition.x) <= marker_size / 2 &&
        std::abs( pos.y - aPosition.y) <= marker_size / 2 )
        return true;

    return false;
}


void WS_DRAW_ITEM_POLYGON::DrawWsItem( EDA_RECT* aClipBox, wxDC* aDC, const wxPoint& aOffset,
        GR_DRAWMODE aDrawMode, COLOR4D aColor )
{
    std::vector<wxPoint> points_moved;
    wxPoint *points;

    if( aOffset.x || aOffset.y )
    {
        for( auto point: m_Corners )
            points_moved.push_back( point + aOffset );
        points = &points_moved[0];
    }
    else
    {
        points = &m_Corners[0];
    }

    auto color = ( aColor == COLOR4D::UNSPECIFIED ) ? GetColor() : aColor;

    GRSetDrawMode( aDC, ( aDrawMode == UNSPECIFIED_DRAWMODE ? GR_COPY : aDrawMode ) );
    GRPoly( aClipBox, aDC,
            m_Corners.size(), points,
            IsFilled() ? FILLED_SHAPE : NO_FILL,
            GetPenWidth(),
            color, color );
    GRSetDrawMode( aDC, GR_COPY );
}


bool WS_DRAW_ITEM_POLYGON::HitTest( const wxPoint& aPosition) const
{
    return TestPointInsidePolygon( &m_Corners[0],
                                   m_Corners.size(), aPosition );
}


bool WS_DRAW_ITEM_POLYGON::HitTest( const EDA_RECT& aRect ) const
{
    // Intersection of two polygons is nontrivial. Test if the rectangle intersects
    // each line, instead.

    if( m_Corners.size() < 2 )
        return false;

    for( size_t i = 1; i < m_Corners.size(); ++i )
    {
        if( aRect.Intersects( m_Corners[i - 1], m_Corners[i] ) )
            return true;
    }

    return false;
}


bool WS_DRAW_ITEM_POLYGON::HitTestStartPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint pos = GetPosition();
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( pos.x - aPosition.x) <= marker_size / 2 &&
        std::abs( pos.y - aPosition.y) <= marker_size / 2 )
        return true;

    return false;
}


void WS_DRAW_ITEM_RECT::DrawWsItem( EDA_RECT* aClipBox, wxDC* aDC, const wxPoint& aOffset,
       GR_DRAWMODE aDrawMode, COLOR4D aColor )
{
    GRSetDrawMode( aDC, ( aDrawMode == UNSPECIFIED_DRAWMODE ? GR_COPY : aDrawMode ) );
    GRRect( aClipBox, aDC,
            GetStart().x + aOffset.x, GetStart().y + aOffset.y,
            GetEnd().x + aOffset.x, GetEnd().y + aOffset.y,
            GetPenWidth(),
            ( aColor == COLOR4D::UNSPECIFIED ) ? GetColor() : aColor );
    GRSetDrawMode( aDC, GR_COPY );
}


bool WS_DRAW_ITEM_RECT::HitTest( const wxPoint& aPosition ) const
{
    int dist =  GetPenWidth()/2;
    wxPoint start = GetStart();
    wxPoint end;
    end.x = GetEnd().x;
    end.y = start.y;

    // Upper line
    if( TestSegmentHit( aPosition, start, end, dist ) )
        return true;

    // Right line
    start = end;
    end.y = GetEnd().y;
    if( TestSegmentHit( aPosition, start, end, dist ) )
        return true;

    // lower line
    start = end;
    end.x = GetStart().x;
    if( TestSegmentHit( aPosition, start, end, dist ) )
        return true;

    // left line
    start = end;
    end = GetStart();
    if( TestSegmentHit( aPosition, start, end, dist ) )
        return true;

    return false;
}


bool WS_DRAW_ITEM_RECT::HitTest( const EDA_RECT& aRect ) const
{
    wxPoint start = GetStart();
    wxPoint end;
    end.x = GetEnd().x;
    end.y = start.y;

    // Upper line
    if( aRect.Intersects( start, end ) )
        return true;

    // Right line
    start = end;
    end.y = GetEnd().y;
    if( aRect.Intersects( start, end ) )
        return true;

    // lower line
    start = end;
    end.x = GetStart().x;
    if( aRect.Intersects( start, end ) )
        return true;

    // left line
    start = end;
    end = GetStart();
    if( aRect.Intersects( start, end ) )
        return true;

    return false;
}


bool WS_DRAW_ITEM_RECT::HitTestStartPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint dist = GetStart() - aPosition;
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( dist.x) <= marker_size / 2 &&
        std::abs( dist.y) <= marker_size / 2 )
        return true;

    return false;
}


bool WS_DRAW_ITEM_RECT::HitTestEndPoint( wxDC *aDC, const wxPoint& aPosition  )
{
    wxPoint pos = GetEnd();
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    int dist = (int) hypot( pos.x - aPosition.x, pos.y - aPosition.y );

    if( dist <= marker_size / 2 )
        return true;

    return false;
}


void WS_DRAW_ITEM_LINE::DrawWsItem( EDA_RECT* aClipBox, wxDC* aDC, const wxPoint& aOffset,
        GR_DRAWMODE aDrawMode, COLOR4D aColor )
{
    GRSetDrawMode( aDC, ( aDrawMode == UNSPECIFIED_DRAWMODE ) ? GR_COPY : aDrawMode );
    GRLine( aClipBox, aDC, GetStart() + aOffset, GetEnd() + aOffset,
            GetPenWidth(),
            ( aColor == COLOR4D::UNSPECIFIED ) ? GetColor() : aColor );
    GRSetDrawMode( aDC, GR_COPY );
}


bool WS_DRAW_ITEM_LINE::HitTest( const wxPoint& aPosition) const
{
    return TestSegmentHit( aPosition, GetStart(), GetEnd(), GetPenWidth()/2 );
}


bool WS_DRAW_ITEM_LINE::HitTest( const EDA_RECT& aRect ) const
{
    return aRect.Intersects( GetStart(), GetEnd() );
}


bool WS_DRAW_ITEM_LINE::HitTestStartPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint dist = GetStart() - aPosition;
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( dist.x) <= marker_size / 2 &&
        std::abs( dist.y) <= marker_size / 2 )
        return true;

    return false;
}


bool WS_DRAW_ITEM_LINE::HitTestEndPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint dist = GetEnd() - aPosition;
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( dist.x) <= marker_size / 2 &&
        std::abs( dist.y) <= marker_size / 2 )
        return true;

    return false;
}


void WS_DRAW_ITEM_LIST::Locate( wxDC* aDC, std::vector <WS_DRAW_ITEM_BASE*>& aList,
                                const wxPoint& aPosition )
{
    for( WS_DRAW_ITEM_BASE* item = GetFirst(); item; item = GetNext() )
    {
        item->m_Flags &= ~(LOCATE_STARTPOINT|LOCATE_ENDPOINT);
        bool found = false;

        if( item->HitTestStartPoint ( aDC, aPosition ) )
        {
            item->m_Flags |= LOCATE_STARTPOINT;
            found = true;
        }

        if( item->HitTestEndPoint ( aDC, aPosition ) )
        {
            item->m_Flags |= LOCATE_ENDPOINT;
            found = true;
        }

        if( found || item->HitTest( aPosition ) )
        {
            aList.push_back( item );
        }
    }
}


void WS_DRAW_ITEM_BITMAP::DrawWsItem( EDA_RECT* aClipBox, wxDC* aDC, const wxPoint& aOffset,
        GR_DRAWMODE aDrawMode, COLOR4D aColor )
{
    WORKSHEET_DATAITEM_BITMAP* parent = (WORKSHEET_DATAITEM_BITMAP*)GetParent();

    if( parent->m_ImageBitmap  )
    {
        GRSetDrawMode( aDC, ( aDrawMode == UNSPECIFIED_DRAWMODE ) ? GR_COPY : aDrawMode );
        parent->m_ImageBitmap->DrawBitmap( aDC, m_pos + aOffset );
    }
}


bool WS_DRAW_ITEM_BITMAP::HitTest( const wxPoint& aPosition) const
{
    const WORKSHEET_DATAITEM_BITMAP* parent = static_cast<const WORKSHEET_DATAITEM_BITMAP*>( GetParent() );

    if( parent->m_ImageBitmap == NULL )
        return false;

    EDA_RECT rect = parent->m_ImageBitmap->GetBoundingBox();
    rect.Move( m_pos );
    return rect.Contains( aPosition );
}


bool WS_DRAW_ITEM_BITMAP::HitTest( const EDA_RECT& aRect ) const
{
    const WORKSHEET_DATAITEM_BITMAP* parent = static_cast<const WORKSHEET_DATAITEM_BITMAP*>( GetParent() );

    if( parent->m_ImageBitmap == NULL )
        return false;

    EDA_RECT rect = parent->m_ImageBitmap->GetBoundingBox();
    rect.Move( m_pos );
    return rect.Intersects( aRect );
}


/**
 * return true if the point aPosition is on the reference point of this item.
 */
bool WS_DRAW_ITEM_BITMAP::HitTestStartPoint( wxDC *aDC, const wxPoint& aPosition )
{
    wxPoint dist = m_pos - aPosition;
    int marker_size = WORKSHEET_DATAITEM::GetMarkerSizeUi( getScaleFromZoom( aDC ) );

    if( std::abs( dist.x) <= marker_size / 2 &&
        std::abs( dist.y) <= marker_size / 2 )
        return true;

    return false;
}

