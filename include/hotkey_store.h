/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 * @file hotkey_store.h
 */

#ifndef HOTKEY_STORE__H
#define HOTKEY_STORE__H


#include <hotkeys_basic.h>


/**
 * Class that manages a hotkey that can be changed, reset to its
 * old value, a default or saved.
 */
class CHANGED_HOTKEY
{
public:
    CHANGED_HOTKEY( EDA_HOTKEY& aHotkey, const wxString& aTag ):
        m_orig( aHotkey ),
        m_changed( aHotkey ),
        m_tag( aTag )
    {
        m_isValid = false;
        m_validMessage = _( "Hotkey never verified" );
    }

    EDA_HOTKEY& GetCurrentValue()
    {
        return m_changed;
    }

    const EDA_HOTKEY& GetCurrentValue() const
    {
        return m_changed;
    }

    /**
     * Gets the original value of the hotkey. This is what the hotkey used
     * to be, and what it would be set to if reset.
     *
     * @return reference to the original hotkey.
     */
    const EDA_HOTKEY& GetOriginalValue() const
    {
        return m_orig;
    }

    /**
     * Save changed hotkey to the original location.
     */
    void SaveHotkey()
    {
        m_orig = m_changed;
    }

    /**
     * @brief Return true if the hotkey doesn't match the original (i.e. it
     * has been changed)
     */
    bool HasUnsavedChange() const
    {
        return m_orig.m_KeyCode != m_changed.m_KeyCode;
    }

    const wxString& GetSectionTag() const
    {
        return m_tag;
    }

    /**
     * Return whether this hotkey has been flagged as invalid
     *
     * @param aMessage - If invalid, contains a string giving the reason for being invalid
     * @return - true if valid, false otherwise
     */
    bool IsValid( wxString& aMessage ) const
    {
        aMessage = m_validMessage;
        return m_isValid;
    }

    /**
     * Return whether this hotkey has been flagged as invalid
     *
     * @return - true if valid, false otherwise
     */
    bool IsValid() const
    {
        return m_isValid;
    }

    /**
     * Set if this hotkey is valid
     *
     * @param aValid - Flag giving true if valid, false otherwise
     * @param amessage - Reason for being invalid (empty if hotkey is valid)
     */
    void SetValidity( bool aValid, wxString& aMessage )
    {
        m_isValid = aValid;
        m_validMessage = aMessage;
    }

private:
    // Reference to an "original" hotkey config
    EDA_HOTKEY&     m_orig;

    // A separate changeable config
    EDA_HOTKEY      m_changed;

    // The hotkey section tag, used to spot conflicts
    const wxString& m_tag;

    // True if the key is a valid hotkey (has no invalid combinations)
    bool m_isValid;

    // Reason for being invalid
    wxString m_validMessage;
};

/**
 * Associates a set of hotkeys (a section) with a display name and the hotkeys
 */
struct HOTKEY_SECTION
{
    // The displayed, translated, name of the section
    wxString                    m_name;

    // List of update-able hotkey data for this section
    std::vector<CHANGED_HOTKEY> m_hotkeys;

    // Back reference to the underlying hotkey data of this section
    EDA_HOTKEY_CONFIG&          m_section;
};


/**
 * A class that contains a set of hotkeys, arranged into "sections"
 * and provides some book-keeping functions for them.
 */
class HOTKEY_STORE
{
public:

    using SECTION_LIST = std::vector<HOTKEY_SECTION>;

    /**
     * Construct a HOTKEY_STORE from a list of hotkey sections
     *
     * @param aHotkeys the hotkey configs that will be managed by this store.
     */
    HOTKEY_STORE( EDA_HOTKEY_CONFIG* aHotkeys );

    /**
     * Get the list of sections managed by this store
     */
    SECTION_LIST& GetSections();


    /**
     * Find a hotkey with the given command ID and in the given section
     * @return pointer to the hotkey if found.
     */
    CHANGED_HOTKEY* FindHotkey( const wxString& aTag, int aCmdId );

    /**
     * Persist all changes to hotkeys in the store to the underlying
     * data structures.
     */
    void SaveAllHotkeys();

    /**
     * Reset every hotkey in the store to the default values
     */
    void ResetAllHotkeysToDefault();

    /**
     * Resets every hotkey to the original values.
     */
    void ResetAllHotkeysToOriginal();

    /**
     * Check whether the given key conflicts with anything in this store. If a command ID is
     * specified, then the conflict will only trigger if the conflicting hotkey is for
     * a different command ID.
     *
     * @param aKey - key to check
     * @param aSectionTag - section tag into which the key is proposed to be installed
     * @param aConfKey - if not NULL, outparam getting the key this one conflicts with
     * @param aConfSect - if not NULL, outparam getting the section this one conflicts with
     * @param aIdCommand - Optional command ID for the key being tested
     */
    bool CheckKeyConflicts( long aKey, const wxString& aSectionTag, EDA_HOTKEY** aConfKey,
            EDA_HOTKEY_CONFIG** aConfSect, const int aIdCommand = -1 );

    /**
     * Check if a given key contains only valid key combinations
     *
     * @param aKey - The key to check
     * @param aMessage - If invalid, the outparam containing the message explaining the invalidity
     * @return - true if valid, false if invalid
     */
    static bool CheckKeyValidity( long aKey, wxString& aMessage );

    /**
     * Test all hotkeys in the hotkey store for validity and conflicts with other keys
     */
    bool TestStoreValidity();

    /**
     * Get a string containing all the errors detected during the validity test
     * It is formatted as:
     *    Action name 1: Reason key is invalid
     *    Action name 2: Reason key is invalid
     *    ...
     *
     * @param aMessage - outparam to store the message in
     * @return true if no errors detected
     */
    bool GetStoreValidityMessage( wxString& aMessage )
    {
        aMessage = m_invalidityCauses;
        return m_isValid;
    }

private:

    /**
     * Generate a HOTKEY_SECTION for a single section
     * described by an EDA_HOTKEY_CONFIG
     */
    HOTKEY_SECTION genSection( EDA_HOTKEY_CONFIG& aSection );

    // Internal data for every hotkey passed in
    SECTION_LIST m_hk_sections;

    // String containing information on the causes of invalidity for the entire store
    wxString m_invalidityCauses;

    // Is the store valid
    bool m_isValid;
};

#endif // HOTKEY_STORE__H
