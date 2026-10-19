#ifndef TSFP_PC_MENU_THEME_H
#define TSFP_PC_MENU_THEME_H
#include "pc_window_theme.h"
#include <wctype.h>

typedef struct PcMenuTheme { HBRUSH brush; HFONT font; int dark; } PcMenuTheme;
static const WCHAR pc_menu_theme_property[]=L"TSFP.MenuTheme";
static const WCHAR *pc_menu_labels[]={L"&PC Settings",L"&Keyboard && Mouse...",L"&Debug...",L"",L"&Fullscreen",L"&Hide Menu Bar"};
static const WCHAR *pc_menu_keys[]={L"",L"F2",L"F12",L"",L"Alt+Enter",L"F1"};

static void pc_menu_theme_refresh(HWND window,int dark)
{
    PcMenuTheme *theme=(PcMenuTheme *)GetPropW(window,pc_menu_theme_property);
    MENUINFO info={sizeof info};
    HMENU bar=GetMenu(window);
    HBRUSH previous;
    if(!theme || !bar) return;
    previous=theme->brush;
    theme->dark=dark;
    theme->brush=CreateSolidBrush(dark?RGB(32,32,32):GetSysColor(COLOR_MENU));
    info.fMask=MIM_BACKGROUND|MIM_APPLYTOSUBMENUS;info.hbrBack=theme->brush;
    SetMenuInfo(bar,&info);
    if(previous) DeleteObject(previous);
    DrawMenuBar(window);
}

static void pc_menu_theme_attach(HWND window)
{
    HMENU bar=GetMenu(window),popup;
    PcMenuTheme *theme;
    MENUITEMINFOW item={sizeof item};
    NONCLIENTMETRICSW metrics={sizeof metrics};
    int index;
    if(!bar || GetPropW(window,pc_menu_theme_property)) return;
    theme=(PcMenuTheme *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof *theme);
    if(!theme) return;
    if(!SetPropW(window,pc_menu_theme_property,(HANDLE)theme)) {HeapFree(GetProcessHeap(),0,theme);return;}
    if(SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,sizeof metrics,&metrics,0))
        theme->font=CreateFontIndirectW(&metrics.lfMenuFont);
    if(!theme->font) theme->font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
    item.fMask=MIIM_FTYPE|MIIM_DATA;item.fType=MFT_OWNERDRAW;item.dwItemData=1;
    SetMenuItemInfoW(bar,0,TRUE,&item);
    popup=GetSubMenu(bar,0);
    for(index=0;index<GetMenuItemCount(popup);index++) {
        item.fType=MFT_OWNERDRAW|(index==2?MFT_SEPARATOR:0);item.dwItemData=index+2;
        SetMenuItemInfoW(popup,index,TRUE,&item);
    }
    pc_menu_theme_refresh(window,pc_window_theme_dark());
}

static void pc_menu_theme_detach(HWND window)
{
    PcMenuTheme *theme=(PcMenuTheme *)RemovePropW(window,pc_menu_theme_property);
    if(theme) {
        if(theme->brush) DeleteObject(theme->brush);
        if(theme->font && theme->font!=(HFONT)GetStockObject(DEFAULT_GUI_FONT)) DeleteObject(theme->font);
        HeapFree(GetProcessHeap(),0,theme);
    }
}

static int pc_menu_theme_message(HWND window,UINT message,WPARAM wp,LPARAM lp,LRESULT *result)
{
    PcMenuTheme *theme=(PcMenuTheme *)GetPropW(window,pc_menu_theme_property);
    (void)wp;
    if(!theme) return 0;
    if(message==WM_SETTINGCHANGE || message==WM_THEMECHANGED)
        pc_menu_theme_refresh(window,pc_window_theme_dark());
    if(message==WM_MEASUREITEM) {
        MEASUREITEMSTRUCT *item=(MEASUREITEMSTRUCT *)lp;
        unsigned index=(unsigned)item->itemData-1;
        HDC dc;HFONT old;SIZE label,key;
        if(item->CtlType!=ODT_MENU || index>=6) return 0;
        dc=GetDC(window);old=(HFONT)SelectObject(dc,theme->font);
        GetTextExtentPoint32W(dc,pc_menu_labels[index],lstrlenW(pc_menu_labels[index]),&label);
        GetTextExtentPoint32W(dc,pc_menu_keys[index],lstrlenW(pc_menu_keys[index]),&key);
        item->itemWidth=label.cx+key.cx+(index?72:16);
        item->itemHeight=index==3?8:label.cy+(index?12:8);
        SelectObject(dc,old);ReleaseDC(window,dc);*result=TRUE;return 1;
    }
    if(message==WM_DRAWITEM) {
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT *)lp;
        unsigned index=(unsigned)item->itemData-1;
        int selected,disabled;COLORREF background,foreground;RECT text;HFONT old;
        if(item->CtlType!=ODT_MENU || index>=6) return 0;
        selected=(item->itemState&(ODS_SELECTED|ODS_HOTLIGHT))!=0;
        disabled=(item->itemState&(ODS_DISABLED|ODS_GRAYED))!=0;
        background=theme->dark?(selected?RGB(60,60,60):RGB(32,32,32)):
            GetSysColor(selected?COLOR_HIGHLIGHT:COLOR_MENU);
        foreground=theme->dark?(disabled?RGB(140,140,140):RGB(240,240,240)):
            GetSysColor(disabled?COLOR_GRAYTEXT:selected?COLOR_HIGHLIGHTTEXT:COLOR_MENUTEXT);
        SetDCBrushColor(item->hDC,background);FillRect(item->hDC,&item->rcItem,(HBRUSH)GetStockObject(DC_BRUSH));
        text=item->rcItem;SetBkMode(item->hDC,TRANSPARENT);SetTextColor(item->hDC,foreground);
        old=(HFONT)SelectObject(item->hDC,theme->font);
        if(index==3) {
            text.left+=12;text.right-=12;text.top+=(text.bottom-text.top)/2;text.bottom=text.top+1;
            SetDCBrushColor(item->hDC,theme->dark?RGB(80,80,80):GetSysColor(COLOR_3DSHADOW));
            FillRect(item->hDC,&text,(HBRUSH)GetStockObject(DC_BRUSH));
        } else {
            UINT flags=DT_SINGLELINE|DT_VCENTER|((item->itemState&ODS_NOACCEL)?DT_HIDEPREFIX:0);
            text.left+=index?30:8;text.right-=12;
            DrawTextW(item->hDC,pc_menu_labels[index],-1,&text,flags);
            DrawTextW(item->hDC,pc_menu_keys[index],-1,&text,flags|DT_RIGHT);
            if(item->itemState&ODS_CHECKED) {
                HPEN pen=CreatePen(PS_SOLID,2,foreground),previous=(HPEN)SelectObject(item->hDC,pen);
                int x=item->rcItem.left+10,y=(text.top+text.bottom)/2;
                MoveToEx(item->hDC,x,y,NULL);LineTo(item->hDC,x+3,y+3);LineTo(item->hDC,x+9,y-4);
                SelectObject(item->hDC,previous);DeleteObject(pen);
            }
        }
        SelectObject(item->hDC,old);*result=TRUE;return 1;
    }
    if(message==WM_MENUCHAR) {
        HMENU menu=(HMENU)lp;
        int index;
        for(index=0;index<GetMenuItemCount(menu);index++) {
            MENUITEMINFOW item={sizeof item};const WCHAR *label;
            item.fMask=MIIM_DATA;
            if(!GetMenuItemInfoW(menu,index,TRUE,&item) || item.dwItemData<1 || item.dwItemData>6) continue;
            label=pc_menu_labels[item.dwItemData-1];
            if(label[0]==L'&' && (WCHAR)towupper(label[1])==(WCHAR)towupper(LOWORD(wp))) {
                *result=MAKELRESULT(index,MNC_EXECUTE);return 1;
            }
        }
    }
    return 0;
}
#endif
