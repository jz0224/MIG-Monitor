// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDISPLAY_H__
#define __PVDISPLAY_H__


#include <PvGUILib.h>
#include <PvWnd.h>
#include <PvBuffer.h>
#include <PvBufferConverter.h>


class DisplayWnd;


class PV_GUI_API PvDisplayWnd : public PvWnd
{
public:

	PvDisplayWnd();
	virtual ~PvDisplayWnd();

	PvResult Display( const PvBuffer &aBuffer, bool aVSync = false );
	PvResult SetBackgroundColor( uint8_t aR, uint8_t aG, uint8_t aB );

    PvBufferConverter &GetConverter();

	void Clear();

    void SetTextOverlay( const PvString &aText );
    void SetTextOverlayColor( uint8_t aR, uint8_t aG, uint8_t aB );
    PvResult SetTextOverlaySize( int32_t aSize );
    void SetTextOverlayOffsetX( uint32_t aX );
    void SetTextOverlayOffsetY( uint32_t aY );

    PvString GetTextOverlay() const;
    void GetTextOverlayColor( uint8_t &aR, uint8_t &aG, uint8_t &aB ) const;
    int32_t GetTextOverlaySize() const;
    int32_t GetTextOverlayOffsetX() const;
    int32_t GetTextOverlayOffsetY() const;

protected:

private:

    // Not implemented
	PvDisplayWnd( const PvDisplayWnd & );
	const PvDisplayWnd &operator=( const PvDisplayWnd & );

};


#endif // __PVDISPLAY_H__
