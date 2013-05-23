#include "hge.h"
#include "hgeVector.h"
#include "hgeFont.h"
#include <vector>
#include <algorithm>
#include "collisionSpace.h"
using std::vector;
using std::sort;

HGE *hge = 0;
hgeFont *font = 0;

Map map;

float time;
bool OnFrame()
{
    static bool flag = false;
    if (!flag)
    {
        float t1 = GetTickCount();
        map.randomVertices();

        time = (GetTickCount() - t1) * 0.001f;
        flag = true;
    }
    return false;
}
bool OnRender()
{
    hge->Gfx_BeginScene();
    hge->Gfx_Clear(0);
    map.render();
    char buf[32];
    sprintf(buf, "%.2f", time);
    font->Render(100, 100, HGETEXT_LEFT, buf);
    hge->Gfx_EndScene();
    return false;
}
int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{	
    //ÉèÖÃHGEÊôÐÔ
    hge = hgeCreate(HGE_VERSION);
    hge->System_SetState(HGE_LOGFILE, "SysInfo.log");
    hge->System_SetState(HGE_INIFILE, "SysINI.ini");
    hge->System_SetState(HGE_TITLE, "MapRouteGenerator");
    hge->System_SetState(HGE_WINDOWED, true);
    hge->System_SetState(HGE_SCREENWIDTH, 800);
    hge->System_SetState(HGE_SCREENHEIGHT, 600);
    hge->System_SetState(HGE_SCREENBPP, 32);
    hge->System_SetState(HGE_FPS, 61);
    hge->System_SetState(HGE_DONTSUSPEND, true);
    hge->System_SetState(HGE_HIDEMOUSE, false);

//#ifdef _DEBUG
#if HGE_VERSION >= 0x170
    hge->System_SetState(HGE_SHOWSPLASH, false);
#else
    hge->System_SetState(hgeIntState(14), 0xFACE0FF);
#endif
//#endif

    hge->System_SetState(HGE_FRAMEFUNC, OnFrame);
    hge->System_SetState(HGE_RENDERFUNC, OnRender);
    if (hge->System_Initiate())
    {
        font = new hgeFont("data/font.fnt");

        hge->System_Start();
    }
    delete font;
    hge->System_Shutdown();
    hge->Release();
    return 0;
}
