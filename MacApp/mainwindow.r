#include "Windows.r"
#include "Controls.r"
#include "MacTypes.r"

resource 'CNTL' (128) {
    { 275, 17, 295, 75 },
    0,
    visible,
    0, 0,
    pushButProc,
    0, "Play"
};

resource 'CNTL' (129) {
    { 275, 81, 295, 139 },
    0,
    visible,
    0, 0,
    pushButProc,
    0, "Pause"
};

resource 'CNTL' (130) {
    { 275, 145, 295, 203 },
    0,
    visible,
    0, 0,
    pushButProc,
    0, "Skip"
};

resource 'WIND' (128) {
    {0, 0, 300, 400}, documentProc;
    visible;
    goAway;
    0,
    "MacJukebox",
    centerMainScreen
};

resource 'WIND' (129) {
    {0, 0, 220, 320}, altDBoxProc;
    visible;
    noGoAway;
    0, "";
    noAutoCenter;
};
