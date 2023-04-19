// Cycle Solver.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Cycle Solver.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
const double Cv = 718;
const double Cp = 1005;
const double Rd = 287.052874;
const double gamma = Cp / Cv;
static vector<double> ps, ts, as, w, q, dU;
static double wT = 0, qT = 0, dUT = 0, qin = 0;
bool isFloat(TCHAR*);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.



    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CYCLESOLVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CYCLESOLVER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CYCLESOLVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CYCLESOLVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static TCHAR result[128] = { 0 };
    const TCHAR defaultResult[10] = { 0 };
    static const TCHAR emptyResult[10] = _T("");
    static HWND pres_input, temp_input, volume_input, output_textbox, add_point, remove_point, points, steps, total, efficiency, carnotEfficiency, 
        pres_input_a, temp_input_a, volume_input_a, pres_input_a_f, temp_input_a_f, volume_input_a_f, adiabatic_f, add_point_a, add_point_a_f;
    static double p = 0, t = 0, a = 0, p_a = 0, t_a = 0, a_a = 0, p_a_f = 0, t_a_f = 0, a_a_f = 0;
    static std::wstring output;
    static std::wostringstream oss;
    
    auto updateValues = [](HWND hWnd) {
        if (ps.size() < 1) {
            SetWindowText(points,emptyResult);
            SetWindowText(steps, emptyResult);
            SetWindowText(total, emptyResult);
            SetWindowText(efficiency, emptyResult);
            SetWindowText(carnotEfficiency, emptyResult);
            return;
        }

        output.clear();
        for (int i = 0; i < ps.size();i++) {
            oss << (i + 1) << L": P=" << ps[i] << L" T=" << ts[i] << L" a=" << as[i] << L" \r\n";
            output += oss.str();
            oss.str(L"");

        }
        TCHAR pointsList[2048];
        _tcscpy_s(pointsList, output.c_str());
        pointsList[output.size()] = '\0';
        SetWindowText(points, pointsList);

        w.clear();
        q.clear();
        dU.clear();
        for (int i = 0; i < ps.size();i++) {
            string type;
            int i1 = (i + 1) % ps.size();
            if (ps[i] == ps[i1]) {
                type = "Isobaric\0";
                w.push_back(ps[i] * (as[i1] - as[i]));
                q.push_back(Cp * (ts[i1] - ts[i]));
                dU.push_back(Cv * (ts[i1] - ts[i]));
            }
            else if (ts[i] == ts[i1]) {
                type = "Isothermal\0";
                w.push_back(Rd * ts[i] * log(as[i1] / as[i]));
                q.push_back(Rd * ts[i] * log(as[i1] / as[i]));
                dU.push_back(0);
            }
            else if (as[i] == as[i1]) {
                type = "Isochoric\0";
                w.push_back(0);
                q.push_back(Cv * (ts[i1] - ts[i]));
                dU.push_back(Cv * (ts[i1] - ts[i]));
            }
            else if (round(ps[i] * pow(as[i], gamma)) == round(ps[i1] * pow(as[i1], gamma))) {
                type = "Adiabatic\0";
                w.push_back(-Cv * (ts[i1] - ts[i]));
                q.push_back(0);
                dU.push_back(Cv * (ts[i1] - ts[i]));
            }
            else {
                type = "Unknown\0";
                w.push_back((ps[i]+ps[i1])/2*(as[i1]-as[i]));
                q.push_back(Cv * (ts[i1] - ts[i]) + (ps[i] + ps[i1]) / 2 * (as[i1] - as[i]));
                dU.push_back(Cv * (ts[i1] - ts[i]));
            }
        }

        output.clear();
        for (int i = 0; i < ps.size();i++) {
            oss << (i + 1) << L"->" << ((i + 1) % ps.size()) + 1 << L": w=" << w[i] << L" q=" << q[i] << L" dU=" << dU[i] << L" \r\n";
            output += oss.str();
            oss.str(L"");
        }
        TCHAR stepsList[2048];
        _tcscpy_s(stepsList, output.c_str());
        stepsList[output.size()] = '\0';
        SetWindowText(steps, stepsList);

        wT = 0;
        qT = 0;
        qin = 0;
        dUT = 0;
        for (int i = 0;i < w.size();i++) {
            wT += w[i];
            qT += q[i];
            dUT += dU[i];
            if (q[i] >= 0) {
                qin += q[i];
            }
        }

        oss << L"Total: w=" << wT << L" q=" << qT << L" dU=" << dUT;
        TCHAR totals[50];
        _tcscpy_s(totals, oss.str().c_str());
        totals[oss.str().size()] = '\0';
        SetWindowText(total, totals);
        oss.str(L"");

        oss << L"Efficiency: " << abs(wT / qin);
        TCHAR eff[50];
        _tcscpy_s(eff, oss.str().c_str());
        eff[oss.str().size()] = '\0';
        SetWindowText(efficiency, eff);
        oss.str(L"");

        oss << L"Max Efficiency: " << 1 - *min_element(ts.begin(), ts.end()) / *max_element(ts.begin(), ts.end());
        _tcscpy_s(eff, oss.str().c_str());
        eff[oss.str().size()] = '\0';
        SetWindowText(carnotEfficiency, eff);
        oss.str(L"");
    };

    switch (message)
    {
    case WM_CREATE:
        pres_input = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 175, 50, 75, 20, hWnd, (HMENU)IDC_TEXT_INPUT, hInst, NULL);
        temp_input = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 175, 80, 75, 20, hWnd, (HMENU)IDC_TEXT_INPUT, hInst, NULL);
        volume_input = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 175, 110, 75, 20, hWnd, (HMENU)IDC_TEXT_INPUT, hInst, NULL);
        output_textbox = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 35, 140, 215, 20, hWnd, (HMENU)2, hInst, NULL);
        add_point = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Add To Cycle"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 35, 170, 215, 30, hWnd, (HMENU)ID_BUTTON1, hInst, NULL);
        remove_point = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Remove Last Point"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 35, 210, 215, 30, hWnd, (HMENU)ID_BUTTON2, hInst, NULL);
        points = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, 325, 50, 300, 150, hWnd, (HMENU)2, hInst, NULL);
        steps = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, 700, 50, 350, 150, hWnd, (HMENU)2, hInst, NULL);
        total = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 700, 250, 350, 30, hWnd, (HMENU)2, hInst, NULL);
        efficiency = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 325, 250, 300, 30, hWnd, (HMENU)2, hInst, NULL);
        carnotEfficiency = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 325, 330, 300, 30, hWnd, (HMENU)2, hInst, NULL);

        pres_input_a = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 720, 375, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT2, hInst, NULL);
        temp_input_a = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 720, 415, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT2, hInst, NULL);
        volume_input_a = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 720, 455, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT2, hInst, NULL);
        pres_input_a_f = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 895, 375, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT_PRES, hInst, NULL);
        temp_input_a_f = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 895, 415, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT_TEMP, hInst, NULL);
        volume_input_a_f = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 895, 455, 140, 20, hWnd, (HMENU)IDC_TEXT_INPUT_VOL, hInst, NULL);
        adiabatic_f = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 720, 490, 315, 20, hWnd, (HMENU)2, hInst, NULL);
        add_point_a = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Add To Cycle"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 760, 325, 100, 22, hWnd, (HMENU)ID_BUTTON4, hInst, NULL);
        add_point_a_f = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Add To Cycle"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 935, 325, 100, 22, hWnd, (HMENU)ID_BUTTON3, hInst, NULL);

        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDC_TEXT_INPUT:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        TCHAR pres[50];
                        TCHAR temp[50];
                        TCHAR volume[50];

                        GetWindowText(pres_input, pres, 50);
                        GetWindowText(temp_input, temp, 50);
                        GetWindowText(volume_input, volume, 50);

                        if ((!isFloat(pres) && _tcslen(pres) > 0) || (!isFloat(temp) && _tcslen(temp) > 0) || (!isFloat(volume) && _tcslen(volume) > 0)) {
                            MessageBox(hWnd, _T("Invalid input.\nPlease input a number."), _T("Message"), MB_OK);
                            break;
                        }

                        SetWindowText(output_textbox, defaultResult);
                        if (_tcslen(pres) <= 0 && _tcslen(temp) > 0 && _tcslen(volume) > 0) {
                            t = std::stod(temp);
                            a = std::stod(volume);

                            p = Rd * t / a;
                            _stprintf_s(result, 128, _T("Pressure: %f"), p);

                            SetWindowText(output_textbox, result);
                            break;
                        } else if (_tcslen(pres) > 0 && _tcslen(temp) <= 0 && _tcslen(volume) > 0) {
                            p = std::stod(pres);
                            a = std::stod(volume);

                            t = p * a / Rd;
                            _stprintf_s(result, 128, _T("Temperature: %f"), t);
                            SetWindowText(output_textbox, result);
                            break;
                        } else if (_tcslen(pres) > 0 && _tcslen(temp) > 0 && _tcslen(volume) <= 0) {
                            t = std::stod(temp);
                            p = std::stod(pres);

                            a = Rd * t / p;
                            _stprintf_s(result, 128, _T("Specific Volume: %f"), a);
                            SetWindowText(output_textbox, result);
                            break;
                        } else if (_tcslen(pres) > 0 && _tcslen(temp) > 0 && _tcslen(volume) > 0) {
                            p = std::stod(pres);
                            a = std::stod(volume);
                            t = std::stod(temp);

                            bool valid = round(p * a) == round(Rd * t);
                            _stprintf_s(result, 128, _T("Valid: %d"), valid);
                            SetWindowText(output_textbox, result);
                            break;
                        }
                        break;
                }
                break;
            case ID_BUTTON1:

                if (p <= 0 || t <= 0 || a <= 0) {
                    MessageBox(hWnd, _T("Please enter fill in two of the fields."), _T("Message"), MB_OK);
                    break;
                }
                ps.push_back(p);
                ts.push_back(t);
                as.push_back(a);
                updateValues(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                break;
            case ID_BUTTON2:
                if (ps.size() < 1) {
                    MessageBox(hWnd, _T("No cycle points left to remove."), _T("Message"), MB_OK);
                    break;
                }
                ps.pop_back();
                ts.pop_back();
                as.pop_back();
                updateValues(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                break;
            case ID_BUTTON3:
                if (p_a_f <= 0 || t_a_f <= 0 || a_a_f <= 0) {
                    MessageBox(hWnd, _T("No final adiabatic variables to add."), _T("Message"), MB_OK);
                    break;
                }
                ps.push_back(p_a_f);
                ts.push_back(t_a_f);
                as.push_back(a_a_f);
                updateValues(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                break;
            case ID_BUTTON4:
                if (p_a <= 0 || t_a <= 0 || a_a <= 0) {
                    MessageBox(hWnd, _T("No initial adiabatic variables to add."), _T("Message"), MB_OK);
                    break;
                }
                ps.push_back(p_a);
                ts.push_back(t_a);
                as.push_back(a_a);
                updateValues(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                break;
            case IDC_TEXT_INPUT2:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        TCHAR pres[50];
                        TCHAR temp[50];
                        TCHAR volume[50];
                        GetWindowText(pres_input_a, pres, 50);
                        GetWindowText(temp_input_a, temp, 50);
                        GetWindowText(volume_input_a, volume, 50);
                        if ((!isFloat(pres) && _tcslen(pres) > 0) || (!isFloat(temp) && _tcslen(temp) > 0) || (!isFloat(volume) && _tcslen(volume) > 0)) {
                            MessageBox(hWnd, _T("Invalid input.\nPlease input a number."), _T("Message"), MB_OK);
                            break;
                        }

                        if (_tcslen(pres) <= 0 && _tcslen(temp) > 0 && _tcslen(volume) > 0) {
                            t_a = std::stod(temp);
                            a_a = std::stod(volume);

                            p_a = Rd * t_a / a_a;
                            _stprintf_s(result, 50, _T("%f"), p_a);
                            break;
                        }
                        else if (_tcslen(pres) > 0 && _tcslen(temp) <= 0 && _tcslen(volume) > 0) {
                            p_a = std::stod(pres);
                            a_a = std::stod(volume);

                            t_a = p_a * a_a / Rd;
                            _stprintf_s(result, 50, _T("%f"), t_a);
                            break;
                        }
                        else if (_tcslen(pres) > 0 && _tcslen(temp) > 0 && _tcslen(volume) <= 0) {
                            t_a = std::stod(temp);
                            p_a = std::stod(pres);

                            a_a = Rd * t_a / p_a;
                            _stprintf_s(result, 50, _T("%f"), a_a);
                            break;
                        }
                        else if (_tcslen(pres) > 0 && _tcslen(temp) > 0 && _tcslen(volume) > 0) {
                            try {
                                if (std::stod(pres) != p_a) {
                                    SetWindowText(pres_input_a, defaultResult);
                                    p_a = Rd * t_a / a_a;
                                }
                                if (std::stod(temp) != t_a) {
                                    SetWindowText(temp_input_a, defaultResult);
                                    t_a = p_a * a_a / Rd;
                                }
                                if (std::stod(volume) != a_a) {
                                    SetWindowText(volume_input_a, defaultResult);
                                    a_a = Rd * t_a / p_a;
                                }
                            }
                            catch (std::invalid_argument e) {}

                        }
                        break;
                }
                break;
            case IDC_TEXT_INPUT_PRES:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        if (p_a == 0 || t_a == 0 || a_a == 0) {
                            MessageBox(hWnd, _T("Please input at least 2 initial state variables."), _T("Message"), MB_OK);
                            break;
                        }
                        TCHAR pres2[50];
                        GetWindowText(pres_input_a_f, pres2, 50);
                        if (!isFloat(pres2) && _tcslen(pres2) > 0) {
                            MessageBox(hWnd, _T("Invalid input.\nPlease input a number."), _T("Message"), MB_OK);
                            break;
                        }
                        if (_tcslen(pres2) > 0) {
                            p_a_f = std::stod(pres2);
                            t_a_f = t_a * pow(p_a_f / p_a, 1 - (1 / gamma));
                            a_a_f = a_a * pow(p_a_f / p_a, -1 / gamma);
                            output.clear();
                            TCHAR adiabaticResult[256];
                            oss << L"P=" << p_a_f << L" T=" << t_a_f << L" A=" << a_a_f;
                            output += oss.str();
                            oss.str(L"");
                            _tcscpy_s(adiabaticResult, output.c_str());
                            adiabaticResult[output.size()] = '\0';

                            SetWindowText(adiabatic_f, adiabaticResult);
                        }
                        break;
                }
                break;
            case IDC_TEXT_INPUT_TEMP:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        if (p_a == 0 || t_a == 0 || a_a == 0) {
                            MessageBox(hWnd, _T("Please input at least 2 initial state variables."), _T("Message"), MB_OK);
                            break;
                        }
                        TCHAR temp2[50];
                        GetWindowText(temp_input_a_f, temp2, 50);
                        if (!isFloat(temp2) && _tcslen(temp2) > 0) {
                            MessageBox(hWnd, _T("Invalid input.\nPlease input a number."), _T("Message"), MB_OK);
                            break;
                        }
                        if (_tcslen(temp2) > 0) {
                            t_a_f = std::stod(temp2);
                            p_a_f = p_a * pow(t_a_f / t_a, gamma / (gamma - 1));
                            a_a_f = a_a * pow(t_a_f / t_a, 1 / gamma);
                            output.clear();
                            TCHAR adiabaticResult[2048];
                            oss << L"P=" << p_a_f << L" T=" << t_a_f << L" A=" << a_a_f << L" \r\n";
                            output += oss.str();
                            oss.str(L"");
                            _tcscpy_s(adiabaticResult, output.c_str());
                            adiabaticResult[output.size()] = '\0';

                            SetWindowText(adiabatic_f, adiabaticResult);
                        }
                        break;
                }
                break;
            case IDC_TEXT_INPUT_VOL:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        if (p_a == 0 || t_a == 0 || a_a == 0) {
                            MessageBox(hWnd, _T("Please input at least 2 initial state variables."), _T("Message"), MB_OK);
                            break;
                        }
                        TCHAR volume2[50];
                        GetWindowText(volume_input_a_f, volume2, 50);
                        if (!isFloat(volume2) && _tcslen(volume2) > 0) {
                            MessageBox(hWnd, _T("Invalid input.\nPlease input a number."), _T("Message"), MB_OK);
                            break;
                        }
                        if (_tcslen(volume2) > 0) {
                            a_a_f = std::stod(volume2);
                            p_a_f = p_a * pow(a_a_f / a_a, -gamma);
                            t_a_f = t_a * pow(a_a_f / a_a, -gamma);
                            output.clear();
                            TCHAR adiabaticResult[2048];
                            oss << L"P=" << p_a_f << L"  T=" << t_a_f << L"  A=" << a_a_f << L" \r\n";
                            output += oss.str();
                            oss.str(L"");
                            _tcscpy_s(adiabaticResult, output.c_str());
                            adiabaticResult[output.size()] = '\0';

                            SetWindowText(adiabatic_f, adiabaticResult);
                        }
                        break;
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps2;
            HDC hdc = BeginPaint(hWnd, &ps2);
            // TODO: Add any drawing code that uses hdc here...

            HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            TextOut(hdc, 35, 10, _T("Ideal Gas Law:"), (int)_tcslen(_T("Ideal Gas Law:")));
            TextOut(hdc, 325, 10, _T("Cycle:"), (int)_tcslen(_T("Cycle:")));
            TextOut(hdc, 700, 10, _T("Steps:"), (int)_tcslen(_T("Steps:")));
            TextOut(hdc, 325, 210, _T("Efficiency:"), (int)_tcslen(_T("Efficiency:")));
            TextOut(hdc, 700, 210, _T("Total:"), (int)_tcslen(_T("Total:")));
            TextOut(hdc, 325, 290, _T("Carnot Efficiency:"), (int)_tcslen(_T("Carnot Efficiency:")));
            TextOut(hdc, 700, 300, _T("Adiabatic Solver:"), (int)_tcslen(_T("Adiabatic Solver:")));


            hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            hOldFont = (HFONT)SelectObject(hdc, hFont);
            TextOut(hdc, 35, 50, _T("Pressure:"), (int)_tcslen(_T("Pressure:")));
            TextOut(hdc, 35, 80, _T("Temperature:"), (int)_tcslen(_T("Temperature:")));
            TextOut(hdc, 35, 110, _T("Specific Volume:"), (int)_tcslen(_T("Specific Volume:")));

            hFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            hOldFont = (HFONT)SelectObject(hdc, hFont);
            TextOut(hdc, 710, 325, _T("Initial:"), (int)_tcslen(_T("Initial:")));
            TextOut(hdc, 885, 325, _T("Final:"), (int)_tcslen(_T("Final:")));

            hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            hOldFont = (HFONT)SelectObject(hdc, hFont);
            TextOut(hdc, 715, 355, _T("Pressure:"), (int)_tcslen(_T("Pressure:")));
            TextOut(hdc, 715, 395, _T("Temperature:"), (int)_tcslen(_T("Temperature:")));
            TextOut(hdc, 715, 435, _T("Specific Volume:"), (int)_tcslen(_T("Specific Volume:")));
            TextOut(hdc, 890, 355, _T("Pressure:"), (int)_tcslen(_T("Pressure:")));
            TextOut(hdc, 890, 395, _T("Temperature:"), (int)_tcslen(_T("Temperature:")));
            TextOut(hdc, 890, 435, _T("Specific Volume:"), (int)_tcslen(_T("Specific Volume:")));

            // Adiabatic Solver Divider
            POINT divider[2];
            divider[0].x = 875;
            divider[0].y = 325;
            divider[1].x = 875;
            divider[1].y = 485;
            Polyline(hdc, divider, 2);

            // Adiabatic Solver Box
            POINT box[5];
            box[0].x = 695;
            box[0].y = 295;
            box[1].x = 695;
            box[1].y = 515;
            box[2].x = 1060;
            box[2].y = 515;
            box[3].x = 1060;
            box[3].y = 295;
            box[4].x = 695;
            box[4].y = 295;
            Polyline(hdc, box, 5);

            // Graphing
            int xVmax = 270;
            int xVmin = 40;
            int yVmin = 500;
            int yVmax = 260;

            POINT axis[3];
            axis[0].x = xVmin - 10;
            axis[0].y = yVmax - 10;
            axis[1].x = xVmin - 10;
            axis[1].y = yVmin + 10;
            axis[2].x = xVmax + 10;
            axis[2].y = yVmin + 10;
            Polyline(hdc, axis, 3);
            
            TextOut(hdc, xVmin - 30, yVmax - 18, _T("P"), (int)_tcslen(_T("P")));
            TextOut(hdc, xVmax + 20, yVmin - 1, _T("a"), (int)_tcslen(_T("a")));


            if (ps.size() > 0) {
                HBRUSH brush;
                if (wT >= 0) {
                    brush = CreateSolidBrush(RGB(153, 230, 255));
                } else {
                    brush = CreateSolidBrush(RGB(255, 102, 102));
                }

                double xSize = xVmax - xVmin;
                double ySize = yVmax - yVmin;
                double xMin = *min_element(as.begin(), as.end());
                double xMax = *max_element(as.begin(), as.end());
                double yMin = *min_element(ps.begin(), ps.end());
                double yMax = *max_element(ps.begin(), ps.end());
                double xMulti = xSize / (xMax - xMin);
                double yMulti = ySize / (yMax - yMin);

                int specSteps = 0;
                for (int i = 0;i < ps.size();i++) {
                    if (ts[i] == ts[(i + 1) % ps.size()] || round(ps[i] * pow(as[i], gamma)) == round(ps[(i + 1) % ps.size()] * pow(as[(i + 1) % ps.size()], gamma))) {
                        specSteps++;
                    }
                }


                int interp = 50;
                const int length = (int)ps.size() + specSteps * interp;
                POINT* points = new POINT[length];
                int c = 0;
                for (int i = 0;i < length;i++) {
                    if (xMax == xMin) {
                        points[i].x = (LONG)(xVmin + (xSize / 2));
                    } else if (ts[c] == ts[(c + 1) % ps.size()]) {
                        double xDiff = (as[(c + 1) % ps.size()] - as[c]) / (interp + 1);
                        for (int j = 0;j < (interp + 1);j++) {
                            points[i+j].x = (LONG)(xVmin + ((as[c] + j * xDiff) - xMin) * xMulti);
                        }
                    } else if (round(ps[c] * pow(as[c], gamma)) == round(ps[(c + 1) % ps.size()] * pow(as[(c + 1) % ps.size()], gamma))) {
                        double xDiff = (as[(c + 1) % ps.size()] - as[c]) / (interp + 1);
                        for (int j = 0;j < (interp + 1);j++) {
                            points[i + j].x = (LONG)(xVmin + ((as[c] + j * xDiff) - xMin) * xMulti);
                        }
                    } else {
                        points[i].x = (LONG)(xVmin + (as[c] - xMin) * xMulti);
                    }
                    if (yMax == yMin) {
                        points[i].y = (LONG) (yVmin + (ySize / 2));
                    } else if (ts[c] == ts[(c + 1) % ps.size()]) {
                        double xDiff = (as[(c + 1) % ps.size()] - as[c]) / (interp + 1);
                        for (int j = 0;j < (interp + 1);j++) {
                            points[i + j].y = (LONG)(yVmin + ((ts[c] * Rd / (as[c] + j * xDiff)) - yMin) * yMulti);
                        }
                        i += interp;
                    } else if (round(ps[c] * pow(as[c], gamma)) == round(ps[(c + 1) % ps.size()] * pow(as[(c + 1) % ps.size()], gamma))) {
                        double xDiff = (as[(c + 1) % ps.size()] - as[c]) / (interp + 1);
                        for (int j = 0;j < (interp + 1);j++) {
                            points[i + j].y = (LONG)(yVmin + ((ps[c] * pow(as[c] / (as[c] + j * xDiff), gamma)) - yMin) * yMulti);
                        }
                        i += interp;
                    } else {
                        points[i].y = (LONG) (yVmin + (ps[c] - yMin) * yMulti);
                    }
                    c++;
                }
                SelectObject(hdc, brush);
                Polygon(hdc, points, length);
                DeleteObject(brush);

                delete[] points;
            }
            DeleteObject(hFont);

            EndPaint(hWnd, &ps2);
        }
        
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool isFloat(TCHAR* st) {
    try {
        double f = std::stod(st);
        return true;
    } catch (std::invalid_argument e) {
        return false;
    }
}