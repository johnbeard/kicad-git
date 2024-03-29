/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2012 KiCad Developers, see change_log.txt for contributors.
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
 * @file modview_frame.h
 */

#ifndef MODVIEWFRM_H_
#define MODVIEWFRM_H_


#include <wx/gdicmn.h>

class wxSashLayoutWindow;
class wxListBox;
class wxSemaphore;
class FP_LIB_TABLE;


/**
 * Component library viewer main window.
 */
class FOOTPRINT_VIEWER_FRAME : public PCB_BASE_FRAME
{
private:
    wxListBox*          m_libList;               // The list of libs names
    wxListBox*          m_footprintList;         // The list of footprint names

    // Flags
    wxSemaphore*        m_semaphore;             // != NULL if the frame emulates a modal dialog
    wxString            m_configPath;            // subpath for configuration

protected:
    static wxString     m_libraryName;           // Current selected library
    static wxString     m_footprintName;         // Current selected footprint
    static wxString     m_selectedFootprintName; // When the viewer is used to select a footprint
                                                 // the selected footprint is here

public:
    FOOTPRINT_VIEWER_FRAME( PCB_BASE_FRAME* aParent, FP_LIB_TABLE* aTable,
                            wxSemaphore* aSemaphore = NULL,
                            long aStyle = KICAD_DEFAULT_DRAWFRAME_STYLE );

    ~FOOTPRINT_VIEWER_FRAME();

    /**
     * Function GetFootprintViewerFrameName (static)
     * @return the frame name used when creating the frame
     * used to get a reference to this frame, if exists
     */
    static const wxChar* GetFootprintViewerFrameName();

    /**
     * Function GetActiveFootprintViewer (static)
     * @return a reference to the current opened Footprint viewer
     * or NULL if no Footprint viewer currently opened
     */
    static FOOTPRINT_VIEWER_FRAME* GetActiveFootprintViewer();

    wxString& GetSelectedFootprint( void ) const { return m_selectedFootprintName; }
    const wxString GetSelectedLibraryFullName( void );

    /**
     * Function GetSelectedLibrary
     * @return the selected library name from the #FP_LIB_TABLE.
     */
    const wxString& GetSelectedLibrary() { return m_libraryName; }

    virtual EDA_COLOR_T GetGridColor( void ) const;

    /**
     * Function ReCreateLibraryList
     *
     * Creates or recreates the list of current loaded libraries.
     * This list is sorted, with the library cache always at end of the list
     */
    void ReCreateLibraryList();

private:

    void OnSize( wxSizeEvent& event );

    void ReCreateFootprintList();
    void OnIterateFootprintList( wxCommandEvent& event );

    /**
     * Function UpdateTitle
     * updates the window title with current library information.
     */
    void UpdateTitle();

    /**
     * Function RedrawActiveWindow
     * Display the current selected component.
     * If the component is an alias, the ROOT component is displayed
     */
    void RedrawActiveWindow( wxDC* DC, bool EraseBg );

    void OnCloseWindow( wxCloseEvent& Event );
    void ReCreateHToolbar();
    void ReCreateVToolbar();
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos );
    void ClickOnLibList( wxCommandEvent& event );
    void ClickOnFootprintList( wxCommandEvent& event );
    void DClickOnFootprintList( wxCommandEvent& event );
    void OnSetRelativeOffset( wxCommandEvent& event );

    void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 );

    /**
     * Function LoadSettings
     * loads the library viewer frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get loaded.
     */
    void LoadSettings();

    /**
     * Function SaveSettings
     * save library viewer frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get saved.
     */
    void SaveSettings();

    wxString& GetFootprintName( void ) const { return m_footprintName; }

    /**
     * Function OnActivate
     * is called when the frame frame is activate to reload the libraries and component lists
     * that can be changed by the schematic editor or the library editor.
     */
    virtual void OnActivate( wxActivateEvent& event );

    void SelectCurrentLibrary( wxCommandEvent& event );

    /**
     * Function SelectCurrentFootprint
     * Selects the current footprint name and display it
     */
    void SelectCurrentFootprint( wxCommandEvent& event );

    /**
     * Function ExportSelectedFootprint
     * exports the current footprint name and close the library browser.
     */
    void ExportSelectedFootprint( wxCommandEvent& event );

    /**
     * Function SelectAndViewFootprint
     * Select and load the next or the previous footprint
     * if no current footprint, Rebuild the list of footprints available in a given footprint
     * library
     * @param aMode = NEXT_PART or PREVIOUS_PART
     */
    void SelectAndViewFootprint( int aMode );

    bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu );

    /**
     * Function Show3D_Frame (virtual)
     * displays 3D view of the footprint (module) being edited.
     */
    void Show3D_Frame( wxCommandEvent& event );

    /**
     * Function Update3D_Frame
     * must be called after a footprint selection
     * Updates the 3D view and 3D frame title.
     * @param aForceReloadFootprint = true to reload data (default)
     *   = false to update title only -(after creating the 3D viewer)
     */
    void Update3D_Frame( bool aForceReloadFootprint = true );

    /*
     * Virtual functions, not used here, but needed by PCB_BASE_FRAME
     * (virtual pure functions )
     */
    void OnLeftDClick( wxDC*, const wxPoint& ) {}
    void SaveCopyInUndoList( BOARD_ITEM*, UNDO_REDO_T, const wxPoint& ) {}
    void SaveCopyInUndoList( const PICKED_ITEMS_LIST&, UNDO_REDO_T, const wxPoint &) {}


    DECLARE_EVENT_TABLE()
};

#endif  // MODVIEWFRM_H_
