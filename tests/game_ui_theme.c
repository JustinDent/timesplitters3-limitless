#include "pc_menu_theme.h"
#include "pc_dialog_theme.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)) {printf("FAIL: line %d\n",__LINE__);return 1;}} while(0)

static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    LRESULT result;
    if(pc_menu_theme_message(window,message,wp,lp,&result)) return result;
    return DefWindowProcW(window,message,wp,lp);
}

int main(void)
{
    WNDCLASSW kind={0};HWND window,list;HMENU bar,popup;MENUINFO info={sizeof info};
    LOGBRUSH brush;LRESULT result;HDC dc;HBITMAP bitmap,old;
    DRAWITEMSTRUCT draw={0};MEASUREITEMSTRUCT measure={0};PcDialogTheme dialog={0};INT_PTR answer;
    INITCOMMONCONTROLSEX controls={sizeof controls,ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&controls);
    kind.lpfnWndProc=procedure;kind.hInstance=GetModuleHandleW(NULL);kind.lpszClassName=L"TSFP Theme Check";kind.cbWndExtra=DLGWINDOWEXTRA;
    CHECK(RegisterClassW(&kind));
    window=CreateWindowW(kind.lpszClassName,L"UI theme check",WS_OVERLAPPEDWINDOW,0,0,500,300,NULL,NULL,kind.hInstance,NULL);
    CHECK(window);bar=CreateMenu();popup=CreatePopupMenu();
    AppendMenuW(popup,MF_STRING,100,L"&Keyboard && Mouse...");AppendMenuW(popup,MF_STRING,101,L"&Debug...");
    AppendMenuW(popup,MF_SEPARATOR,0,NULL);AppendMenuW(popup,MF_STRING,102,L"&Fullscreen");AppendMenuW(popup,MF_STRING,103,L"&Hide Menu Bar");
    AppendMenuW(bar,MF_POPUP,(UINT_PTR)popup,L"&PC Settings");SetMenu(window,bar);pc_menu_theme_attach(window);
    pc_menu_theme_refresh(window,1);info.fMask=MIM_BACKGROUND;
    CHECK(GetMenuInfo(bar,&info));CHECK(GetObject(info.hbrBack,sizeof brush,&brush));CHECK(brush.lbColor==RGB(32,32,32));
    measure.CtlType=ODT_MENU;measure.itemData=1;
    CHECK(pc_menu_theme_message(window,WM_MEASUREITEM,0,(LPARAM)&measure,&result));CHECK(measure.itemHeight>12 && measure.itemWidth>50);
    dc=CreateCompatibleDC(NULL);bitmap=CreateBitmap(400,50,1,32,NULL);old=(HBITMAP)SelectObject(dc,bitmap);
    draw.CtlType=ODT_MENU;draw.itemData=1;draw.hDC=dc;draw.rcItem.right=400;draw.rcItem.bottom=50;
    CHECK(pc_menu_theme_message(window,WM_DRAWITEM,0,(LPARAM)&draw,&result));CHECK(GetPixel(dc,1,1)==RGB(32,32,32));
    draw.itemState=ODS_SELECTED;CHECK(pc_menu_theme_message(window,WM_DRAWITEM,0,(LPARAM)&draw,&result));CHECK(GetPixel(dc,1,1)==RGB(60,60,60));
    CHECK(pc_menu_theme_message(window,WM_MENUCHAR,L'f',(LPARAM)popup,&result));CHECK(LOWORD(result)==3 && HIWORD(result)==MNC_EXECUTE);
    list=CreateWindowW(WC_LISTVIEWW,L"",WS_CHILD|LVS_REPORT,0,0,100,100,window,NULL,kind.hInstance,NULL);CHECK(list);
    dialog.dark=1;dialog.brush=CreateSolidBrush(RGB(32,32,32));pc_dialog_theme_child(list,(LPARAM)&dialog);
    CHECK(ListView_GetBkColor(list)==RGB(32,32,32));CHECK(ListView_GetTextColor(list)==RGB(240,240,240));
    CHECK(pc_dialog_theme_message(window,&dialog,WM_CTLCOLORDLG,(WPARAM)dc,(LPARAM)window,&answer));CHECK((HBRUSH)answer==dialog.brush);
    dialog.dark=0;pc_dialog_theme_child(list,(LPARAM)&dialog);CHECK(ListView_GetBkColor(list)==GetSysColor(COLOR_WINDOW));
    CHECK(!pc_dialog_theme_message(window,&dialog,WM_CTLCOLORDLG,(WPARAM)dc,(LPARAM)window,&answer));
    pc_menu_theme_refresh(window,0);GetMenuInfo(bar,&info);GetObject(info.hbrBack,sizeof brush,&brush);CHECK(brush.lbColor==GetSysColor(COLOR_MENU));
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);DeleteObject(dialog.brush);
    pc_menu_theme_detach(window);DestroyWindow(window);
    puts("PASS: menu background, hover, measurements, mnemonics, dark dialog/list colors and native light restore");
    return 0;
}
