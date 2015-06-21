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

BOOL EnumNDISDrivers(InfoNode *pInfoListHeader, HRESULT *pHR)
{
    if (NULL == pInfoListHeader)
    {
        return FALSE;
    }
    InfoNode *pCurNode = pInfoListHeader;

    INetCfg *pnc = NULL;
    LPWSTR lpszApp = NULL;
    HRESULT hr = HrGetINetCfg(FALSE, APP_NAME, &pnc, &lpszApp);
    if (S_OK == hr)
    {
        // Get Component Enumerator Interface
        IEnumNetCfgComponent *pencc = NULL;
        hr = HrGetComponentEnum(pnc, &GUID_DEVCLASS_NETSERVICE, &pencc);
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

    if (pHR)
    {
        *pHR = hr;
    }
    return S_OK == hr;
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

NDIS_DRIVER_INST_API int __stdcall IsNDISDriverInstalled(const wchar_t *szComponentId, long *pResult)
{
    BOOL bReturn = FALSE;

    InfoNode InfoHeader = {0};
    if (EnumNDISDrivers(&InfoHeader, pResult))
    {
        InfoNode *pCurInfo = InfoHeader.pNext;
        while (pCurInfo)
        {
            if (0 == lstrcmpiW(szComponentId, pCurInfo->info.lpszId))
            {
                bReturn = TRUE;
                break;
            }
            pCurInfo = pCurInfo->pNext;
        }
    }
    FreeEnumInfos(&InfoHeader);

    return bReturn;
}

NDIS_DRIVER_INST_API int __stdcall InstallNDISDriver(const wchar_t *szInfFile, int *pNeedReboot, long *pResult)
{
    BOOL bReturn = FALSE;

    WCHAR *szPnpID = NULL;
    HRESULT hr = GetPnpID(szInfFile, &szPnpID);
    if (S_OK == hr)
    {
        hr = InstallSpecifiedComponent(szInfFile, szPnpID, &GUID_DEVCLASS_NETSERVICE);
        CoTaskMemFree(szPnpID);

        BOOL bReturn = FALSE;
        switch (hr)
        {
        case S_OK:
            bReturn = TRUE;
            if (pNeedReboot)
            {
                *pNeedReboot = FALSE;
            }
            break;

        case NETCFG_S_REBOOT:
            bReturn = TRUE;
            if (pNeedReboot)
            {
                *pNeedReboot = TRUE;;
            }
            break;

        default:
            break;
        }
    }

    if (pResult)
    {
        *pResult = hr;
    }
    return bReturn;
}

NDIS_DRIVER_INST_API int __stdcall UninstallNDISDriver(const wchar_t *szComponentId, int *pNeedReboot, long *pResult)
{
    HRESULT hr = UninstallComponent(szComponentId);

    BOOL bReturn = FALSE;
    switch (hr)
    {
    case S_OK:
        bReturn = TRUE;
        if (pNeedReboot)
        {
            *pNeedReboot = FALSE;
        }
        break;

    case NETCFG_S_REBOOT:
        bReturn = TRUE;
        if (pNeedReboot)
        {
            *pNeedReboot = TRUE;;
        }
        break;

    default:
        break;
    }

    if (pResult)
    {
        *pResult = hr;
    }
    return bReturn;
}

