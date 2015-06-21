//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2001.
//
//  File:       C O M P O N E N T . C P P
//
//  Contents:   Functions to illustrate
//              o How to enumerate network components.
//              o How to install protocols, clients and services.
//              o How to uninstall protocols, clients and services.
//              o How to bind/unbind network components.
//
//  Notes:
//
//  Author:     Alok Sinha    15-May-01
//
//----------------------------------------------------------------------------
#include "Component.h"


HRESULT InstallSpecifiedComponent ( __in LPCWSTR lpszInfFile,
                                   __in LPCWSTR lpszPnpID,
                                   const GUID *pguidClass)
{
    INetCfg    *pnc;
    LPWSTR     lpszApp;
    HRESULT    hr;

    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

        //
        // Install the network component.
        //

        hr = HrInstallNetComponent( pnc,
                                    lpszPnpID,
                                    pguidClass,
                                    lpszInfFile );
        if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) ) {

            hr = pnc->Apply();
        }
        else {
            if ( hr != HRESULT_FROM_WIN32(ERROR_CANCELLED) ) {
                ErrMsg( hr,
                        L"Couldn't install the network component." );
            }
        }

        HrReleaseINetCfg( pnc,
                          TRUE );
    }
    else {
        if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp ) {
            ErrMsg( hr,
                    L"%s currently holds the lock, try later.",
                    lpszApp );

            CoTaskMemFree( lpszApp );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't the get notify object interface." );
        }
    }

    return hr;
}

HRESULT UninstallComponent ( __in LPCWSTR lpszInfId)
{
    INetCfg              *pnc;
    INetCfgComponent     *pncc;
    INetCfgClass         *pncClass;
    INetCfgClassSetup    *pncClassSetup;
    LPWSTR               lpszApp;
    GUID                 guidClass;
    OBO_TOKEN            obo;
    HRESULT              hr;

    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

        //
        // Get a reference to the network component to uninstall.
        //

        hr = pnc->FindComponent( lpszInfId,
                                 &pncc );

        if ( hr == S_OK ) {

            //
            // Get the class GUID.
            //

            hr = pncc->GetClassGuid( &guidClass );

            if ( hr == S_OK ) {

                //
                // Get a reference to component's class.
                //

                hr = pnc->QueryNetCfgClass( &guidClass,
                                            IID_INetCfgClass,
                                            (PVOID *)&pncClass );
                if ( hr == S_OK ) {

                    //
                    // Get the setup interface.
                    //

                    hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                                   (LPVOID *)&pncClassSetup );

                    if ( hr == S_OK ) {

                        //
                        // Uninstall the component.
                        //

                        ZeroMemory( &obo,
                                    sizeof(OBO_TOKEN) );

                        obo.Type = OBO_USER;

                        hr = pncClassSetup->DeInstall( pncc,
                                                       &obo,
                                                       NULL );
                        if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) ) {

                            hr = pnc->Apply();

                            if ( (hr != S_OK) && (hr != NETCFG_S_REBOOT) ) {
                                ErrMsg( hr,
                                        L"Couldn't apply the changes after"
                                        L" uninstalling %s.",
                                        lpszInfId );
                            }
                        }
                        else {
                            ErrMsg( hr,
                                    L"Failed to uninstall %s.",
                                    lpszInfId );
                        }

                        ReleaseRef( pncClassSetup );
                    }
                    else {
                        ErrMsg( hr,
                                L"Couldn't get an interface to setup class." );
                    }

                    ReleaseRef( pncClass );
                }
                else {
                    ErrMsg( hr,
                            L"Couldn't get a pointer to class interface "
                            L"of %s.",
                            lpszInfId );
                }
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get the class guid of %s.",
                        lpszInfId );
            }

            ReleaseRef( pncc );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get an interface pointer to %s.",
                    lpszInfId );
        }

        HrReleaseINetCfg( pnc,
                          TRUE );
    }
    else {
        if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp ) {
            ErrMsg( hr,
                    L"%s currently holds the lock, try later.",
                    lpszApp );

            CoTaskMemFree( lpszApp );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the notify object interface." );
        }
    }

    return hr;
}


//
// Function:  GetKeyValue
//
// Purpose:   Retrieve the value of a key from the inf file.
//
// Arguments:
//    hInf        [in]  Inf file handle.
//    lpszSection [in]  Section name.
//    lpszKey     [in]  Key name.
//    dwIndex     [in]  Key index.
//    lppszValue  [out] Key value.
//
// Returns:   S_OK on success, otherwise and error code.
//
// Notes:
//

HRESULT
GetKeyValue (
             HINF hInf,
             __in LPCWSTR lpszSection,
             __in_opt LPCWSTR lpszKey,
             DWORD  dwIndex,
             __deref_out_opt LPWSTR *lppszValue)
{
    INFCONTEXT  infCtx;
    __range(0, 512) DWORD       dwSizeNeeded;
    HRESULT     hr;

    *lppszValue = NULL;

    if ( SetupFindFirstLineW(hInf,
        lpszSection,
        lpszKey,
        &infCtx) == FALSE )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if ( SetupGetStringFieldW(&infCtx,
        dwIndex,
        NULL,
        0,
        &dwSizeNeeded) )
    {
        *lppszValue = (LPWSTR)CoTaskMemAlloc( sizeof(WCHAR) * dwSizeNeeded );

        if ( !*lppszValue  )
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }

        if ( SetupGetStringFieldW(&infCtx,
            dwIndex,
            *lppszValue,
            dwSizeNeeded,
            NULL) == FALSE )
        {

            hr = HRESULT_FROM_WIN32(GetLastError());

            CoTaskMemFree( *lppszValue );
            *lppszValue = NULL;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}

//
// Function:  GetPnpID
//
// Purpose:   Retrieve PnpID from an inf file.
//
// Arguments:
//    lpszInfFile [in]  Inf file to search.
//    lppszPnpID  [out] PnpID found.
//
// Returns:   TRUE on success.
//
// Notes:
//

HRESULT
GetPnpID (
          __in LPCWSTR lpszInfFile,
          __deref_out_opt LPWSTR *lppszPnpID)
{
    HINF    hInf;
    LPWSTR  lpszModelSection;
    HRESULT hr;

    *lppszPnpID = NULL;

    hInf = SetupOpenInfFileW( lpszInfFile,
        NULL,
        INF_STYLE_WIN4,
        NULL );

    if ( hInf == INVALID_HANDLE_VALUE )
    {

        return HRESULT_FROM_WIN32(GetLastError());
    }

    //
    // Read the Model section name from Manufacturer section.
    //

    hr = GetKeyValue( hInf,
        L"Manufacturer",
        NULL,
        1,
        &lpszModelSection );

    if ( hr == S_OK )
    {

        //
        // Read PnpID from the Model section.
        //

        hr = GetKeyValue( hInf,
            lpszModelSection,
            NULL,
            2,
            lppszPnpID );

        CoTaskMemFree( lpszModelSection );
    }

    SetupCloseInfFile( hInf );

    return hr;
}
