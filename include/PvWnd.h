// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PV_WND_H__
#define __PV_WND_H__

#include <PvGUILib.h>


class Wnd;


class PV_GUI_API PvWnd
{
public:

	void SetPosition( int32_t  aPosX, int32_t  aPosY, int32_t  aSizeX, int32_t  aSizeY );
	void GetPosition( int32_t &aPosX, int32_t &aPosY, int32_t &aSizeX, int32_t &aSizeY );

	PvResult ShowModal( PvWindowHandle aParentHwnd = 0 );
	PvResult ShowModeless( PvWindowHandle aParentHwnd = 0 );
	PvResult Create( PvWindowHandle aHwnd, uint32_t aID );

	PvString GetTitle() const;
	void SetTitle( const PvString &aTitle );

	PvResult Close();

	PvWindowHandle GetHandle();
    PvResult DoEvents();

protected:

    PvWnd();
	virtual ~PvWnd();

    Wnd *mThis;

private:

    // Not implemented
	PvWnd( const PvWnd & );
	const PvWnd &operator=( const PvWnd & );

};


#endif // __PV_WND_H__

