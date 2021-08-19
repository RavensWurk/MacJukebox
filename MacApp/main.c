/*
     Copyright 2021 Ravenswurk.

     This file is part of MacJukebox.

     MacJukebox is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     MacJukebox is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with MacJukebox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Quickdraw.h>
#include <Windows.h>
#include <Lists.h>
#include <Fonts.h>
#include <Resources.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <Devices.h>
#include <Serial.h>
#include <Sound.h>
#include <parsers/commands.h>
#include "serial.h"
#include "commands.h"
#include "filelist.pb.h"

static Rect initialWindowRect, nextWindowRect;

enum
{
    kMenuApple = 128,
    kMenuFile,
    kMenuOptions,
};

enum
{
    kItemAbout = 1,
    kItemQuit = 1,
    kModemPort = 1,
    kPrinterPort = 2
};

void ShowAboutBox()
{
    WindowRef w = GetNewWindow(129, NULL, (WindowPtr) - 1);
    MoveWindow(w,
        qd.screenBits.bounds.right/2 - w->portRect.right/2,
        qd.screenBits.bounds.bottom/2 - w->portRect.bottom/2,
        false);
    ShowWindow(w);
    SetPort(w);

    Handle h = GetResource('TEXT', 128);
    HLock(h);
    Rect r = w->portRect;
    InsetRect(&r, 10,10);
    TETextBox(*h, GetHandleSize(h), &r, teJustLeft);

    ReleaseResource(h);
    while(!Button())
        ;
    while(Button())
        ;
    FlushEvents(everyEvent, 0);
    DisposeWindow(w);
}


void DoMenuCommand(long menuCommand)
{
    Str255 str;
    WindowRef w;
    short menuID = menuCommand >> 16;
    short menuItem = menuCommand & 0xFFFF;
    if(menuID == kMenuApple)
    {
        if(menuItem == kItemAbout)
            ShowAboutBox();
        else
        {
            GetMenuItemText(GetMenu(128), menuItem, str);
            OpenDeskAcc(str);
        }
    }
    else if(menuID == kMenuFile)
    {
        switch(menuItem)
        {
            case kItemQuit:
                ExitToShell();
                break;
        }
    }
    else if (menuID == kMenuOptions)
    {
        if (menuItem == kModemPort) {
            CheckItem(GetMenu(kMenuOptions), kModemPort, true);
            CheckItem(GetMenu(kMenuOptions), kPrinterPort, false);
        } else if (menuItem == kPrinterPort) {
            CheckItem(GetMenu(kMenuOptions), kModemPort, false);
            CheckItem(GetMenu(kMenuOptions), kPrinterPort, true);
        }
    }

    HiliteMenu(0);
}

void Update(WindowRef w) {
    SetPort(w);
    BeginUpdate(w);
    EndUpdate(w);
}

void UpdateList(ListHandle list) {
    SetPort((*list)->port);
    LUpdate((*list)->port->visRgn, list);

    PenState penState;
    GetPenState(&penState);
    PenSize(1, 1);
    InsetRect(&(*list)->rView, -1, -1);
    FrameRect(&(*list)->rView);
    SetPenState(&penState);
}

void FileListCallback(FileListing_File *file, void* arg) {
    ListHandle list = (ListHandle)arg;
}

int main(int argc, char** argv) {
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);

    SetMenuBar(GetNewMBar(128));
    AppendResMenu(GetMenu(128), 'DRVR');
    DrawMenuBar();

    WindowRef w = GetNewWindow(128, NULL, (WindowPtr) - 1);

    ControlRef playBtn = GetNewControl(128, w);
    ControlRef pauseBtn = GetNewControl(129, w);
    ControlRef skipBtn = GetNewControl(130, w);

    Rect listRect;
    Point p;
    SetRect(&listRect, 0, 0, 3, 0);
    SetPt(&p, 0, 0);

    Rect boundRect = w->portRect;
    boundRect.left += 10;
    boundRect.top += 10;
    boundRect.right -= 15;
    boundRect.bottom -= 50;

    ListHandle list = LNew(&boundRect, &listRect, p, 0, w,
           true, false, true, true);

    (**list).selFlags = 0;

    IOParam param = {0};
    SerialInit("\p.AOut", &param);
    ListFiles(&param, FileListCallback, &list);

    LAddRow(1, 1, list);
    SetPt(&p, 0, 0);
    LSetCell("David Bowie", strlen("David Bowie"), p, list);
    SetPt(&p, 1, 0);
    LSetCell("Ziggy", strlen("Ziggy"), p, list);
    LDraw(p, list);

    InitCursor();
    ShowWindow(w);
    SetPort(w);

    while(true) {
        EventRecord e;
        WindowRef win;

        SystemTask();
        if(GetNextEvent(everyEvent, &e))
        {
            switch(e.what)
            {
                case keyDown:
                    if(e.modifiers & cmdKey)
                    {
                        DoMenuCommand(MenuKey(e.message & charCodeMask));
                    }
                    break;
                case mouseDown:
                    switch(FindWindow(e.where, &win))
                    {
                        case inGoAway:
                            if(TrackGoAway(win, e.where))
                                DisposeWindow(win);
                            break;
                        case inDrag:
                            DragWindow(win, e.where, &qd.screenBits.bounds);
                            break;
                        case inMenuBar:
                            DoMenuCommand( MenuSelect(e.where) );
                            break;
                        case inContent:
                            SelectWindow(win);

                            Point point = e.where;
                            GlobalToLocal(&point);
                            ControlRef cntrl;
                            if (FindControl(point, win, &cntrl) != 0) {
                                TrackControl(cntrl, point, NULL);
                            }

                            LClick(point, e.modifiers, list);

                            break;
                        case inSysWindow: {
                            SystemClick(&e, win);
                            break;
                        }
                    }
                    break;
                case updateEvt:
                    Update((WindowRef)e.message);
                    UpdateList(list);
                    break;
            }
        }
    }

    return 0;
}
