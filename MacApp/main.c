/*
     Copyright 2014 Wolfgang Thaller.

     This file is part of Retro68.

     Retro68 is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     Retro68 is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with Retro68.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Quickdraw.h>
#include <Dialogs.h>
#include <Fonts.h>
#include <Resources.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <Devices.h>
#include <Serial.h>
#include <parsers/commands.h>

static Rect initialWindowRect, nextWindowRect;

enum
{
    kMenuApple = 128,
    kMenuFile,
    kMenuEdit
};

enum
{
    kItemAbout = 1,
    kItemQuit = 1
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

    HiliteMenu(0);
}

void Update(WindowRef w) {
    SetPort(w);
    BeginUpdate(w);
    EndUpdate(w);
}

int main(int argc, char** argv) {
    IOParam outPort;
    OpenDriver("\p.AOut", &outPort.ioRefNum);

    CntrlParam cb;
    cb.ioCRefNum = outPort.ioRefNum;
    cb.csCode = 8;
    cb.csParam[0] = stop10 | noParity | data8 | baud9600;
    OSErr err = PBControl ((ParmBlkPtr) & cb, 0);

    CloseDriver(outPort.ioRefNum);

    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);

    SetMenuBar(GetNewMBar(128));
    AppendResMenu(GetMenu(128), 'DRVR');
    DrawMenuBar();

    InitCursor();

    WindowRef w = GetNewWindow(128, NULL, (WindowPtr) - 1);

    ControlRef playBtn = GetNewControl(128, w);
    ControlRef pauseBtn = GetNewControl(129, w);
    ControlRef skipBtn = GetNewControl(130, w);

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
                            break;
                        case inSysWindow: {
                            SystemClick(&e, win);

                            Point point = e.where;
                            GlobalToLocal(&point);
                            ControlRef cntrl;
                            if (FindControl(point, win, &cntrl) != 0) {
                                TrackControl(cntrl, point, NULL);
                            }

                            break;
                        }
                    }
                    break;
                case updateEvt:
                    Update((WindowRef)e.message);
                    break;
            }
        }
    }

    return 0;
}
