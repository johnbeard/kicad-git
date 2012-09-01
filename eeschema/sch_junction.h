/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file sch_junction.h
 */

#ifndef _SCH_JUNCTION_H_
#define _SCH_JUNCTION_H_


#include <sch_item_struct.h>


class SCH_JUNCTION : public SCH_ITEM
{
    wxPoint m_pos;                  /* XY coordinates of connection. */
    wxSize  m_size;

public:
    SCH_JUNCTION( const wxPoint& pos = wxPoint( 0, 0 ) );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~SCH_JUNCTION() { }

    wxString GetClass() const
    {
        return wxT( "SCH_JUNCTION" );
    }

    void SwapData( SCH_ITEM* aItem );

    EDA_RECT GetBoundingBox() const;

    void Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
               GR_DRAWMODE aDrawMode, int aColor = -1 );

    bool Save( FILE* aFile ) const;

    bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    void Move( const wxPoint& aMoveVector )
    {
        m_pos += aMoveVector;
    }

    void MirrorY( int aYaxis_position );

    void MirrorX( int aXaxis_position );

    void Rotate( wxPoint aPosition );

    void GetEndPoints( std::vector <DANGLING_END_ITEM>& aItemList );

    bool IsSelectStateChanged( const wxRect& aRect );

    bool IsConnectable() const { return true; }

    void GetConnectionPoints( vector< wxPoint >& aPoints ) const;

    wxString GetSelectMenuText() const { return wxString( _( "Junction" ) ); }

    BITMAP_DEF GetMenuImage() const { return  add_junction_xpm; }

    void GetNetListItem( vector<NETLIST_OBJECT*>& aNetListItems, SCH_SHEET_PATH* aSheetPath );

    wxPoint GetPosition() const { return m_pos; }

    void SetPosition( const wxPoint& aPosition ) { m_pos = aPosition; }

    bool HitTest( const wxPoint& aPosition, int aAccuracy ) const;

    bool HitTest( const EDA_RECT& aRect, bool aContained = false,
                          int aAccuracy = 0 ) const;
    void Plot( PLOTTER* aPlotter );

    EDA_ITEM* Clone() const;

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const;     // override
#endif

private:
    bool doIsConnected( const wxPoint& aPosition ) const;
};


#endif    // _SCH_JUNCTION_H_
