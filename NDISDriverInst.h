//使用时，默认链接静态库，若要链接动态库，请在包含此头文件前定义宏NDIS_INST_IMPORT_DYN
#ifndef _NDIS_DRIVER_INST_H_INCLUDED
#define _NDIS_DRIVER_INST_H_INCLUDED



#if defined(_NDIS_INST_EXPORTS_STATIC)//编译静态库
#define NDIS_DRIVER_INST_API

#elif defined(_NDIS_INST_EXPORTS_DYN)//编译动态库
#define NDIS_DRIVER_INST_API _declspec(dllexport)

#elif defined(NDIS_INST_IMPORT_DYN)//使用动态库
#define NDIS_DRIVER_INST_API _declspec(dllimport)
#if defined(_DEBUG) || defined(DEBUG)
#pragma comment(lib, "NDISDriverInst_d.lib")
#else
#pragma comment(lib, "NDISDriverInst.lib")
#endif

#else//使用静态库(默认)
#define NDIS_DRIVER_INST_API
#pragma comment(lib, "Setupapi.lib")
#if defined(_DEBUG) || defined(DEBUG)
#pragma comment(lib, "NDISDriverInstLib_d.lib")
#else//if defined(_DEBUG) || defined(DEBUG)
#pragma comment(lib, "NDISDriverInstLib.lib")
#endif//if defined(_DEBUG) || defined(DEBUG)

#endif//if defined(_NDIS_INST_EXPORTS_STATIC)



#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef __success(return >= 0) long HRESULT;
#endif // !_HRESULT_DEFINED


#if !defined(S_OK)
#define S_OK (0L)
#endif



#ifdef __cplusplus
extern "C"
{
#endif

    /*
    说明：HRESULT包含的具体错误信息可以使用_com_error类来获取
        或者自行调用FormatMessage获取
        简单的，S_OK表示成功，否则表示错误
    */

    enum NDIS_INST_STATE
    {
        NDIS_INSTALLED,
        NDIS_NOT_INSTALLED,
        NDIS_QUERY_ERROR,
    };

    //驱动类型
    enum NDIS_DEV_CLASS
    {
        DEV_NETCLIENT,          //客户端
        DEV_NETSERVICE,         //服务
        DEV_NETTRANS,           //协议
    };

    /************************************************************************
    brief:          判断指定驱动是否已安装
    param:          DevCls                              驱动类型
    param:          szComponentId                       组件ID，INF文件中指定的名称。比如WDK自带的passthru的netsf.inf中指定的是ms_passthru
    param:          pResult                             错误码。错误码的使用见示例
    return:         NDIS_DRIVER_INST_API NDIS_INST_STATE __stdcall
    history:        2015/07/13
    remarks:
    ************************************************************************/
    NDIS_DRIVER_INST_API NDIS_INST_STATE __stdcall IsNDISDriverInstalled(const NDIS_DEV_CLASS DevCls, const wchar_t *szComponentId, HRESULT *pResult);


    /************************************************************************
    brief:          安装/卸载指定驱动
    param:          DevCls                              参见IsNDISDriverInstalled
    param:          szInfFile                           用于安装驱动的INF文件路径，比如WDK自带的passthru的netsf.inf
    param:          szComponentId                       参见IsNDISDriverInstalled
    param:          pNeedReboot                         函数成功返回后，若此值为非0，则表示需要重启，否则不需重启
    return:         NDIS_DRIVER_INST_API HRESULT __stdcall
    history:        2015/07/13
    remarks:
    ************************************************************************/
    NDIS_DRIVER_INST_API HRESULT __stdcall InstallNDISDriver(const NDIS_DEV_CLASS DevCls, const wchar_t *szInfFile, int *pNeedReboot);
    NDIS_DRIVER_INST_API HRESULT __stdcall UninstallNDISDriver(const wchar_t *szComponentId, int *pNeedReboot);


#ifdef __cplusplus
}
#endif


#endif
