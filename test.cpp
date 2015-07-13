#include <iostream>
#include <string>
#include <comdef.h>
#include "NDISDriverInst.h"



using namespace std;



void print_error(const HRESULT& hr)
{
    _com_error e(hr);
#if defined(UNICODE) || defined(_UNICODE)
    wcout << e.ErrorMessage() << endl;
#else
    cout << e.ErrorMessage() << endl;
#endif
}



int main()
{
    setlocale(LC_ALL,"chs");

    HRESULT error_code = S_OK;
    int needreboot = 0;
    NDIS_INST_STATE state = IsNDISDriverInstalled(L"ms_passthru", &error_code);
    switch (state)
    {
    case NDIS_INSTALLED:
        {
            cout << "have installed, uninstall it" <<endl;

            error_code = UninstallNDISDriver(L"ms_passthru", &needreboot);
            if (S_OK != error_code)
            {
                cout << "uninstall fail" << endl;
                print_error(error_code);
            }
            else
            {
                cout << "uninstall success" << endl;
            }
        }
        break;

    case NDIS_NOT_INSTALLED:
        {
            cout << "not installed, install it" <<endl;

            error_code = InstallNDISDriver(L"C:\\netsf.inf", &needreboot);
            if (S_OK != error_code)
            {
                cout << "install fail" << endl;
                print_error(error_code);
            }
            else
            {
                cout << "install success" << endl;
            }
        }
        break;

    case NDIS_QUERY_ERROR:
        cout << "query installed fail" << endl;
        print_error(error_code);
        break;

    default:
        break;
    }

    return 0;
}

