/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2018 CERN
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#include <map>
#include <stack>
#include <algorithm>

#include <core/optional.h>

#include <wx/event.h>
#include <wx/clipbrd.h>

#include <view/view.h>

#include <tool/tool_base.h>
#include <tool/tool_interactive.h>
#include <tool/tool_manager.h>
#include <tool/context_menu.h>
#include <tool/coroutine.h>
#include <tool/action_manager.h>

#include <pcb_edit_frame.h>
#include <class_draw_panel_gal.h>

/// Struct describing the current execution state of a TOOL
struct TOOL_MANAGER::TOOL_STATE
{
    TOOL_STATE( TOOL_BASE* aTool ) :
        theTool( aTool )
    {
        clear();
    }

    TOOL_STATE( const TOOL_STATE& aState )
    {
        theTool = aState.theTool;
        idle = aState.idle;
        pendingWait = aState.pendingWait;
        pendingContextMenu = aState.pendingContextMenu;
        contextMenu = aState.contextMenu;
        contextMenuTrigger = aState.contextMenuTrigger;
        cofunc = aState.cofunc;
        wakeupEvent = aState.wakeupEvent;
        waitEvents = aState.waitEvents;
        transitions = aState.transitions;
        vcSettings = aState.vcSettings;
        // do not copy stateStack
    }

    ~TOOL_STATE()
    {
        wxASSERT( stateStack.empty() );
    }

    /// The tool itself
    TOOL_BASE* theTool;

    /// Is the tool active (pending execution) or disabled at the moment
    bool idle;

    /// Flag defining if the tool is waiting for any event (i.e. if it
    /// issued a Wait() call).
    bool pendingWait;

    /// Is there a context menu being displayed
    bool pendingContextMenu;

    /// Context menu currently used by the tool
    CONTEXT_MENU* contextMenu;

    /// Defines when the context menu is opened
    CONTEXT_MENU_TRIGGER contextMenuTrigger;

    /// Tool execution context
    COROUTINE<int, const TOOL_EVENT&>* cofunc;

    /// The event that triggered the execution/wakeup of the tool after Wait() call
    TOOL_EVENT wakeupEvent;

    /// List of events the tool is currently waiting for
    TOOL_EVENT_LIST waitEvents;

    /// List of possible transitions (ie. association of events and state handlers that are executed
    /// upon the event reception
    std::vector<TRANSITION> transitions;

    /// VIEW_CONTROLS settings to preserve settings when the tools are switched
    KIGFX::VC_SETTINGS vcSettings;

    TOOL_STATE& operator=( const TOOL_STATE& aState )
    {
        theTool = aState.theTool;
        idle = aState.idle;
        pendingWait = aState.pendingWait;
        pendingContextMenu = aState.pendingContextMenu;
        contextMenu = aState.contextMenu;
        contextMenuTrigger = aState.contextMenuTrigger;
        cofunc = aState.cofunc;
        wakeupEvent = aState.wakeupEvent;
        waitEvents = aState.waitEvents;
        transitions = aState.transitions;
        vcSettings = aState.vcSettings;
        // do not copy stateStack
        return *this;
    }

    bool operator==( const TOOL_MANAGER::TOOL_STATE& aRhs ) const
    {
        return aRhs.theTool == this->theTool;
    }

    bool operator!=( const TOOL_MANAGER::TOOL_STATE& aRhs ) const
    {
        return aRhs.theTool != this->theTool;
    }

    /**
     * Function Push()
     * Stores the current state of the tool on stack. Stacks are stored internally and are not
     * shared between different TOOL_STATE objects.
     */
    void Push()
    {
        auto state = std::make_unique<TOOL_STATE>( *this );
        stateStack.push( std::move( state ) );
        clear();
    }

    /**
     * Function Pop()
     * Restores state of the tool from stack. Stacks are stored internally and are not
     * shared between different TOOL_STATE objects.
     * @return True if state was restored, false if the stack was empty.
     */
    bool Pop()
    {
        delete cofunc;

        if( !stateStack.empty() )
        {
            *this = *stateStack.top().get();
            stateStack.pop();
            return true;
        }
        else
        {
            cofunc = NULL;
            return false;
        }
    }

private:
    ///> Stack preserving previous states of a TOOL.
    std::stack<std::unique_ptr<TOOL_STATE>> stateStack;

    ///> Restores the initial state.
    void clear()
    {
        idle = true;
        pendingWait = false;
        pendingContextMenu = false;
        cofunc = NULL;
        contextMenu = NULL;
        contextMenuTrigger = CMENU_OFF;
        vcSettings.Reset();
        transitions.clear();
    }
};


TOOL_MANAGER::TOOL_MANAGER() :
    m_model( NULL ),
    m_view( NULL ),
    m_viewControls( NULL ),
    m_editFrame( NULL ),
    m_passEvent( false ),
    m_menuActive( false ),
    m_menuOwner( -1 ),
    m_activeState( nullptr )
{
    m_actionMgr = new ACTION_MANAGER( this );
}


TOOL_MANAGER::~TOOL_MANAGER()
{
    std::map<TOOL_BASE*, TOOL_STATE*>::iterator it, it_end;

    for( it = m_toolState.begin(), it_end = m_toolState.end(); it != it_end; ++it )
    {
        delete it->second->cofunc;  // delete cofunction
        delete it->second;          // delete TOOL_STATE
        delete it->first;           // delete the tool itself
    }

    delete m_actionMgr;
}


void TOOL_MANAGER::RegisterTool( TOOL_BASE* aTool )
{
    wxASSERT_MSG( m_toolNameIndex.find( aTool->GetName() ) == m_toolNameIndex.end(),
            wxT( "Adding two tools with the same name may result in unexpected behaviour.") );
    wxASSERT_MSG( m_toolIdIndex.find( aTool->GetId() ) == m_toolIdIndex.end(),
            wxT( "Adding two tools with the same ID may result in unexpected behaviour.") );
    wxASSERT_MSG( m_toolTypes.find( typeid( *aTool ).name() ) == m_toolTypes.end(),
            wxT( "Adding two tools of the same type may result in unexpected behaviour.") );

    TOOL_STATE* st = new TOOL_STATE( aTool );

    m_toolState[aTool] = st;
    m_toolNameIndex[aTool->GetName()] = st;
    m_toolIdIndex[aTool->GetId()] = st;
    m_toolTypes[typeid( *aTool ).name()] = st->theTool;

    aTool->attachManager( this );
}


bool TOOL_MANAGER::InvokeTool( TOOL_ID aToolId )
{
    TOOL_BASE* tool = FindTool( aToolId );

    if( tool && tool->GetType() == INTERACTIVE )
        return invokeTool( tool );

    return false;       // there is no tool with the given id
}


bool TOOL_MANAGER::InvokeTool( const std::string& aToolName )
{
    TOOL_BASE* tool = FindTool( aToolName );

    if( tool && tool->GetType() == INTERACTIVE )
        return invokeTool( tool );

    return false;       // there is no tool with the given name
}


void TOOL_MANAGER::RegisterAction( TOOL_ACTION* aAction )
{
    m_actionMgr->RegisterAction( aAction );
}


void TOOL_MANAGER::UnregisterAction( TOOL_ACTION* aAction )
{
    m_actionMgr->UnregisterAction( aAction );
}


bool TOOL_MANAGER::RunAction( const std::string& aActionName, bool aNow, void* aParam )
{
    TOOL_ACTION* action = m_actionMgr->FindAction( aActionName );

    if( !action )
    {
        wxASSERT_MSG( false, wxString::Format( wxT( "Could not find action %s." ), aActionName ) );
        return false;
    }

    RunAction( *action, aNow, aParam );

    return false;
}


void TOOL_MANAGER::RunAction( const TOOL_ACTION& aAction, bool aNow, void* aParam )
{
    TOOL_EVENT event = aAction.MakeEvent();

    // Allow to override the action parameter
    if( aParam )
        event.SetParameter( aParam );

    if( aNow )
    {
        TOOL_STATE* current = m_activeState;
        processEvent( event );
        setActiveState( current );
    }
    else
    {
        PostEvent( event );
    }
}


int TOOL_MANAGER::GetHotKey( const TOOL_ACTION& aAction )
{
    return m_actionMgr->GetHotKey( aAction );
}


void TOOL_MANAGER::UpdateHotKeys()
{
    m_actionMgr->UpdateHotKeys();
}


bool TOOL_MANAGER::invokeTool( TOOL_BASE* aTool )
{
    wxASSERT( aTool != NULL );

    TOOL_EVENT evt( TC_COMMAND, TA_ACTIVATE, aTool->GetName() );
    processEvent( evt );

    if( TOOL_STATE* active = GetCurrentToolState() )
        setActiveState( active );

    return true;
}


bool TOOL_MANAGER::runTool( TOOL_ID aToolId )
{
    TOOL_BASE* tool = FindTool( aToolId );

    if( tool && tool->GetType() == INTERACTIVE )
        return runTool( tool );

    return false;       // there is no tool with the given id
}


bool TOOL_MANAGER::runTool( const std::string& aToolName )
{
    TOOL_BASE* tool = FindTool( aToolName );

    if( tool && tool->GetType() == INTERACTIVE )
        return runTool( tool );

    return false;       // there is no tool with the given name
}


bool TOOL_MANAGER::runTool( TOOL_BASE* aTool )
{
    wxASSERT( aTool != NULL );

    if( !isRegistered( aTool ) )
    {
        wxASSERT_MSG( false, wxT( "You cannot run unregistered tools" ) );
        return false;
    }

    TOOL_ID id = aTool->GetId();

    if( aTool->GetType() == INTERACTIVE )
        static_cast<TOOL_INTERACTIVE*>( aTool )->resetTransitions();

    // If the tool is already active, bring it to the top of the active tools stack
    if( isActive( aTool ) && m_activeTools.size() > 1 )
    {
        auto it = std::find( m_activeTools.begin(), m_activeTools.end(), id );

        if( it != m_activeTools.end() )
        {
            if( it != m_activeTools.begin() )
            {
                m_activeTools.erase( it );
                m_activeTools.push_front( id );
            }

            return false;
        }
    }

    setActiveState( m_toolIdIndex[id] );
    aTool->Reset( TOOL_INTERACTIVE::RUN );

    // Add the tool on the front of the processing queue (it gets events first)
    m_activeTools.push_front( id );

    return true;
}


TOOL_BASE* TOOL_MANAGER::FindTool( int aId ) const
{
    std::map<TOOL_ID, TOOL_STATE*>::const_iterator it = m_toolIdIndex.find( aId );

    if( it != m_toolIdIndex.end() )
        return it->second->theTool;

    return NULL;
}


TOOL_BASE* TOOL_MANAGER::FindTool( const std::string& aName ) const
{
    std::map<std::string, TOOL_STATE*>::const_iterator it = m_toolNameIndex.find( aName );

    if( it != m_toolNameIndex.end() )
        return it->second->theTool;

    return NULL;
}


void TOOL_MANAGER::DeactivateTool()
{
    // Deactivate the active tool, but do not run anything new
    TOOL_EVENT evt( TC_COMMAND, TA_CANCEL_TOOL );
    processEvent( evt );
}


void TOOL_MANAGER::ResetTools( TOOL_BASE::RESET_REASON aReason )
{
    DeactivateTool();

    for( auto& state : m_toolState )
    {
        TOOL_BASE* tool = state.first;
        setActiveState( state.second );
        tool->Reset( aReason );

        if( tool->GetType() == INTERACTIVE )
            static_cast<TOOL_INTERACTIVE*>( tool )->resetTransitions();
    }
}


void TOOL_MANAGER::InitTools()
{
    for( auto it = m_toolState.begin(); it != m_toolState.end(); /* iteration in the loop */ )
    {
        TOOL_BASE* tool = it->first;
        TOOL_STATE* state = it->second;
        setActiveState( state );
        ++it;   // keep the iterator valid if the element is going to be erased

        if( !tool->Init() )
        {
            wxMessageBox(
                    wxString::Format( "Initialization of tool \"%s\" failed", tool->GetName() ) );

            // Unregister the tool
            setActiveState( nullptr );
            m_toolState.erase( tool );
            m_toolNameIndex.erase( tool->GetName() );
            m_toolIdIndex.erase( tool->GetId() );
            m_toolTypes.erase( typeid( *tool ).name() );

            delete state;
            delete tool;
        }
    }

    ResetTools( TOOL_BASE::RUN );
}


int TOOL_MANAGER::GetPriority( int aToolId ) const
{
    int priority = 0;

    for( auto it = m_activeTools.begin(), itEnd = m_activeTools.end(); it != itEnd; ++it )
    {
        if( *it == aToolId )
            return priority;

        ++priority;
    }

    return -1;
}


void TOOL_MANAGER::ScheduleNextState( TOOL_BASE* aTool, TOOL_STATE_FUNC& aHandler,
                                      const TOOL_EVENT_LIST& aConditions )
{
    TOOL_STATE* st = m_toolState[aTool];

    st->transitions.push_back( TRANSITION( aConditions, aHandler ) );
}


void TOOL_MANAGER::ClearTransitions( TOOL_BASE* aTool )
{
    m_toolState[aTool]->transitions.clear();
}


void TOOL_MANAGER::RunMainStack( TOOL_BASE* aTool, std::function<void()> aFunc )
{
    TOOL_STATE* st = m_toolState[aTool];
    setActiveState( st );
    st->cofunc->RunMainStack( std::move( aFunc ) );
}


OPT<TOOL_EVENT> TOOL_MANAGER::ScheduleWait( TOOL_BASE* aTool, const TOOL_EVENT_LIST& aConditions )
{
    TOOL_STATE* st = m_toolState[aTool];

    wxASSERT( !st->pendingWait ); // everything collapses on two KiYield() in a row

    // indicate to the manager that we are going to sleep and we shall be
    // woken up when an event matching aConditions arrive
    st->pendingWait = true;
    st->waitEvents = aConditions;

    // switch context back to event dispatcher loop
    st->cofunc->KiYield();

    return st->wakeupEvent;
}


void TOOL_MANAGER::dispatchInternal( const TOOL_EVENT& aEvent )
{
    // iterate over all registered tools
    for( auto it = m_activeTools.begin(); it != m_activeTools.end(); ++it )
    {
        TOOL_STATE* st = m_toolIdIndex[*it];

        // forward context menu events to the tool that created the menu
        if( aEvent.IsMenu() )
        {
            if( *it != m_menuOwner )
                continue;
        }

        // the tool state handler is waiting for events (i.e. called Wait() method)
        if( st->pendingWait )
        {
            if( st->waitEvents.Matches( aEvent ) )
            {
                // By default only messages are passed further
                m_passEvent = ( aEvent.Category() == TC_MESSAGE );

                // got matching event? clear wait list and wake up the coroutine
                st->wakeupEvent = aEvent;
                st->pendingWait = false;
                st->waitEvents.clear();

                if( st->cofunc )
                {
                    setActiveState( st );
                    bool end = !st->cofunc->Resume();

                    if( end )
                        it = finishTool( st );
                }

                // If the tool did not request to propagate
                // the event to other tools, we should stop it now
                if( !m_passEvent )
                    break;
            }
        }
    }

    for( auto& state : m_toolState )
    {
        TOOL_STATE* st = state.second;
        bool finished = false;

        // no state handler in progress - check if there are any transitions (defined by
        // Go() method that match the event.
        if( !st->transitions.empty() )
        {
            for( TRANSITION& tr : st->transitions )
            {
                if( tr.first.Matches( aEvent ) )
                {
                    auto func_copy = tr.second;

                    // if there is already a context, then push it on the stack
                    // and transfer the previous view control settings to the new context
                    if( st->cofunc )
                    {
                        auto vc = st->vcSettings;
                        st->Push();
                        st->vcSettings = vc;
                    }

                    st->cofunc = new COROUTINE<int, const TOOL_EVENT&>( std::move( func_copy ) );

                    // as the state changes, the transition table has to be set up again
                    st->transitions.clear();

                    // got match? Run the handler.
                    setActiveState( st );
                    st->idle = false;
                    st->cofunc->Call( aEvent );

                    if( !st->cofunc->Running() )
                        finishTool( st ); // The couroutine has finished immediately?

                    // if it is a message, continue processing
                    finished = !( aEvent.Category() == TC_MESSAGE );

                    // there is no point in further checking, as transitions got cleared
                    break;
                }
            }
        }

        if( finished )
            break;      // only the first tool gets the event
    }
}


bool TOOL_MANAGER::dispatchStandardEvents( const TOOL_EVENT& aEvent )
{
    if( aEvent.Action() == TA_KEY_PRESSED )
    {
        // Check if there is a hotkey associated
        if( m_actionMgr->RunHotKey( aEvent.Modifier() | aEvent.KeyCode() ) )
            return false;                 // hotkey event was handled so it does not go any further
    }

    return true;
}


bool TOOL_MANAGER::dispatchActivation( const TOOL_EVENT& aEvent )
{
    if( aEvent.IsActivate() )
    {
        std::map<std::string, TOOL_STATE*>::iterator tool = m_toolNameIndex.find( *aEvent.GetCommandStr() );

        if( tool != m_toolNameIndex.end() )
        {
            runTool( tool->second->theTool );
            return true;
        }
    }

    return false;
}

void TOOL_MANAGER::dispatchContextMenu( const TOOL_EVENT& aEvent )
{
    for( TOOL_ID toolId : m_activeTools )
    {
        TOOL_STATE* st = m_toolIdIndex[toolId];

        // the tool requested a context menu. The menu is activated on RMB click (CMENU_BUTTON mode)
        // or immediately (CMENU_NOW) mode. The latter is used for clarification lists.
        if( st->contextMenuTrigger == CMENU_OFF )
            continue;

        if( st->contextMenuTrigger == CMENU_BUTTON && !aEvent.IsClick( BUT_RIGHT ) )
            break;

        st->pendingWait = true;
        st->waitEvents = TOOL_EVENT( TC_ANY, TA_ANY );

        // Store the menu pointer in case it is changed by the TOOL when handling menu events
        CONTEXT_MENU* m = st->contextMenu;

        if( st->contextMenuTrigger == CMENU_NOW )
            st->contextMenuTrigger = CMENU_OFF;

        // Store the cursor position, so the tools could execute actions
        // using the point where the user has invoked a context menu
        m_menuCursor = m_viewControls->GetCursorPosition();

        // Save all tools cursor settings, as they will be overridden
        for( auto idState : m_toolIdIndex )
        {
            TOOL_STATE* s = idState.second;
            const auto& vc = s->vcSettings;

            if( vc.m_forceCursorPosition )
                m_cursorSettings[idState.first] = vc.m_forcedPosition;
            else
                m_cursorSettings[idState.first] = NULLOPT;
        }

        m_viewControls->ForceCursorPosition( true, m_menuCursor );

        // Display a copy of menu
        std::unique_ptr<CONTEXT_MENU> menu( m->Clone() );

        // Run update handlers on the created copy
        menu->UpdateAll();
        m_menuOwner = toolId;
        m_menuActive = true;

        auto frame = dynamic_cast<wxFrame*>( m_editFrame );

        if( frame )
            frame->PopupMenu( menu.get() );

        // Warp the cursor if a menu item was selected
        if( menu->GetSelected() >= 0 && m_warpMouseAfterContextMenu )
            m_viewControls->WarpCursor( m_menuCursor, true, false );
        // Otherwise notify the tool of a cancelled menu
        else
        {
            TOOL_EVENT evt( TC_COMMAND, TA_CONTEXT_MENU_CHOICE, -1 );
            evt.SetParameter( m );
            dispatchInternal( evt );
        }

        // Restore setting in case it was vetoed
        m_warpMouseAfterContextMenu = true;

        // Notify the tools that menu has been closed
        TOOL_EVENT evt( TC_COMMAND, TA_CONTEXT_MENU_CLOSED );
        evt.SetParameter( m );
        dispatchInternal( evt );

        m_menuActive = false;
        m_menuOwner = -1;

        // Restore cursor settings
        for( auto cursorSetting : m_cursorSettings )
        {
            auto it = m_toolIdIndex.find( cursorSetting.first );
            wxASSERT( it != m_toolIdIndex.end() );

            if( it == m_toolIdIndex.end() )
                continue;

            KIGFX::VC_SETTINGS& vc = it->second->vcSettings;
            vc.m_forceCursorPosition = (bool) cursorSetting.second;
            vc.m_forcedPosition = cursorSetting.second ? *cursorSetting.second : VECTOR2D( 0, 0 );
        }

        m_cursorSettings.clear();
        break;
    }
}


TOOL_MANAGER::ID_LIST::iterator TOOL_MANAGER::finishTool( TOOL_STATE* aState )
{
    auto it = std::find( m_activeTools.begin(), m_activeTools.end(), aState->theTool->GetId() );

    if( !aState->Pop() )
    {
        // Deactivate the tool if there are no other contexts saved on the stack
        if( it != m_activeTools.end() )
            it = m_activeTools.erase( it );

        aState->idle = true;
    }

    if( aState == m_activeState )
        setActiveState( nullptr );

    // Set transitions to be ready for future TOOL_EVENTs
    TOOL_BASE* tool = aState->theTool;

    if( tool->GetType() == INTERACTIVE )
        static_cast<TOOL_INTERACTIVE*>( tool )->resetTransitions();

    return it;
}


bool TOOL_MANAGER::ProcessEvent( const TOOL_EVENT& aEvent )
{
    bool hotkey_handled = processEvent( aEvent );

    if( TOOL_STATE* active = GetCurrentToolState() )
        setActiveState( active );

    if( m_view->IsDirty() )
    {
        auto f = dynamic_cast<EDA_DRAW_FRAME*>( GetEditFrame() );

        if( f )
            f->GetGalCanvas()->Refresh(); // fixme: ugly hack, provide a method in TOOL_DISPATCHER.

#if defined( __WXMAC__ )
        wxTheApp->ProcessPendingEvents(); // required for updating brightening behind a popup menu
#endif
    }

    return hotkey_handled;
}


void TOOL_MANAGER::ScheduleContextMenu( TOOL_BASE* aTool, CONTEXT_MENU* aMenu,
                                        CONTEXT_MENU_TRIGGER aTrigger )
{
    TOOL_STATE* st = m_toolState[aTool];

    st->contextMenu = aMenu;
    st->contextMenuTrigger = aTrigger;
}


bool TOOL_MANAGER::SaveClipboard( const std::string& aText )
{
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->SetData( new wxTextDataObject( wxString( aText.c_str(), wxConvUTF8 ) ) );
        wxTheClipboard->Close();

        return true;
    }

    return false;
}


std::string TOOL_MANAGER::GetClipboard() const
{
    std::string result;

    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_TEXT ) )
        {
            wxTextDataObject data;
            wxTheClipboard->GetData( data );

            result = data.GetText().mb_str();
        }

        wxTheClipboard->Close();
    }

    return result;
}


const KIGFX::VC_SETTINGS& TOOL_MANAGER::GetCurrentToolVC() const
{
    if( TOOL_STATE* active = GetCurrentToolState() )
        return active->vcSettings;

    return m_viewControls->GetSettings();
}


TOOL_ID TOOL_MANAGER::MakeToolId( const std::string& aToolName )
{
    static int currentId;

    return currentId++;
}


void TOOL_MANAGER::SetEnvironment( EDA_ITEM* aModel, KIGFX::VIEW* aView,
                                   KIGFX::VIEW_CONTROLS* aViewControls, wxWindow* aFrame )
{
    m_model = aModel;
    m_view = aView;
    m_viewControls = aViewControls;
    m_editFrame = aFrame;
    m_actionMgr->UpdateHotKeys();
}


bool TOOL_MANAGER::isActive( TOOL_BASE* aTool )
{
    if( !isRegistered( aTool ) )
        return false;

    // Just check if the tool is on the active tools stack
    return std::find( m_activeTools.begin(), m_activeTools.end(), aTool->GetId() ) != m_activeTools.end();
}


void TOOL_MANAGER::saveViewControls( TOOL_STATE* aState )
{
    aState->vcSettings = m_viewControls->GetSettings();

    if( m_menuActive )
    {
        // Context menu is active, so the cursor settings are overridden (see dispatchContextMenu())
        auto it = m_cursorSettings.find( aState->theTool->GetId() );

        if( it != m_cursorSettings.end() )
        {
            const KIGFX::VC_SETTINGS& curr = m_viewControls->GetSettings();

            // Tool has overridden the cursor position, so store the new settings
            if( !curr.m_forceCursorPosition || curr.m_forcedPosition != m_menuCursor )
            {
                if( !curr.m_forceCursorPosition )
                    it->second = NULLOPT;
                else
                    it->second = curr.m_forcedPosition;
            }
            else
            {
                OPT<VECTOR2D> cursor = it->second;

                if( cursor )
                {
                    aState->vcSettings.m_forceCursorPosition = true;
                    aState->vcSettings.m_forcedPosition = *cursor;
                }
                else
                {
                    aState->vcSettings.m_forceCursorPosition = false;
                }
            }
        }
    }
}


void TOOL_MANAGER::applyViewControls( TOOL_STATE* aState )
{
    m_viewControls->ApplySettings( aState->vcSettings );
}


bool TOOL_MANAGER::processEvent( const TOOL_EVENT& aEvent )
{
    // Early dispatch of events destined for the TOOL_MANAGER
    if( !dispatchStandardEvents( aEvent ) )
        return true;

    dispatchInternal( aEvent );
    dispatchActivation( aEvent );
    dispatchContextMenu( aEvent );

    // Dispatch queue
    while( !m_eventQueue.empty() )
    {
        TOOL_EVENT event = m_eventQueue.front();
        m_eventQueue.pop_front();
        processEvent( event );
    }

    return false;
}


void TOOL_MANAGER::setActiveState( TOOL_STATE* aState )
{
    if( m_activeState )
        saveViewControls( m_activeState );

    m_activeState = aState;

    if( m_activeState )
        applyViewControls( aState );
}


bool TOOL_MANAGER::IsToolActive( TOOL_ID aId ) const
{
    auto it = m_toolIdIndex.find( aId );
    return !it->second->idle;
}
