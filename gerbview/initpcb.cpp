/****************************************/
/******* initpcb.cpp ********************/
/****************************************/


#include "fctsys.h"
#include "common.h"
#include "class_drawpanel.h"
#include "confirm.h"

#include "gerbview.h"
#include "protos.h"


bool WinEDA_GerberFrame::Clear_Pcb( bool query )
{
    int layer;

    if( GetBoard() == NULL )
        return FALSE;

    if( query )
    {
        if( GetBoard()->m_Drawings || GetBoard()->m_Track
            || GetBoard()->m_Zone )
        {
            if( !IsOK( this, _( "Current data will be lost?" ) ) )
                return FALSE;
        }
    }

    GetBoard()->m_Drawings.DeleteAll();

    GetBoard()->m_Track.DeleteAll();

    GetBoard()->m_Zone.DeleteAll();

    for( layer = 0; layer < 32; layer++ )
    {
        if( g_GERBER_List[layer] )
            g_GERBER_List[layer]->InitToolTable();
    }

    GetBoard()->m_BoundaryBox.SetOrigin( 0, 0 );
    GetBoard()->m_BoundaryBox.SetSize( 0, 0 );
    GetBoard()->m_Status_Pcb  = 0;
    GetBoard()->m_NbNodes     = 0;
    GetBoard()->m_NbNoconnect = 0;

    SetBaseScreen( ActiveScreen = ScreenPcb );
    GetScreen()->Init();

    return TRUE;
}


void WinEDA_GerberFrame::Erase_Zones( bool query )
{
    if( query && !IsOK( this, _( "Delete zones?" ) ) )
        return;

    GetBoard()->m_Zone.DeleteAll();

    ScreenPcb->SetModify();
}


void WinEDA_GerberFrame::Erase_Segments_Pcb( bool all_layers, bool query )
{
    int layer = GetScreen()->m_Active_Layer;

    if( all_layers )
        layer = -1;

    BOARD_ITEM* next;
    for( BOARD_ITEM* item = GetBoard()->m_Drawings; item; item = next )
    {
        next = item->Next();

        switch( item->Type() )
        {
        case TYPE_DRAWSEGMENT:
        case TYPE_TEXTE:
        case TYPE_COTATION:
        case TYPE_MIRE:
            if( item->GetLayer() == layer  || layer < 0 )
                GetBoard()->Delete( item );
            break;

        default:
            DisplayError( this, wxT( "Draw type unknown." ) );
            break;
        }
    }

    ScreenPcb->SetModify();
}


/* Delete track segments.
 * masque_type mask options selection:
 *   SEGM_FIXE, SEGM_AR
 * If a bit is 1, segment is not erased.
 */
void WinEDA_GerberFrame::Erase_Pistes( int masque_type, bool query )
{
    TRACK*      pt_segm;
    BOARD_ITEM* PtNext;

    if( query && !IsOK( this, _( "Delete tracks?" ) ) )
        return;

    /* Mark tracks to clear. */
    for( pt_segm = GetBoard()->m_Track;
         pt_segm != NULL;
         pt_segm = (TRACK*) PtNext )
    {
        PtNext = pt_segm->Next();
        if( pt_segm->GetState( SEGM_FIXE | SEGM_AR ) & masque_type )
            continue;
        pt_segm->DeleteStructure();
    }

    ScreenPcb->SetModify();
}


void WinEDA_GerberFrame::Erase_Textes_Pcb( bool query )
{
    BOARD_ITEM* PtStruct;
    BOARD_ITEM* PtNext;

    if( query && !IsOK( this, _( "Delete pcb text?" ) ) )
        return;

    PtStruct = GetBoard()->m_Drawings;
    for( ; PtStruct != NULL; PtStruct = PtNext )
    {
        PtNext = PtStruct->Next();
        if( PtStruct->Type() == TYPE_TEXTE )
            PtStruct->DeleteStructure();
    }

    ScreenPcb->SetModify();
}


void WinEDA_GerberFrame::Erase_Current_Layer( bool query )
{
    int      layer = GetScreen()->m_Active_Layer;
    wxString msg;

    msg.Printf( _( "Delete layer %d?" ), layer + 1 );
    if( query && !IsOK( this, msg ) )
        return;

    /* Delete tracks (spots and lines) */
    TRACK* PtNext;
    for( TRACK* pt_segm = GetBoard()->m_Track;
         pt_segm != NULL;
         pt_segm = (TRACK*) PtNext )
    {
        PtNext = pt_segm->Next();
        if( pt_segm->GetLayer() != layer )
            continue;
        pt_segm->DeleteStructure();
    }

    /* Delete polygons */
    SEGZONE* Nextzone;
    for( SEGZONE* zone = GetBoard()->m_Zone; zone != NULL; zone = Nextzone )
    {
        Nextzone = zone->Next();
        if( zone->GetLayer() != layer )
            continue;
        zone->DeleteStructure();
    }

    ScreenPcb->SetModify();
    ScreenPcb->SetRefreshReq();
}
