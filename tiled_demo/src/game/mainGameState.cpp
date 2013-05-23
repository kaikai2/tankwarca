#pragma warning(disable: 4996)

#include <cmath>
#include <cassert>
#include <cctype>
#include <sstream>
#include <hgefont.h>
#include <hgesprite.h>
#include <hgeguictrls.h>
#include <hgecolor.h>

#include "game/mainGameState.h"
#include "common/entity.h"
#include "common/entityManager.h"
#include "common/soundManager.h"
#include "common/graphicEntity.h"
#include "common/profile.h"
#include "common/iRenderQueue.h"

#include "map.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) if (!a);else {delete a; a = 0;}
#endif

enum GUI_ID
{
    /// GUI main
    GID_BtnRand = 1,
    GID_BtnStart,
    GID_BtnToggleProfiler,

    NumGUIId,
};
extern void renderFontWithBk(hgeFont &font, int x, int y, DWORD bcolor, DWORD color, int align, const char *str);

MainGameState::~MainGameState()
{
    assert(!this->gui);
    assert(!this->hge);
}

void MainGameState::OnEnter()
{
    this->hge = hgeCreate(HGE_VERSION);
    renderQueue.setWindowSize(Point2f((float)hge->System_GetState(HGE_SCREENWIDTH), (float)hge->System_GetState(HGE_SCREENHEIGHT)));

    this->font = new hgeFont("data/font.fnt");
    this->font->SetColor(ARGB(255, 0, 255, 0));

    this->system = new hgeCurvedAniSystem;
    this->animResManager = iSystem::GetInstance()->createAnimResManager();

    iSystem::GetInstance()->attach(&this->dataTriangleRenderer);

    this->texGui = hge->Texture_Load("data/ui.png");
    this->gui = new hgeGUI;
    //this->gui->AddCtrl(new hgeGUIButton(GID_BtnRand, 10, 100, 36, 19, texGui, 0, 0));
    //this->gui->AddCtrl(new hgeGUIButton(GID_BtnStart, 10, 140, 39, 19, texGui, 0, 19));
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnToggleProfiler, 10, 500, 39, 19, texGui, 0, 38));
    
    Rectf playRange;
    playRange.leftTop = Point2f(-2500, -2500);
    playRange.rightBottom = Point2f(2500, 2500);
    EntityManager::getSingleton().setPlayRange(playRange);
    EntityManager::getSingleton().clear();

    float windowWidth = iRenderQueue::getSingleton().getWindowSize().x;
    float windowHeight = iRenderQueue::getSingleton().getWindowSize().y;

    this->map = new TiledMapEntity(*animResManager);

    this->map->centerPos = Point2f(windowWidth/2, windowHeight/2);
    this->map->tileSize = Point2f(160, 80);

	for (int i = -10; i < 10; i++)
	{
		for (int j = -10; j < 10; j++)
		{
			//if (rand() % 10 > 6)
			//	continue;
			std::stringstream ss;
			ss << "data/Image/Tile/tile" << rand() % 10 << ".xml";
			TileEntity *tile = new TileEntity(*animResManager, *map, ss.str().c_str());
			tile->SetTilePos(TilePos(i, j));
			//tile->setTileId(rand()%10);
			this->map->AddTile(*tile);
		}
	}
    Rectf range;
    range.leftTop = Point2f(-2000.0f, -2000.0f);
    range.rightBottom = Point2f(2000.0f, 2000.0f);
}

void MainGameState::OnLeave()
{
    EntityManager::getSingleton().dettach(this->map);
    EntityManager::getSingleton().setCollisionChecker(0);
    SAFE_DELETE(this->map);
    EntityManager::getSingleton().clear();
    SAFE_DELETE(this->gui);
    SAFE_DELETE(this->font);
    iSystem::GetInstance()->release(this->animResManager);
    iSystem::GetInstance()->dettach(&this->dataTriangleRenderer);
    SAFE_DELETE(this->system);
    if (this->texGui)
    {
        this->hge->Texture_Free(this->texGui);
        this->texGui = 0;
    }
    this->hge->Release();
    this->hge = 0;
}

void MainGameState::ProcessControl(const hgeInputEvent &event)
{
    event;

    static bool turn = false;
    switch(event.type)
    {
#if 0
    case INPUT_KEYDOWN:
        switch(event.key)
        {
        case HGEK_A:
            this->tankController.sendCommand(Tank::TCI_TurnLeft, "1");
            break;
        case HGEK_D:
            this->tankController.sendCommand(Tank::TCI_TurnRight, "1");
            break;
        case HGEK_W:
            this->tankController.sendCommand(Tank::TCI_Forward, "1");
            break;
        case HGEK_S:
            this->tankController.sendCommand(Tank::TCI_Backward, "1");
            break;
            //case HGEK_Q:
            //    this->tankController.sendCommand(Tank::TCI_CutEnginePower);
            //    break;  
        case HGEK_SPACE:
            this->tankController.sendCommand(Tank::TCI_Fire, "1");
            break;
        case HGEK_SHIFT:
            this->tankController.sendCommand(Tank::TCI_HalfEnginePower, "1");
            break;
        }
        break;
    case INPUT_KEYUP:
        switch(event.key)
        {
        case HGEK_A:
            this->tankController.sendCommand(Tank::TCI_TurnLeft, 0);
            break;
        case HGEK_D:
            this->tankController.sendCommand(Tank::TCI_TurnRight, 0);
            break;
        case HGEK_W:
            this->tankController.sendCommand(Tank::TCI_Forward, 0);
            break;
        case HGEK_S:
            this->tankController.sendCommand(Tank::TCI_Backward, 0);
            break;
        case HGEK_SPACE:
            this->tankController.sendCommand(Tank::TCI_Fire, 0);
            break;
        case HGEK_SHIFT:
            this->tankController.sendCommand(Tank::TCI_HalfEnginePower, 0);
            break;
        }
        break;
    case INPUT_MBUTTONDOWN:
        switch(event.key)
        {
        case HGEK_LBUTTON:
            this->tankController.sendCommand(Tank::TCI_Shoot, "1");
            break;
        case HGEK_RBUTTON:
            turn = true;
            {
                float x, y;
                hge->Input_GetMousePos(&x, &y);
                Point2f p(x, y);
                iRenderQueue::getSingleton().screenToGame(p);
                this->tankController.sendCommand(Tank::TCI_Turn, &p);
            }
            break;
        }
        break;
    case INPUT_MBUTTONUP:
        switch(event.key)
        {
        case HGEK_LBUTTON:
            this->tankController.sendCommand(Tank::TCI_Shoot, 0);
            break;
        case HGEK_RBUTTON:
            turn = false;
            break;
        }
        break;
#endif
    case INPUT_MOUSEMOVE:
        {
            float x, y;
            hge->Input_GetMousePos(&x, &y);
            Point2f p(x*2-800, y*2-600);
            //iRenderQueue::getSingleton().screenToGame(p);
			iRenderQueue::getSingleton().setViewerPos(p);
      //      this->tankController.sendCommand(Tank::TCI_Aim, &p);
        //    if (turn)
          //  {
            //    this->tankController.sendCommand(Tank::TCI_Turn, &p);
           // }
        }
        break;
    case INPUT_MOUSEWHEEL:
        break;
    }
}
int checkCount;
bool g_bProfiler = false;
void MainGameState::OnFrame()
{
    PROFILE_BEGINPOINT(Frame);
    if (this->hge->Input_GetKeyState(HGEK_ESCAPE))
    {
        // RequestState("mainmenu");
        RequestState("exit");
    }
    hgeInputEvent event;
    while(this->hge->Input_GetEvent(&event))
    {
        ProcessControl(event);
    }

    //float fCurTime = this->hge->Timer_GetTime();

    int id = this->gui->Update(this->hge->Timer_GetDelta());
    switch(id)
    {
    case GID_BtnRand:
        //if (!tankController.isAttached())
        {
            //resetMyTank();
        }
        break;
    case GID_BtnStart:
        //if (!tankController.isAttached())
        {
           // resetMyTank();
        }
        break;
    case GID_BtnToggleProfiler:
        g_bProfiler = !g_bProfiler;
        break;
    }
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();

    PROFILE_CHECKPOINT(EntityManager.Step);
    EntityManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_CHECKPOINT(SoundManager.Step);

    SoundManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_ENDPOINT();
}

void MainGameState::OnRender()
{
    PROFILE_BEGINPOINT(Render);
    this->hge->Gfx_BeginScene(0);
    this->hge->Gfx_Clear(0);//黑色背景
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    this->hge->Gfx_SetTransform();
   // this->map->render();

    EntityManager::getSingleton().render(gameTime, deltaTime);
    iRenderQueue::getSingleton().flush();
    static char buf[256];
    sprintf(buf, "entity: %d", EntityManager::getSingleton().getCount());
    font->Render(100, 70, TA_LEFT, buf);
    sprintf(buf, "cpf: %d", checkCount);
    font->Render(100, 100, TA_LEFT, buf);
    sprintf(buf, "fps: %d", hge->Timer_GetFPS());
    font->Render(100, 130, TA_LEFT, buf);

    this->gui->Render();
#if USE_PROFILER
    if (g_bProfiler)
    {
        PROFILE_CHECKPOINT(ProfileRender);
        const vector<Profile::SumNode> &sn = Profile::getSingleton().getSummary();
        int x = 0;
        int y = 0;
        DWORD bgColor = ARGB(64, 0, 0, 0), fontColor = ARGB(255, 128, 255, 128);
        renderFontWithBk(*font, x, y, bgColor, fontColor, TA_LEFT, "name");
        y = 20;
        int rightMost = x;
        for (vector<Profile::SumNode>::const_iterator isn = sn.begin(); isn != sn.end(); ++isn)
        {
            size_t id = isn->parentIndex;
            int depth = 0;
            while(id < sn.size())
            {
                id = sn[id].parentIndex;
                depth++;
            }
            renderFontWithBk(*font, x + depth * 5, y, bgColor, fontColor, TA_LEFT, isn->name.c_str());
            int rightPos = (int)font->GetStringWidth(isn->name.c_str()) + x + depth * 5;
            if (rightPos > rightMost)
                rightMost = rightPos;

            y += 20;
        }
        x = rightMost + 5;
        y = 0;
        renderFontWithBk(*font, x, y, bgColor, fontColor, TA_LEFT,
            " totalTime(s)    max(ms)     logCount averTime(ms) lastTime(ms)");
        y += 20;
        for (vector<Profile::SumNode>::const_iterator isn = sn.begin(); isn != sn.end(); ++isn)
        {
            sprintf(buf, "%10.2f%10.2f%10d%10.2f%10.2f", isn->totalTime, isn->maxTime * 1000, isn->logCount, isn->totalTime * 1000 / isn->logCount, isn->lastTime * 1000);
            renderFontWithBk(*font, x, y, bgColor, fontColor, TA_LEFT, buf);
            y += 20;
        }
    }
#endif
    // 游戏状态信息
    this->hge->Gfx_EndScene();
    PROFILE_ENDPOINT();
}
