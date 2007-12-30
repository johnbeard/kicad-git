/**********************************/
/* classes to handle copper zones */
/**********************************/

#ifndef CLASS_ZONE_H
#define CLASS_ZONE_H

#include "PolyLine.h"

/************************/
/* class ZONE_CONTAINER */
/************************/
/* handle a list of polygons delimiting a copper zone
 * a zone is described by a main polygon, a time stamp, a layer and a net name.
 * others polygons inside this main polygon are holes.
*/

class ZONE_CONTAINER : public BOARD_ITEM, public CPolyLine
{
public:
	enum m_PadInZone {			// How pads are covered by copper in zone
		PAD_NOT_IN_ZONE,		// Pads are not covered
		THERMAL_PAD,			// Use thermal relief for pads
		PAD_IN_ZONE				// pads are covered by copper
	};
    wxString m_Netname;         // Net Name
	int m_CornerSelection;      // For corner moving, corner index to drag, or -1 if no selection
	int m_ZoneClearance;		// clearance value
	int m_GridFillValue;		// Grid used for filling
	m_PadInZone m_PadOption;	// see m_PadInZone

private:
    int     m_NetCode;          // Net number for fast comparisons

public:
	ZONE_CONTAINER(BOARD * parent);
	~ZONE_CONTAINER();

    bool Save( FILE* aFile ) const;
    int  ReadDescr( FILE* aFile, int* aLineNum = NULL );

	wxPoint & GetPosition( ) { static wxPoint pos ;return pos; }
	void UnLink(void) {};

	void Display_Infos( WinEDA_DrawFrame* frame );

	/** Function Draw
	* Draws the zone outline.
	* @param panel = current Draw Panel
	* @param DC = current Device Context
	* @param offset = Draw offset (usually wxPoint(0,0))
	* @param draw_mode = draw mode: OR, XOR ..
	*/
	void Draw( WinEDA_DrawPanel* panel, wxDC* DC,
                   const wxPoint& offset, int draw_mode );
	
	int GetNet( void ) { return m_NetCode; }
	void SetNet( int anet_code ) { m_NetCode = anet_code; }
	/**
	 * Function HitTest
	 * tests if the given wxPoint is within the bounds of this object.
	 * @param refPos A wxPoint to test
	 * @return bool - true if a hit, else false
	 */
	bool HitTest( const wxPoint& refPos );

	/**
	 * Function HitTestForCorner
	 * tests if the given wxPoint near a corner, or near the segment define by 2 corners.
	 * @return -1 if none, corner index in .corner <vector>
	 * @param refPos : A wxPoint to test
	 */
	int HitTestForCorner( const wxPoint& refPos );
	/**
	 * Function HitTestForEdge
	 * tests if the given wxPoint near a corner, or near the segment define by 2 corners.
	 * @return -1 if none,  or index of the starting corner in .corner <vector>
	 * @param refPos : A wxPoint to test
	 */
	int HitTestForEdge( const wxPoint& refPos );
	
	/** Function Fill_Zone()
	 *  Calculate the zone filling
	 *  The zone outline is a frontier, and can be complex (with holes)
	 *  The filling starts from starting points like pads, tracks.
	 * If exists the old filling is removed
	 * @param frame = reference to the main frame
	 * @param DC = current Device Context
	 * @param verbose = true to show error messages
	 * @return error level (0 = no error)
	 */
	int Fill_Zone( WinEDA_PcbFrame* frame, wxDC* DC, bool verbose = TRUE);

};

/*******************/
/* class EDGE_ZONE */
/*******************/

class EDGE_ZONE : public DRAWSEGMENT
{
public:
    EDGE_ZONE( BOARD_ITEM* StructFather );
    EDGE_ZONE( const EDGE_ZONE& edgezone );
    ~EDGE_ZONE();

    EDGE_ZONE* Next() { return (EDGE_ZONE*) Pnext; }

    EDGE_ZONE* Back() { return (EDGE_ZONE*) Pback; }
    
    
    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.brd" format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */ 
    bool Save( FILE* aFile ) const;
};

#endif	// #ifndef CLASS_ZONE_H
