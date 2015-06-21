#include <iostream>
#include <string>
#include <comdef.h>
#include "NDISDriverInst.h"


#if !defined(S_OK)
#define S_OK (0L)
#endif


using namespace std;


int main()
{
    long error_code = 0;
    int needreboot = 0;
    int query_installed = IsNDISDriverInstalled(L"ms_passthru", &error_code);
    if (!query_installed)
    {
        if (S_OK != error_code)
        {
            _com_error e(error_code);
            std::wstring error_msg = e.ErrorMessage();
        }
        else
        {
            cout << "not installed, install it" <<endl;

            error_code = 0;
            int install_ok = InstallNDISDriver(L"C:\\netsf.inf", &needreboot, &error_code);
            if (!install_ok)
            {
                cout << "install fail" << endl;
                if (S_OK != error_code)
                {
                    _com_error e(error_code);
                    std::wstring error_msg = e.ErrorMessage();
                }
            }
            else
            {
                cout << "install success" << endl;
            }
        }
    }
    else
    {
        cout << "have installed, uninstall it" <<endl;

        error_code = 0;
        int uninstall_ok = UninstallNDISDriver(L"ms_passthru", &needreboot, &error_code);
        if (!uninstall_ok)
        {
            cout << "uninstall fail" << endl;
            if (S_OK != error_code)
            {
                _com_error e(error_code);
                std::wstring error_msg = e.ErrorMessage();
            }
        }
        else
        {
            cout << "uninstall success" << endl;
        }
    }

    return 0;
}

