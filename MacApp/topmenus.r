#include "Menus.r"
#include "Windows.r"
#include "MacTypes.r"

resource 'MENU' (128) {
    128, textMenuProc;
    allEnabled, enabled;
    apple;
    {
        "About MacJukebox", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
    }
};

resource 'MENU' (129) {
    129, textMenuProc;
    allEnabled, enabled;
    "File";
    {
        "Quit", noIcon, "Q", noMark, plain;
    }
};

resource 'MBAR' (128) {
    { 128, 129 };
};

data 'TEXT' (128) {
    "About MacJukebox\r\r"
    "Created by Ravenswurk, 2021.\r"
    "Check out the code at github.com/ravenswurk/MacJukebox"
};
