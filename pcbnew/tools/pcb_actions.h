/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2016 CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef __PCB_ACTIONS_H
#define __PCB_ACTIONS_H

#include <tool/tool_action.h>
#include <tool/actions.h>
#include <core/optional.h>

class TOOL_EVENT;
class TOOL_MANAGER;

/**
 * Class PCB_ACTIONS
 *
 * Gathers all the actions that are shared by tools. The instance of PCB_ACTIONS is created
 * inside of ACTION_MANAGER object that registers the actions.
 */
class PCB_ACTIONS : public ACTIONS
{
public:
    // Selection Tool
    /// Activation of the selection tool
    static TOOL_ACTION selectionActivate;

    /// Select a single item under the cursor position
    static TOOL_ACTION selectionCursor;

    /// Clears the current selection
    static TOOL_ACTION selectionClear;

    /// Selects an item (specified as the event parameter).
    static TOOL_ACTION selectItem;

    /// Selects a list of items (specified as the event parameter)
    static TOOL_ACTION selectItems;

    /// Unselects an item (specified as the event parameter).
    static TOOL_ACTION unselectItem;

    /// Unselects a list of items (specified as the event parameter)
    static TOOL_ACTION unselectItems;

    /// Runs a selection menu to select from a list of items
    static TOOL_ACTION selectionMenu;

    /// Selects a connection between junctions.
    static TOOL_ACTION selectConnection;

    /// Expands the current selection to select a connection between two junctions
    static TOOL_ACTION expandSelectedConnection;

    /// Selects whole copper connection.
    static TOOL_ACTION selectCopper;

    /// Selects all connections belonging to a single net.
    static TOOL_ACTION selectNet;

    /// Selects all components on sheet from Eeschema crossprobing.
    static TOOL_ACTION selectOnSheetFromEeschema;

    /// Selects all components on the same sheet as the selected footprint.
    static TOOL_ACTION selectSameSheet;

    /// Filters the items in the current selection (invokes dialog)
    static TOOL_ACTION filterSelection;

    // Edit Tool
    /// Activation of the edit tool
    static TOOL_ACTION editActivate;

    /// move or drag an item
    static TOOL_ACTION move;
    static TOOL_ACTION drag;

    /// Rotation of selected objects clockwise
    static TOOL_ACTION rotateCw;

    /// Rotation of selected objects counter-clockwise
    static TOOL_ACTION rotateCcw;

    /// Flipping of selected objects
    static TOOL_ACTION flip;

    /// Mirroring of selected items
    static TOOL_ACTION mirror;

    /// Activation of the edit tool
    static TOOL_ACTION properties;

    /// Modified selection notification
    static TOOL_ACTION selectionModified;

    /// Activation of the exact move tool
    static TOOL_ACTION moveExact;

    /// Activation of the duplication tool
    static TOOL_ACTION duplicate;

    /// Activation of the duplication tool with incrementing (e.g. pad number)
    static TOOL_ACTION duplicateIncrement;

    /// Update footprints to reflect any changes in the library
    static TOOL_ACTION updateFootprints;

    /// Exchange footprints of modules
    static TOOL_ACTION exchangeFootprints;

    /// Deleting a BOARD_ITEM
    static TOOL_ACTION remove;
    static TOOL_ACTION removeAlt;

    /// Break a single track into two segments at the cursor
    static TOOL_ACTION breakTrack;

    /// Breaks track when router is not activated
    static TOOL_ACTION inlineBreakTrack;

    static TOOL_ACTION drag45Degree;
    static TOOL_ACTION dragFreeAngle;


    // Drawing Tool
    /// Activation of the drawing tool (line)
    static TOOL_ACTION drawLine;

    // Activation of the drawing tool (graphic polygons)
    static TOOL_ACTION drawGraphicPolygon;

    /// Activation of the drawing tool (circle)
    static TOOL_ACTION drawCircle;

    /// Activation of the drawing tool (arc)
    static TOOL_ACTION drawArc;

    /// Activation of the drawing tool (text)
    static TOOL_ACTION placeText;

    /// Activation of the drawing tool (dimension)
    static TOOL_ACTION drawDimension;

    /// Activation of the drawing tool (drawing a ZONE)
    static TOOL_ACTION drawZone;

    /// Activation of the drawing tool (drawing a VIA)
    static TOOL_ACTION drawVia;

    /// Activation of the drawing tool (drawing a keepout area)
    static TOOL_ACTION drawZoneKeepout;

    /// Activation of the drawing tool (drawing a ZONE cutout)
    static TOOL_ACTION drawZoneCutout;

    /// Activation of the drawing tool (drawing a similar ZONE to another one)
    static TOOL_ACTION drawSimilarZone;

    /// Activation of the drawing tool (placing a TARGET)
    static TOOL_ACTION placeTarget;

    /// Activation of the drawing tool (placing a MODULE)
    static TOOL_ACTION placeModule;

    /// Activation of the drawing tool (placing a drawing imported from DXF or SVG file)
    static TOOL_ACTION placeImportedGraphics;

    /// Activation of the drawing tool (placing the footprint anchor)
    static TOOL_ACTION setAnchor;

    /// Increase width of currently drawn line
    static TOOL_ACTION incWidth;

    /// Decrease width of currently drawn line
    static TOOL_ACTION decWidth;

    /// Switch posture when drawing arc
    static TOOL_ACTION arcPosture;

    // Push and Shove Router Tool

    /// Activation of the Push and Shove router
    static TOOL_ACTION routerActivateSingle;

    /// Activation of the Push and Shove router (differential pair mode)
    static TOOL_ACTION routerActivateDiffPair;

    /// Activation of the Push and Shove router (tune single line mode)
    static TOOL_ACTION routerActivateTuneSingleTrace;

    /// Activation of the Push and Shove router (diff pair tuning mode)
    static TOOL_ACTION routerActivateTuneDiffPair;

    /// Activation of the Push and Shove router (skew tuning mode)
    static TOOL_ACTION routerActivateTuneDiffPairSkew;

    /// Activation of the Push and Shove settings dialogs
    static TOOL_ACTION routerActivateSettingsDialog;
    static TOOL_ACTION routerActivateDpDimensionsDialog;


    /// Activation of the Push and Shove router (inline dragging mode)
    static TOOL_ACTION routerInlineDrag;

    // Point Editor
    /// Break outline (insert additional points to an edge)
    static TOOL_ACTION pointEditorAddCorner;

    /// Removes a corner
    static TOOL_ACTION pointEditorRemoveCorner;

    // Placement tool
    /// Align items to the top edge of selection bounding box
    static TOOL_ACTION alignTop;

    /// Align items to the bottom edge of selection bounding box
    static TOOL_ACTION alignBottom;

    /// Align items to the left edge of selection bounding box
    static TOOL_ACTION alignLeft;

    /// Align items to the right edge of selection bounding box
    static TOOL_ACTION alignRight;

    /// Align items to the middle of selection bounding box
    static TOOL_ACTION alignCenterX;

    /// Align items to the center of selection bounding box
    static TOOL_ACTION alignCenterY;

    /// Distributes items evenly along the horizontal axis
    static TOOL_ACTION distributeHorizontally;

    /// Distributes items evenly along the vertical axis
    static TOOL_ACTION distributeVertically;

    // Position Relative Tool
    /// Activation of the position relative tool
    static TOOL_ACTION positionRelative;

    /// Selection of anchor item for position relative tool
    static TOOL_ACTION selectpositionRelativeItem;

    // Display modes
    static TOOL_ACTION trackDisplayMode;
    static TOOL_ACTION padDisplayMode;
    static TOOL_ACTION viaDisplayMode;
    static TOOL_ACTION graphicDisplayMode;
    static TOOL_ACTION zoneDisplayEnable;
    static TOOL_ACTION zoneDisplayDisable;
    static TOOL_ACTION zoneDisplayOutlines;
    static TOOL_ACTION highContrastMode;
    static TOOL_ACTION highContrastInc;
    static TOOL_ACTION highContrastDec;

    // Layer control
    static TOOL_ACTION layerTop;
    static TOOL_ACTION layerInner1;
    static TOOL_ACTION layerInner2;
    static TOOL_ACTION layerInner3;
    static TOOL_ACTION layerInner4;
    static TOOL_ACTION layerInner5;
    static TOOL_ACTION layerInner6;
    static TOOL_ACTION layerBottom;
    static TOOL_ACTION layerNext;
    static TOOL_ACTION layerPrev;
    static TOOL_ACTION layerAlphaInc;
    static TOOL_ACTION layerAlphaDec;
    static TOOL_ACTION layerToggle;

    static TOOL_ACTION layerChanged;        // notification

    // Track & via size control
    static TOOL_ACTION trackWidthInc;
    static TOOL_ACTION trackWidthDec;
    static TOOL_ACTION viaSizeInc;
    static TOOL_ACTION viaSizeDec;

    static TOOL_ACTION trackViaSizeChanged;   // notification

    // Zone actions
    static TOOL_ACTION zoneFill;
    static TOOL_ACTION zoneFillAll;
    static TOOL_ACTION zoneUnfill;
    static TOOL_ACTION zoneUnfillAll;
    static TOOL_ACTION zoneMerge;
    static TOOL_ACTION zoneDeleteSegzone;

    /// Duplicate zone onto another layer
    static TOOL_ACTION zoneDuplicate;

    // Module editor tools
    /// Activation of the drawing tool (placing a PAD)
    static TOOL_ACTION placePad;

    static TOOL_ACTION createPadFromShapes;
    static TOOL_ACTION explodePadToShapes;

    /// Tool for quick pad enumeration
    static TOOL_ACTION enumeratePads;

    /// Tool for creating an array of objects
    static TOOL_ACTION createArray;

    /// Copy selected items to clipboard
    static TOOL_ACTION copyToClipboard;

    /// Paste from clipboard
    static TOOL_ACTION pasteFromClipboard;

    /// Paste from clipboard
    static TOOL_ACTION cutToClipboard;

    /// Display module edges as outlines
    static TOOL_ACTION moduleEdgeOutlines;

    /// Display module texts as outlines
    static TOOL_ACTION moduleTextOutlines;

    // Pad tools
    /// Copy the selected pad's settings to the board design settings
    static TOOL_ACTION copyPadSettings;

    /// Copy the pad settings in the board design settings to the selected pad
    static TOOL_ACTION applyPadSettings;

    /// Copy the current pad's settings to other pads in the module or on the board
    static TOOL_ACTION pushPadSettings;

    // Microwave tools
    static TOOL_ACTION microwaveCreateGap;

    static TOOL_ACTION microwaveCreateStub;

    static TOOL_ACTION microwaveCreateStubArc;

    static TOOL_ACTION microwaveCreateFunctionShape;

    static TOOL_ACTION microwaveCreateLine;

    // Locking
    static TOOL_ACTION toggleLock;
    static TOOL_ACTION lock;
    static TOOL_ACTION unlock;

    // Miscellaneous
    static TOOL_ACTION selectionTool;
    static TOOL_ACTION pickerTool;
    static TOOL_ACTION resetCoords;
    static TOOL_ACTION measureTool;
    static TOOL_ACTION switchCursor;
    static TOOL_ACTION switchUnits;
    static TOOL_ACTION updateUnits;
    static TOOL_ACTION deleteItemCursor;
    static TOOL_ACTION clearHighlight;
    static TOOL_ACTION highlightNet;
    static TOOL_ACTION highlightNetCursor;
    static TOOL_ACTION highlightNetSelection;
    static TOOL_ACTION drillOrigin;
    static TOOL_ACTION crossProbeSchToPcb;
    static TOOL_ACTION appendBoard;
    static TOOL_ACTION showHotkeyList;
    static TOOL_ACTION showHelp;
    static TOOL_ACTION toBeDone;

    // Ratsnest
    static TOOL_ACTION showLocalRatsnest;
    static TOOL_ACTION hideLocalRatsnest;
    static TOOL_ACTION updateLocalRatsnest;

    /// Find an item
    static TOOL_ACTION find;

    /// Find an item and start moving
    static TOOL_ACTION findMove;

    static TOOL_ACTION editFootprintInFpEditor;

    static TOOL_ACTION autoplaceOffboardComponents;
    static TOOL_ACTION autoplaceSelectedComponents;

    ///> @copydoc COMMON_ACTIONS::TranslateLegacyId()
    virtual OPT<TOOL_EVENT> TranslateLegacyId( int aId ) override;

    ///> @copydoc COMMON_ACTIONS::RegisterAllTools()
    virtual void RegisterAllTools( TOOL_MANAGER* aToolManager ) override;
};

#endif
