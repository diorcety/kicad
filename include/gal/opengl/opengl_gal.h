/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2012 Torsten Hueter, torstenhtr <at> gmx.de
 * Copyright (C) 2020 Kicad Developers, see AUTHORS.txt for contributors.
 * Copyright (C) 2013-2017 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * Graphics Abstraction Layer (GAL) for OpenGL
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

#ifndef OPENGLGAL_H_
#define OPENGLGAL_H_

// GAL imports
#include <gal/graphics_abstraction_layer.h>
#include <gal/gal_display_options.h>
#include <gal/opengl/shader.h>
#include <gal/opengl/vertex_manager.h>
#include <gal/opengl/vertex_item.h>
#include <gal/opengl/cached_container.h>
#include <gal/opengl/noncached_container.h>
#include <gal/opengl/opengl_compositor.h>
#include <gal/hidpi_gl_canvas.h>

#include <unordered_map>
#include <boost/smart_ptr/shared_array.hpp>
#include <memory>

#ifndef CALLBACK
#define CALLBACK
#endif

struct bitmap_glyph;

namespace KIGFX
{
class SHADER;
class GL_BITMAP_CACHE;

/**
 * @brief Class OpenGL_GAL is the OpenGL implementation of the Graphics Abstraction Layer.
 *
 * This is a direct OpenGL-implementation and uses low-level graphics primitives like triangles
 * and quads. The purpose is to provide a fast graphics interface, that takes advantage of modern
 * graphics card GPUs. All methods here benefit thus from the hardware acceleration.
 */
class OPENGL_GAL : public GAL, public HIDPI_GL_CANVAS
{
public:
    /**
     * @brief Constructor OPENGL_GAL
     *
     * @param aParent is the wxWidgets immediate wxWindow parent of this object.
     *
     * @param aMouseListener is the wxEvtHandler that should receive the mouse events,
     *  this can be can be any wxWindow, but is often a wxFrame container.
     *
     * @param aPaintListener is the wxEvtHandler that should receive the paint
     *  event.  This can be any wxWindow, but is often a derived instance
     *  of this class or a containing wxFrame.  The "paint event" here is
     *  a wxCommandEvent holding EVT_GAL_REDRAW, as sent by PostPaint().
     *
     * @param aName is the name of this window for use by wxWindow::FindWindowByName()
     */
    OPENGL_GAL( GAL_DISPLAY_OPTIONS& aDisplayOptions, wxWindow* aParent,
                wxEvtHandler* aMouseListener = nullptr, wxEvtHandler* aPaintListener = nullptr,
                const wxString& aName = wxT( "GLCanvas" ) );

    virtual ~OPENGL_GAL();

    /**
     * @brief Checks OpenGL features
     * @param aOptions 
     * @return wxEmptyString if OpenGL 2.1 or greater is available, otherwise returns error message
     */
    static wxString CheckFeatures( GAL_DISPLAY_OPTIONS& aOptions );

    virtual bool IsOpenGlEngine() override { return true; }

    /// @copydoc GAL::IsInitialized()
    virtual bool IsInitialized() const override
    {
        // is*Initialized flags, but it is enough for OpenGL to show up
        return IsShownOnScreen() && !GetClientRect().IsEmpty();
    }

    ///> @copydoc GAL::IsVisible()
    bool IsVisible() const override
    {
        return IsShownOnScreen() && !GetClientRect().IsEmpty();
    }

    // ---------------
    // Drawing methods
    // ---------------

    /// @copydoc GAL::DrawLine()
    virtual void DrawLine( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint ) override;

    /// @copydoc GAL::DrawSegment()
    virtual void DrawSegment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint,
                              double aWidth ) override;

    /// @copydoc GAL::DrawCircle()
    virtual void DrawCircle( const VECTOR2D& aCenterPoint, double aRadius ) override;

    /// @copydoc GAL::DrawArc()
    virtual void DrawArc( const VECTOR2D& aCenterPoint, double aRadius,
                          double aStartAngle, double aEndAngle ) override;

    /// @copydoc GAL::DrawArcSegment()
    virtual void DrawArcSegment( const VECTOR2D& aCenterPoint, double aRadius,
                                 double aStartAngle, double aEndAngle, double aWidth ) override;

    /// @copydoc GAL::DrawRectangle()
    virtual void DrawRectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint ) override;

    /// @copydoc GAL::DrawPolyline()
    virtual void DrawPolyline( const std::deque<VECTOR2D>& aPointList ) override;
    virtual void DrawPolyline( const VECTOR2D aPointList[], int aListSize ) override;
    virtual void DrawPolyline( const SHAPE_LINE_CHAIN& aLineChain ) override;

    /// @copydoc GAL::DrawPolygon()
    virtual void DrawPolygon( const std::deque<VECTOR2D>& aPointList ) override;
    virtual void DrawPolygon( const VECTOR2D aPointList[], int aListSize ) override;
    virtual void DrawPolygon( const SHAPE_POLY_SET& aPolySet ) override;
    virtual void DrawPolygon( const SHAPE_LINE_CHAIN& aPolySet ) override;

    /// @copydoc GAL::DrawCurve()
    virtual void DrawCurve( const VECTOR2D& startPoint, const VECTOR2D& controlPointA,
                            const VECTOR2D& controlPointB, const VECTOR2D& endPoint,
                            double aFilterValue = 0.0 ) override;

    /// @copydoc GAL::DrawBitmap()
    virtual void DrawBitmap( const BITMAP_BASE& aBitmap ) override;

    /// @copydoc GAL::BitmapText()
    virtual void BitmapText( const wxString& aText, const VECTOR2D& aPosition,
                             double aRotationAngle ) override;

    /// @copydoc GAL::DrawGrid()
    virtual void DrawGrid() override;

    // --------------
    // Screen methods
    // --------------

    /// @brief Resizes the canvas.
    virtual void ResizeScreen( int aWidth, int aHeight ) override;

    /// @brief Shows/hides the GAL canvas
    virtual bool Show( bool aShow ) override;

    /// @copydoc GAL::Flush()
    virtual void Flush() override;

    /// @copydoc GAL::ClearScreen()
    virtual void ClearScreen( ) override;

    // --------------
    // Transformation
    // --------------

    /// @copydoc GAL::Transform()
    virtual void Transform( const MATRIX3x3D& aTransformation ) override;

    /// @copydoc GAL::Rotate()
    virtual void Rotate( double aAngle ) override;

    /// @copydoc GAL::Translate()
    virtual void Translate( const VECTOR2D& aTranslation ) override;

    /// @copydoc GAL::Scale()
    virtual void Scale( const VECTOR2D& aScale ) override;

    /// @copydoc GAL::Save()
    virtual void Save() override;

    /// @copydoc GAL::Restore()
    virtual void Restore() override;

    // --------------------------------------------
    // Group methods
    // ---------------------------------------------

    /// @copydoc GAL::BeginGroup()
    virtual int BeginGroup() override;

    /// @copydoc GAL::EndGroup()
    virtual void EndGroup() override;

    /// @copydoc GAL::DrawGroup()
    virtual void DrawGroup( int aGroupNumber ) override;

    /// @copydoc GAL::ChangeGroupColor()
    virtual void ChangeGroupColor( int aGroupNumber, const COLOR4D& aNewColor ) override;

    /// @copydoc GAL::ChangeGroupDepth()
    virtual void ChangeGroupDepth( int aGroupNumber, int aDepth ) override;

    /// @copydoc GAL::DeleteGroup()
    virtual void DeleteGroup( int aGroupNumber ) override;

    /// @copydoc GAL::ClearCache()
    virtual void ClearCache() override;

    // --------------------------------------------------------
    // Handling the world <-> screen transformation
    // --------------------------------------------------------

    /// @copydoc GAL::SaveScreen()
    virtual void SaveScreen() override;

    /// @copydoc GAL::RestoreScreen()
    virtual void RestoreScreen() override;

    /// @copydoc GAL::SetTarget()
    virtual void SetTarget( RENDER_TARGET aTarget ) override;

    /// @copydoc GAL::GetTarget()
    virtual RENDER_TARGET GetTarget() const override;

    /// @copydoc GAL::ClearTarget()
    virtual void ClearTarget( RENDER_TARGET aTarget ) override;

    /// @copydoc GAL::SetNegativeDrawMode()
    virtual void SetNegativeDrawMode( bool aSetting ) override {}

    virtual void ComputeWorldScreenMatrix() override;

    // -------
    // Cursor
    // -------

    /// @copydoc GAL::DrawCursor()
    virtual void DrawCursor( const VECTOR2D& aCursorPosition ) override;

    /**
     * @brief Function PostPaint
     * posts an event to m_paint_listener.  A post is used so that the actual drawing
     * function can use a device context type that is not specific to the wxEVT_PAINT event,
     * just by changing the PostPaint code.
     */
    void PostPaint( wxPaintEvent& aEvent );

    void SetMouseListener( wxEvtHandler* aMouseListener )
    {
        mouseListener = aMouseListener;
    }

    void SetPaintListener( wxEvtHandler* aPaintListener )
    {
        paintListener = aPaintListener;
    }

    virtual void EnableDepthTest( bool aEnabled = false ) override;

    ///< Parameters passed to the GLU tesselator
    typedef struct
    {
        /// Manager used for storing new vertices
        VERTEX_MANAGER* vboManager;

        /// Intersect points, that have to be freed after tessellation
        std::deque< boost::shared_array<GLdouble> >& intersectPoints;
    } TessParams;

private:
    /// Super class definition
    typedef GAL super;

    static const int    CIRCLE_POINTS   = 64;   ///< The number of points for circle approximation
    static const int    CURVE_POINTS    = 32;   ///< The number of points for curve approximation

    static wxGLContext*     glMainContext;      ///< Parent OpenGL context
    wxGLContext*            glPrivContext;      ///< Canvas-specific OpenGL context
    static int              instanceCounter;    ///< GL GAL instance counter
    wxEvtHandler*           mouseListener;
    wxEvtHandler*           paintListener;

    static GLuint fontTexture;                  ///< Bitmap font texture handle (shared)

    // Vertex buffer objects related fields
    typedef std::unordered_map< unsigned int, std::shared_ptr<VERTEX_ITEM> > GROUPS_MAP;
    GROUPS_MAP              groups;                 ///< Stores informations about VBO objects (groups)
    unsigned int            groupCounter;           ///< Counter used for generating keys for groups
    VERTEX_MANAGER*         currentManager;         ///< Currently used VERTEX_MANAGER (for storing VERTEX_ITEMs)
    VERTEX_MANAGER*         cachedManager;          ///< Container for storing cached VERTEX_ITEMs
    VERTEX_MANAGER*         nonCachedManager;       ///< Container for storing non-cached VERTEX_ITEMs
    VERTEX_MANAGER*         overlayManager;         ///< Container for storing overlaid VERTEX_ITEMs

    // Framebuffer & compositing
    OPENGL_COMPOSITOR*      compositor;             ///< Handles multiple rendering targets
    unsigned int            mainBuffer;             ///< Main rendering target
    unsigned int            overlayBuffer;          ///< Auxiliary rendering target (for menus etc.)
    RENDER_TARGET           currentTarget;          ///< Current rendering target

    // Shader
    SHADER*                 shader;                 ///< There is only one shader used for different objects

    // Internal flags
    bool                    isFramebufferInitialized;   ///< Are the framebuffers initialized?
    static bool             isBitmapFontLoaded;         ///< Is the bitmap font texture loaded?
    bool                    isBitmapFontInitialized;    ///< Is the shader set to use bitmap fonts?
    bool                    isInitialized;              ///< Basic initialization flag, has to be done
                                                        ///< when the window is visible
    bool                    isGrouping;                 ///< Was a group started?
    bool                    isContextLocked;            ///< Used for assertion checking
    int                     lockClientCookie;
    GLint                   ufm_worldPixelSize;
    GLint                   ufm_screenPixelSize;
    GLint                   ufm_pixelSizeMultiplier;

    std::unique_ptr<GL_BITMAP_CACHE>         bitmapCache;

    void lockContext( int aClientCookie ) override;

    void unlockContext( int aClientCookie ) override;

    /// @copydoc GAL::BeginUpdate()
    virtual void beginUpdate() override;

    /// @copydoc GAL::EndUpdate()
    virtual void endUpdate() override;

    /// @copydoc GAL::BeginDrawing()
    virtual void beginDrawing() override;

    /// @copydoc GAL::EndDrawing()
    virtual void endDrawing() override;

    ///< Update handler for OpenGL settings
    bool updatedGalDisplayOptions( const GAL_DISPLAY_OPTIONS& aOptions ) override;

    // Polygon tesselation
    /// The tessellator
    GLUtesselator*          tesselator;
    /// Storage for intersecting points
    std::deque< boost::shared_array<GLdouble> > tessIntersects;

    /**
     * @brief Draw a quad for the line.
     *
     * @param aStartPoint is the start point of the line.
     * @param aEndPoint is the end point of the line.
     */
    void drawLineQuad( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint );

    /**
     * @brief Draw a semicircle. Depending on settings (isStrokeEnabled & isFilledEnabled) it runs
     * the proper function (drawStrokedSemiCircle or drawFilledSemiCircle).
     *
     * @param aCenterPoint is the center point.
     * @param aRadius is the radius of the semicircle.
     * @param aAngle is the angle of the semicircle.
     *
     */
    void drawSemiCircle( const VECTOR2D& aCenterPoint, double aRadius, double aAngle );

    /**
     * @brief Draw a filled semicircle.
     *
     * @param aCenterPoint is the center point.
     * @param aRadius is the radius of the semicircle.
     * @param aAngle is the angle of the semicircle.
     *
     */
    void drawFilledSemiCircle( const VECTOR2D& aCenterPoint, double aRadius, double aAngle );

    /**
     * @brief Draw a stroked semicircle.
     *
     * @param aCenterPoint is the center point.
     * @param aRadius is the radius of the semicircle.
     * @param aAngle is the angle of the semicircle.
     *
     */
    void drawStrokedSemiCircle( const VECTOR2D& aCenterPoint, double aRadius, double aAngle );

    /**
     * @brief Generic way of drawing a polyline stored in different containers.
     * @param aPointGetter is a function to obtain coordinates of n-th vertex.
     * @param aPointCount is the number of points to be drawn.
     */
    void drawPolyline( const std::function<VECTOR2D (int)>& aPointGetter, int aPointCount );

    /**
     * @brief Draws a filled polygon. It does not need the last point to have the same coordinates
     * as the first one.
     * @param aPoints is the vertices data (3 coordinates: x, y, z).
     * @param aPointCount is the number of points.
     */
    void drawPolygon( GLdouble* aPoints, int aPointCount );

    /**
     * @brief Draws a set of polygons with a cached triangulation. Way faster than drawPolygon.
     */
    void drawTriangulatedPolyset( const SHAPE_POLY_SET& aPoly );


    /**
     * @brief Draws a single character using bitmap font.
     * Its main purpose is to be used in BitmapText() function.
     *
     * @param aChar is the character to be drawn.
     * @return Width of the drawn glyph.
     */
    int drawBitmapChar( unsigned long aChar );

    /**
     * @brief Draws an overbar over the currently drawn text.
     * Its main purpose is to be used in BitmapText() function.
     * This method requires appropriate scaling to be applied (as is done in BitmapText() function).
     * The current X coordinate will be the overbar ending.
     *
     * @param aLength is the width of the overbar.
     * @param aHeight is the height for the overbar.
     */
    void drawBitmapOverbar( double aLength, double aHeight );

    /**
     * @brief Computes a size of text drawn using bitmap font with current text setting applied.
     *
     * @param aText is the text to be drawn.
     * @return Pair containing text bounding box and common Y axis offset. The values are expressed
     * as a number of pixels on the bitmap font texture and need to be scaled before drawing.
     */
    std::pair<VECTOR2D, float> computeBitmapTextSize( const UTF8& aText ) const;

    // Event handling
    /**
     * @brief This is the OnPaint event handler.
     *
     * @param aEvent is the OnPaint event.
     */
    void onPaint( wxPaintEvent& aEvent );

    /**
     * @brief Skip the mouse event to the parent.
     *
     * @param aEvent is the mouse event.
     */
    void skipMouseEvent( wxMouseEvent& aEvent );

    /**
     * @brief Blits cursor into the current screen.
     */
    void blitCursor();

    /**
     * @brief Returns a valid key that can be used as a new group number.
     *
     * @return An unique group number that is not used by any other group.
     */
    unsigned int getNewGroupNumber();

    /**
     * @brief Compute the angle step when drawing arcs/circles approximated with lines.
     */
    double calcAngleStep( double aRadius ) const
    {
        // Bigger arcs need smaller alpha increment to make them look smooth
        return std::min( 1e6 / aRadius, 2.0 * M_PI / CIRCLE_POINTS );
    }

    double getWorldPixelSize() const;

    VECTOR2D getScreenPixelSize() const;


    /**
     * @brief Basic OpenGL initialization.
     */
    void init();
};
} // namespace KIGFX

#endif  // OPENGLGAL_H_
