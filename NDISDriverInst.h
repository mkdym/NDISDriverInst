//使用时，默认链接静态库，若要链接动态库，请在包含此头文件前定义宏NDIS_INST_IMPORT_DYN
#ifndef _NDIS_DRIVER_INST_H_INCLUDED
#define _NDIS_DRIVER_INST_H_INCLUDED


#if defined(_NDIS_INST_EXPORTS_STATIC)//编译静态库版本
#define NDIS_DRIVER_INST_API

#elif defined(_NDIS_INST_EXPORTS_DYN)//编译动态库版本
#define NDIS_DRIVER_INST_API _declspec(dllexport)

#elif defined(NDIS_INST_IMPORT_DYN)//使用动态库版本
#define NDIS_DRIVER_INST_API _declspec(dllimport)
#if defined(_DEBUG) || defined(DEBUG)
#pragma comment(lib, "NDISDriverInst_d.lib")
#else
#pragma comment(lib, "NDISDriverInst.lib")
#endif

#else//使用静态库版本(默认)
#define NDIS_DRIVER_INST_API
#pragma comment(lib, "Setupapi.lib")
#if defined(_DEBUG) || defined(DEBUG)
#pragma comment(lib, "NDISDriverInstLib_d.lib")
#else
#pragma comment(lib, "NDISDriverInstLib.lib")
#endif

#endif


#ifdef __cplusplus
extern "C"
{
#endif

    /*
    说明：pResult是HRESULT的指针
        HRESULT包含的具体错误信息可以使用_com_error类来获取
        或者自行调用FormatMessage获取
    */

    //************************************
    // brief:    判断指定驱动是否已安装
    // name:     IsNDISDriverInstalled
    // param:    const wchar_t * szComponentId          组件ID，INF文件中指定的名称。比如WDK自带的passthru的netsf.inf中指定的是ms_passthru
    // param:    long * pResult                         错误码，错误码的使用见示例
    // return:   NDIS_DRIVER_INST_API int __stdcall     若已安装，则返回非0，否则，返回0
    // ps:       当返回0，即未安装时，还应判断pResult是否包含错误。因为有可能函数执行失败，并不是真的未安装
    //************************************
    NDIS_DRIVER_INST_API int __stdcall IsNDISDriverInstalled(const wchar_t *szComponentId, long *pResult);

    //************************************
    // brief:    安装/卸载指定驱动
    // name:     InstallNDISDriver/UninstallNDISDriver
    // param:    const wchar_t * szInfFile              用于安装驱动的INF文件路径，比如WDK自带的passthru的netsf.inf
    // param:    const wchar_t * szComponentId          参见上面
    // param:    int * pNeedReboot                      函数返回后，若此值为非0，则表示需要重启，否则不需重启
    // param:    long * pResult                         参见上面
    // return:   NDIS_DRIVER_INST_API int __stdcall     若安装/卸载成功，则返回非0，否则，返回0
    // ps:       当返回0，即安装/卸载失败时，还应判断pResult是否包含错误。因为有可能函数执行失败，并不是真的失败
    //           若返回失败，也有可能已经安装/卸载成功
    //************************************
    NDIS_DRIVER_INST_API int __stdcall InstallNDISDriver(const wchar_t *szInfFile, int *pNeedReboot, long *pResult);
    NDIS_DRIVER_INST_API int __stdcall UninstallNDISDriver(const wchar_t *szComponentId, int *pNeedReboot, long *pResult);


#ifdef __cplusplus
}
#endif


#endif
