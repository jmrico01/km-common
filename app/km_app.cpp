#include "km_app.h"

#if GAME_WIN32
#include "km_win32_app.cpp"
#else
#error "Unsupported platform"
#endif
