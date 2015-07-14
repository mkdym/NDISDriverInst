#include <ctype.h>
#include "Component.h"
#include "NDISDriverInst.h"


typedef struct tagNDISDriverInfo
{
    LPWSTR lpszItemName;
    LPWSTR lpszId;
    BOOL bEnabled;

} NDISDriverInfo, *PNDISDriverInfo;

typedef struct tagInfoNode
{
    NDISDriverInfo info;
    tagInfoNode *pNext;

} InfoNode, *PInfoNode;


BOOL GetComponentInfo(INetCfgComponent *pncc, NDISDriverInfo& info, HRESULT *pHR)
{
    HRESULT hr = pncc->GetDisplayName(&info.lpszItemName);
    if (S_OK == hr)
    {
        hr = pncc->GetId(&info.lpszId);
        if (S_OK == hr)
        {
            // If it is a network adapter then, find out if it enabled/disabled.
            GUID guidClass = {0};
            hr = pncc->GetClassGuid(&guidClass);
            BOOL bEnabled = FALSE;
            if (S_OK == hr)
            {
                if (IsEqualGUID(guidClass, GUID_DEVCLASS_NET))
                {
                    ULONG ulStatus = 0;
                    hr = pncc->GetDeviceStatus(&ulStatus);
                    bEnabled = (0 == ulStatus);
                }
                else
                {
                    bEnabled = TRUE;
                }
            }
            else
            {
                // We can't get the status, so assume that it is disabled.
                bEnabled = FALSE;
            }
            info.bEnabled = bEnabled;
        }
        else
        {
            info.lpszId = NULL;
        }
    }
    else
    {
        info.lpszItemName = NULL;
    }

    if (pHR)
    {
        *pHR = hr;
    }
    return S_OK == hr;
}

void FreeComponentInfo(NDISDriverInfo& info)
{
    if (info.lpszItemName)
    {
        CoTaskMemFree(info.lpszItemName);
        info.lpszItemName = NULL;
    }
    if (info.lpszId)
    {
        CoTaskMemFree(info.lpszId);
        info.lpszId = NULL;
    }
}

HRESULT EnumNDISDrivers(const GUID * pGUIDClass, InfoNode *pInfoListHeader)
{
    if (NULL == pInfoListHeader)
    {
        return S_FALSE;
    }
    InfoNode *pCurNode = pInfoListHeader;

    INetCfg *pnc = NULL;
    LPWSTR lpszApp = NULL;
    HRESULT hr = HrGetINetCfg(FALSE, APP_NAME, &pnc, &lpszApp);
    if (S_OK == hr)
    {
        // Get Component Enumerator Interface
        IEnumNetCfgComponent *pencc = NULL;
        hr = HrGetComponentEnum(pnc, pGUIDClass, &pencc);
        if (S_OK == hr)
        {
            INetCfgComponent *pncc = NULL;
            hr = HrGetFirstComponent(pencc, &pncc);
            while (S_OK == hr)
            {
                InfoNode *pNode = new InfoNode;
                memset(pNode, 0, sizeof(InfoNode));
                if (GetComponentInfo(pncc, pNode->info, &hr))
                {
                    pCurNode->pNext = pNode;
                    pCurNode = pCurNode->pNext;
                }
                ReleaseRef(pncc);
                hr = HrGetNextComponent(pencc, &pncc);
            }

            // S_FALSE merely indicates that there are no more components
            if (S_FALSE == hr)
            {
                hr = S_OK;
            }
            ReleaseRef( pencc );
        }
        HrReleaseINetCfg(pnc, FALSE);
    }
    else
    {
        if ((NETCFG_E_NO_WRITE_LOCK == hr) && lpszApp)
        {
            CoTaskMemFree(lpszApp);
        }
    }

    return hr;
}

void FreeEnumInfos(InfoNode *pInfoListHeader)
{
    if (NULL == pInfoListHeader)
    {
        return;
    }

    InfoNode *pCurNode = pInfoListHeader->pNext;
    while (pCurNode)
    {
        InfoNode *pNextNode = pCurNode->pNext;
        FreeComponentInfo(pCurNode->info);
        delete pCurNode;
        pCurNode = NULL;
        pCurNode = pNextNode;
    }
    memset(pInfoListHeader, 0, sizeof(InfoNode));
}

const GUID * DevClass2GUID(const NDIS_DEV_CLASS DevCls)
{
    switch (DevCls)
    {
    case DEV_NETCLIENT:
        return &GUID_DEVCLASS_NETCLIENT;
        break;

    case DEV_NETSERVICE:
        return &GUID_DEVCLASS_NETSERVICE;
        break;

    case DEV_NETTRANS:
        return &GUID_DEVCLASS_NETTRANS;
        break;

    default:
        return NULL;
        break;
    }
}



NDIS_DRIVER_INST_API NDIS_INST_STATE __stdcall IsNDISDriverInstalled(const NDIS_DEV_CLASS DevCls, const wchar_t *szComponentId, HRESULT *pResult)
{
    NDIS_INST_STATE inst_state = NDIS_NOT_INSTALLED;

    InfoNode InfoHeader = {0};
    HRESULT hr = EnumNDISDrivers(DevClass2GUID(DevCls), &InfoHeader);
    if (S_OK == hr)
    {
        InfoNode *pCurInfo = InfoHeader.pNext;
        while (pCurInfo)
        {
            if (0 == lstrcmpiW(szComponentId, pCurInfo->info.lpszId))
            {
                inst_state = NDIS_INSTALLED;
                break;
            }
            pCurInfo = pCurInfo->pNext;
        }
    }
    else
    {
        inst_state = NDIS_QUERY_ERROR;
        if (pResult)
        {
            *pResult = hr;
        }
    }
    FreeEnumInfos(&InfoHeader);

    return inst_state;
}

NDIS_DRIVER_INST_API HRESULT __stdcall InstallNDISDriver(const NDIS_DEV_CLASS DevCls, const wchar_t *szInfFile, int *pNeedReboot)
{
    WCHAR *szPnpID = NULL;
    HRESULT hr = GetPnpID(szInfFile, &szPnpID);
    if (S_OK == hr)
    {
        hr = InstallSpecifiedComponent(szInfFile, szPnpID, DevClass2GUID(DevCls));
        CoTaskMemFree(szPnpID);
        switch (hr)
        {
        case S_OK:
            if (pNeedReboot)
            {
                *pNeedReboot = FALSE;
            }
            break;

        case NETCFG_S_REBOOT:
            hr = S_OK;
            if (pNeedReboot)
            {
                *pNeedReboot = TRUE;
            }
            break;

        default:
            break;
        }
    }

    return hr;
}

NDIS_DRIVER_INST_API HRESULT __stdcall UninstallNDISDriver(const wchar_t *szComponentId, int *pNeedReboot)
{
    HRESULT hr = UninstallComponent(szComponentId);
    switch (hr)
    {
    case S_OK:
        if (pNeedReboot)
        {
            *pNeedReboot = FALSE;
        }
        break;

    case NETCFG_S_REBOOT:
        hr = S_OK;
        if (pNeedReboot)
        {
            *pNeedReboot = TRUE;
        }
        break;

    default:
        break;
    }

    return hr;
}

