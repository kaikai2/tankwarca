#include "hge.h"
#include "editState.h"
#include "common/entityManager.h"
#include "common/soundManager.h"
#include "common/profile.h"
#include <windows.h>
int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
    //ÉèÖÃHGEÊôÐÔ
    HGE *hge = hgeCreate(HGE_VERSION);
    hge->System_SetState(HGE_LOGFILE, "SysInfo.log");
    hge->System_SetState(HGE_INIFILE, "SysINI.ini");
    hge->System_SetState(HGE_TITLE, "pathFinding");
    hge->System_SetState(HGE_WINDOWED, true);
    hge->System_SetState(HGE_SCREENWIDTH, 640);
    hge->System_SetState(HGE_SCREENHEIGHT, 480);
    hge->System_SetState(HGE_SCREENBPP, 32);
    hge->System_SetState(HGE_FPS, 72);
    hge->System_SetState(HGE_DONTSUSPEND, true);
    hge->System_SetState(HGE_HIDEMOUSE, false);

#ifdef _DEBUG
#if HGE_VERSION >= 0x170
    hge->System_SetState(HGE_SHOWSPLASH, false);
#else
    hge->System_SetState(hgeIntState(14), 0xFACE0FF);
#endif
#endif
    GameStateManager gsm;
    EntityManager em;
    SoundManager sm;
#if USE_PROFILER
    Profile pf;
#endif
    {
        EditState mgs;
        mgs.SetName("edit");
        GameStateManager::getSingleton().RegisterState(&mgs);

        GameStateManager::getSingleton().RequestState("edit");
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
