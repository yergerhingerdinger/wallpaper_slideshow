#include <stdio.h>
#include <Windows.h>
#include "Gdiplus.h"
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <ctime>

#define internal static
#define local_persist static
#define global_variable static

using namespace Gdiplus;

struct ENUM_DISP_ARG
{
	TCHAR msg[500];
	int monId;
};

internal BOOL CALLBACK 
EnumWindowsProc(HWND Window, LPARAM LParam) 
{
    HWND ShellDLLDefView = FindWindowEx(Window, 0, _T("SHELLDLL_DefView"), 0);
    HWND* WorkerW = (HWND*)LParam;
    if (ShellDLLDefView) 
    {
        // Gets the WorkerW Window after the current one
        *WorkerW = FindWindowEx(0, Window, _T("WorkerW"), 0);
    }
    
    return TRUE;
}


internal HDC
GetBackgroundDeviceContext(void)
{
    HWND ProgMan = FindWindow(_T("Progman"), 0);
    SendMessageTimeout(ProgMan, 0x052C, 0, 0, SMTO_NORMAL, 1000, 0);
    
    HWND WorkerW = 0;
    EnumWindows(EnumWindowsProc, (LPARAM)&WorkerW);
    
    return(GetDC(WorkerW));
}


// callback function called by EnumDisplayMonitors for each enabled monitor
BOOL CALLBACK EnumDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam)
{
	ENUM_DISP_ARG* pArg = (ENUM_DISP_ARG*)lParam;
    
	TCHAR str[100] = _T("");
	StringCbPrintf(str, sizeof(str), _T("Monitor %d: %d x %d @ %d,%d\n"), pArg->monId,
                   pRcMon->right - pRcMon->left, pRcMon->bottom - pRcMon->top, pRcMon->left, pRcMon->top);
	StringCbCat(pArg->msg, sizeof(pArg->msg), str);
	pArg->monId++;
    
	return TRUE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
{
    
    
    
    //MONITOR 1: 1920 X 1080 @ 0,0
    //MONITOR 2: 1600 X 900  @ 1920,0
    
    /*
      ---------------------------
        EnumDisp
        ---------------------------
        Monitor 1: 1920 x 1080 @ 2560,0
        Monitor 2: 2560 x 1440 @ 0,0
        
        ---------------------------
        OK   
        ---------------------------
        */
    
    
    // Display box showing dimensions of the monitors
    ENUM_DISP_ARG arg = {0};
    arg.monId = 1;
    EnumDisplayMonitors(0, 0, EnumDispProc, (LPARAM)&arg);
    MessageBox(0, arg.msg, _T("EnumDisp"), MB_ICONINFORMATION | MB_OK);
    
    
    HDC DeviceContext = GetBackgroundDeviceContext();
    
    // Initialize GdiPlus
    GdiplusStartupInput startInput;
    ULONG_PTR gdiToken;
    Status status = GdiplusStartup(&gdiToken, &startInput, NULL);
    
    
    // First iteration to count the number of files inside directory
    int FileCount = 0;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFile("D:\\Wallpapers\\Wallpapers\\*", &FindData);
    
    if (FindHandle != INVALID_HANDLE_VALUE) 
    {
        while (FindNextFile(FindHandle, &FindData)) 
        {
            if (strcmp(FindData.cFileName, ".") != 0 && strcmp(FindData.cFileName, "..") != 0) 
            {
                ++FileCount;
            }
        }
    }
    FindClose(FindHandle);
    
    //Puts all the image files in the FileArray
    std::vector<std::string> FileArray;
    const std::string directory = "D:\\Wallpapers\\Wallpapers\\";
    FindHandle = FindFirstFile("D:\\Wallpapers\\Wallpapers\\*", &FindData);
    
    if (FindHandle != INVALID_HANDLE_VALUE) 
    {
        
        while (FindNextFile(FindHandle, &FindData)) 
        {
            
            if (strcmp(FindData.cFileName, ".") != 0 && strcmp(FindData.cFileName, "..") != 0) 
            {
                std::string file_name = directory + FindData.cFileName;
                FileArray.push_back(file_name);
            }
        }
    }
    
    FindClose(FindHandle);
    
    Graphics *graphics = nullptr;
    
    //TODO: Fix this
    srand(time(NULL));
    int index = rand() % 75;
    
    std::string temp = FileArray[index];
    std::wstring widestr = std::wstring(FileArray[index].begin(), FileArray[index].end());
    const wchar_t* widecstr = widestr.c_str();
    Image *image = Image::FromFile(widecstr);
    
    // Scheduling (Timer)
    DWORD now = timeGetTime();
    DWORD nextActivatedTime = now+3000;
    
    if (DeviceContext != NULL) 
    {
        graphics = Graphics::FromHDC(DeviceContext);
        
        if (graphics != NULL) 
        {
            //graphics->FillRectangle(new SolidBrush(Color::Color(255,255,255)), 1920, 0, 1500, 800);
            // Monitor 1
            //graphics->DrawImage(image, 0, 0, 1920, 1080);
            graphics->DrawImage(image, 0, 0, 1920, 1080);
        }
    }
    
    // TODO: Potential  Leak?
    while(1)
    {
        now = timeGetTime();
        DWORD nextEventTime = nextActivatedTime;
        
        DWORD MillisecondsToSleep = nextEventTime-now;
        Sleep(MillisecondsToSleep);
        
        now = timeGetTime();
        
        // TODO:Change Wallpaper?
        if (now >= nextActivatedTime)
        {
            index = rand() % 75;
            temp = FileArray[index];
            
            widestr = std::wstring(FileArray[index].begin(), FileArray[index].end());
            image = Image::FromFile(widestr.c_str());
            
            if (graphics != NULL) 
            {
                graphics->Clear((Color::Color(255,255,255)));
                //graphics->DrawImage(image, 1920, 0, 1600, 900);
                //graphics->DrawImage(image, 1920, 0, 2560, 900);
                graphics->DrawImage(image, 2560, 0, 1920, 1080);
            }
            nextActivatedTime += 6000;
        }
    }
    graphics->ReleaseHDC(DeviceContext);
    GdiplusShutdown(gdiToken);
}