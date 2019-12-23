// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Header file containing #include, #define and some inline code shared by
// all eBUS SDK samples.
//
// *****************************************************************************

#ifndef __PV_SAMPLEUTILS_H__
#define __PV_SAMPLEUTILS_H__


#ifdef WIN32

#ifdef _AFXDLL
    #include <afxwin.h>
#else
    #include <windows.h>
    //define PV_GUI_NOT_AVAILABLE
#endif // _AFXDLL
#include <process.h>
#include <conio.h>

#pragma comment(linker, "/manifestdependency:\"type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' " \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")

#endif // WIN32

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <PvTypes.h>
#include <PvDevice.h>
#include <PvSystem.h>
#include <PvDeviceGEV.h>

using namespace std;

#ifdef _UNIX_

#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#define PV_INIT_SIGNAL_HANDLER()                                    \
    void CatchCtrlC( int aSig ) { extern int gStop; gStop = 1; }    \
    int InitHandler() { signal( SIGINT, CatchCtrlC ); return 1; }   \
    int gInit = InitHandler();                                      \
    int gStop = 0;                                                  \

#define PV_SAMPLE_INIT()
#define PV_SAMPLE_TERMINATE()

inline int PvKbHit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    extern int gStop;

    if( gStop )
    {
        return 1;
    }

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

inline int PvGetChar()
{
    extern int gStop;
    if( gStop )
    {
        return 0;
    }

    return getchar();
}

inline void PvWaitForKeyPress()
{
    while ( !PvKbHit() )
    {
        usleep( 10000 );
    }

    PvGetChar(); // Flush key buffer for next stop
}

inline uint64_t PvGetTickCountMs()
{
    timeval ts;
    gettimeofday( &ts, 0 );

    uint64_t lTickCount = (int64_t)(ts.tv_sec * 1000LL + ( ts.tv_usec / 1000LL ) );

    return lTickCount;
}

inline void PvSleepMs( uint32_t aSleepTime )
{
    usleep( aSleepTime * 1000 );
}

#define PvScanf ( scanf )

#endif // _UNIX_

#ifdef WIN32

#define PV_INIT_SIGNAL_HANDLER()   

// This macro creates the right context to properly use MFC dialogs from a command line application
#ifdef _AFXDLL
    #define PV_SAMPLE_INIT() \
        CWinApp lApp; \
        AfxWinInit( ::GetModuleHandle( NULL ), NULL, ::GetCommandLine(), 0 );
#else
    #define PV_SAMPLE_INIT()
#endif // _AFXDLL

// Cleans up the context set by PV_SAMPLE_INIT
#ifdef _AFXDLL
    #define PV_SAMPLE_TERMINATE() \
        AfxWinTerm();
#else
    #define PV_SAMPLE_TERMINATE()
#endif // _AFXDLL

inline int PvKbHit( void )
{
    return _kbhit();
}

inline int PvGetChar()
{
    return _getch();
}

inline void PvWaitForKeyPress()
{
    while ( !PvKbHit() )
    {
        ::Sleep( 10 );
    }

    PvGetChar(); // Flush key buffer for next stop.
}

inline uint64_t PvGetTickCountMs()
{
    return ::GetTickCount();
}

inline void PvSleepMs( uint32_t aSleepTime )
{
    ::Sleep( aSleepTime );
}

#define PvScanf ( scanf_s )

#endif // WIN32

inline void PvFlushKeyboard()
{
    int c;
    while( ( c = PvGetChar() ) != '\n' && c != EOF );
}


// These defines ensures no old/deprecated pixel types are used in the samples.
#ifndef PV_NO_GEV1X_PIXEL_TYPES
#define PV_NO_GEV1X_PIXEL_TYPES
#endif // PV_NO_GEV1X_PIXEL_TYPES
#ifndef PV_NO_DEPRECATED_PIXEL_TYPES
#define PV_NO_DEPRECATED_PIXEL_TYPES
#endif // PV_NO_DEPRECATED_PIXEL_TYPES


inline const PvDeviceInfo* PvSelectDevice( PvSystem& aSystem )
{
    PvResult lResult;
    const PvDeviceInfo *lSelectedDI = nullptr;
#ifdef _UNIX_
    extern int gStop;
#endif

    cout << endl << "Detecting devices." << endl;

    while( 1 )
    {

#ifdef _UNIX_
        if( gStop )
        { 
            return NULL;
        }
#endif
        aSystem.Find();

        // Detect, select device.
        vector<const PvDeviceInfo *> lDIVector;
        for ( uint32_t i = 0; i < aSystem.GetInterfaceCount(); i++ )
        {
            const PvInterface *lInterface = dynamic_cast<const PvInterface *>( aSystem.GetInterface( i ) );
            if ( lInterface != nullptr )
            {
                cout << "   " << lInterface->GetDisplayID().GetAscii() << endl;
                for ( uint32_t j = 0; j < lInterface->GetDeviceCount(); j++ )
                {
                    const PvDeviceInfo *lDI = dynamic_cast<const PvDeviceInfo *>( lInterface->GetDeviceInfo( j ) );
                    if ( lDI != nullptr )
                    {
                        lDIVector.push_back( lDI );
                        cout << "[" << ( lDIVector.size() - 1 ) << "]" << "\t" << lDI->GetDisplayID().GetAscii() << endl;
                    }					
                }
            }
        }

        if( lDIVector.size() == 0)
        {
            cout << "No device found!" << endl;
        }

        cout << "[" << lDIVector.size() << "] to abort" << endl;
        cout << "[" << ( lDIVector.size() + 1 ) << "] to search again" << endl << endl;

        cout << "Enter your action or device selection?" << endl;
        cout << ">";

        // Read device selection, optional new IP address.
        uint32_t lIndex = 0;
        cin >> lIndex;
        if( lIndex == lDIVector.size() )
        {
            // We abort the selection process.
            return nullptr;
        }
        else if ( ( lIndex >=0 ) && ( lIndex < lDIVector.size() ) )
        {
            // The device is selected
            lSelectedDI = lDIVector[ lIndex ];
            break;
        }
    }

    // If the IP Address valid?
    if ( lSelectedDI->IsConfigurationValid() )
    {
        cout << endl;
        return lSelectedDI;
    }

    // Ask the user for a new IP address.
    cout << "The IP configuration of the device is not valid." << endl;
    cout << "Which IP address should be assigned to the device?" << endl;
    cout << ">";

    // Read new IP address.
    string lNewIPAddress;
    cin >> lNewIPAddress;
    if ( lNewIPAddress.length() )
    {
        return nullptr;
    }

    const PvDeviceInfoGEV* lDeviceGEV = dynamic_cast<const PvDeviceInfoGEV *>( lSelectedDI );
    if ( lDeviceGEV != nullptr )
    {
        // Force new IP address.
        lResult = PvDeviceGEV::SetIPConfiguration( lDeviceGEV->GetMACAddress().GetAscii(), lNewIPAddress.c_str(),
            lDeviceGEV->GetSubnetMask().GetAscii(), lDeviceGEV->GetDefaultGateway().GetAscii() );
        if ( !lResult.IsOK() )
        {
            cout << "Unable to force new IP address." << endl;
            return nullptr;
        }
    }	

    // Wait for the device to come back on the network.
    int lTimeout;
    while( 1 )
    {
#ifdef _UNIX_
        if( gStop )
        { 
            return NULL;
        }
#endif
        lTimeout = 10;
        while( lTimeout )
        {
#ifdef _UNIX_
            if( gStop )
            { 
                return NULL;
            }
#endif

            aSystem.Find();

            vector<const PvDeviceInfo *> lDIVector;
            for ( uint32_t i = 0; i < aSystem.GetInterfaceCount(); i++ )
            {
                const PvInterface *lInterface = aSystem.GetInterface( i );
                for ( uint32_t j = 0; j < lInterface->GetDeviceCount(); j++ )
                {
                    if ( lInterface->GetDeviceInfo( j )->GetType() == PvDeviceInfoTypeGEV )
                    {
                        const PvDeviceInfoGEV *lDI = dynamic_cast<const PvDeviceInfoGEV*>( lInterface->GetDeviceInfo( j ) );
                        if ( lDI != nullptr )
                        {
                            if( lNewIPAddress == lDI->GetIPAddress().GetAscii() )
                            {
                                cout << endl;
                                return lDI;
                            }
                        }
                    }
                }
            }
            PvSleepMs( 1000 );

            lTimeout--;
        }

        cout << "The device " << lNewIPAddress << " was not located. Do you want to continue waiting? yes or no" << endl;
        cout << ">";

        string lAnswer;
        cin >> lAnswer;
        if ( ( lAnswer == "n" ) || ( lAnswer == "no" ) )
        {
            break;
        }
    }

    cout << endl;
    return nullptr;
}

inline bool ParseOptionFlag( int aCount, const char ** aArgs, const char *aOption, bool *aValue = nullptr )
{
    string lOption = aOption;
    for ( int i = 1; i < aCount; i++ )
    {
        string lString = aArgs[i];
        size_t lPos = lString.find( aOption );
        if ( lPos != string::npos )
        {
            if ( aValue != nullptr )
            {
                *aValue = true;
            }

            return true;
        }
    }    

    return false;
}

template <class T>
inline bool ParseOption( int aCount, const char ** aArgs, const char *aOption, T &aValue )
{
    string lOption = aOption;
    lOption += "=";
    for ( int i = 1; i < aCount; i++ )
    {
        string lString = aArgs[i];

        size_t lPos = lString.find( aOption );
        if ( lPos != string::npos )
        {
            if ( lString.size() > lOption.size() )
            {
                string lParameter = lString.substr( lOption.size(), ( lString.size() - lOption.size() ) + 1 );
                istringstream iss( lParameter, istringstream::in );
                iss >> aValue;
                return true;
            }
        }
    }

    return false;
}

#endif // __PV_SAMPLEUTILS_H__

