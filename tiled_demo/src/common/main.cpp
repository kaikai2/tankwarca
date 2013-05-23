#include "hge.h"
#include "game/mainGameState.h"
#include "menu/mainMenuState.h"
#include "common/entityManager.h"
#include "common/soundManager.h"
#include "common/profile.h"
#include <iostream>
using namespace std;

int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
    //ÉèÖÃHGEÊôÐÔ
    HGE *hge = hgeCreate(HGE_VERSION);
    hge->System_SetState(HGE_LOGFILE, "SysInfo.log");
    hge->System_SetState(HGE_INIFILE, "SysINI.ini");
    hge->System_SetState(HGE_TITLE, "Tiled Demo");
    hge->System_SetState(HGE_WINDOWED, true);
    hge->System_SetState(HGE_SCREENWIDTH, 800);
    hge->System_SetState(HGE_SCREENHEIGHT, 600);
    hge->System_SetState(HGE_SCREENBPP, 32);
    //hge->System_SetState(HGE_FPS, 0);
    hge->System_SetState(HGE_DONTSUSPEND, true);
    hge->System_SetState(HGE_HIDEMOUSE, false);

//#ifdef _DEBUG
#if HGE_VERSION >= 0x170
    hge->System_SetState(HGE_SHOWSPLASH, false);
#else
    hge->System_SetState(hgeIntState(14), 0xFACE0FF);
#endif
//#endif
    GameStateManager gsm;
    EntityManager em;
    SoundManager sm;
#if USE_PROFILER
    Profile pf;
#endif
    {
        MainMenuState mms;
        mms.SetName("mainmenu");
        GameStateManager::getSingleton().RegisterState(&mms);

        MainGameState mgs;
        mgs.SetName("maingame");
        GameStateManager::getSingleton().RegisterState(&mgs);

        GameStateManager::getSingleton().RequestState("maingame");
        if (hge->System_Initiate())
        {
            hge->System_Start();
        }
        GameStateManager::getSingleton().releaseSingleton();
    }
    hge->System_Shutdown();
    hge->Release();
    return 0;
}
