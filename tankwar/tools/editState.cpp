#pragma warning(disable: 4996)

#include <cmath>
#include <cassert>
#include <cctype>

#include <hgefont.h>
#include <hgesprite.h>
#include <hgeguictrls.h>
#include <hgecolor.h>

#include "common/entity.h"
#include "common/entityManager.h"
#include "common/soundManager.h"
#include "common/graphicEntity.h"
#include "common/profile.h"

#include "../src/game/tank.h"
#include "../src/game/weapon.h"
#include "../src/game/cannonBall.h"
#include "../src/game/bullet.h"
#include "../src/game/items.h"
#include "../src/game/map.h"
#include "../src/game/aiTankController.h"
#include "../src/game/radarMap.h"
#include "../src/game/targetIndicator.h"
#include "../src/game/cooldownIcon.h"

#include "editState.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) if (!a);else {delete a; a = 0;}
#endif

enum GUI_ID
{
    /// GUI main
    GID_BtnRand = 1,
    GID_BtnStart,

    NumGUIId,
};

bool TankCollisionChecker::checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo)
{
    TankContactInfo &tankContactInfo = (TankContactInfo &)contactInfo;
    size_t n = a->getBody().Collide(b->getBody(), deltaTime, tankContactInfo.getData(), 8);
    tankContactInfo.setNumber(n);
    checkCount++;
    return n > 0;
}
EditState::~EditState()
{
    assert(!this->gui);
    assert(!this->hge);
}

void EditState::OnEnter()
{
    this->hge = hgeCreate(HGE_VERSION);
    renderQueue.setWindowSize(Point2f((float)hge->System_GetState(HGE_SCREENWIDTH), (float)hge->System_GetState(HGE_SCREENHEIGHT)));

    this->font = new hgeFont("data/font.fnt");
    this->font->SetColor(ARGB(255, 0, 255, 0));

    this->system = new hgeCurvedAniSystem;
    this->animResManager = iSystem::GetInstance()->createAnimResManager();

    this->texGui = hge->Texture_Load("data/ui.png");
    this->gui = new hgeGUI;
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnRand, 10, 10, 36, 19, texGui, 0, 0));
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnStart, 50, 10, 39, 19, texGui, 0, 19));
    Rectf playRange;
    playRange.leftTop = Point2f(-2500, -2500);
    playRange.rightBottom = Point2f(2500, 2500);
    EntityManager::getSingleton().setPlayRange(playRange);
    EntityManager::getSingleton().setCollisionChecker(&tankCollisionChecker);
    EntityManager::getSingleton().setContactInfoBuffer(tankContactInfo);
    EntityManager::getSingleton().clear();

    this->map = new Map(*this->animResManager);
    
    Rectf range;
    range.leftTop = Point2f(20.0f, 20.0f);
    range.rightBottom = Point2f(1800.0f, 1600.0f);

    range.leftTop = Point2f(-2000.0f, -2000.0f);
    range.rightBottom = Point2f(2000.0f, 2000.0f);

    this->map->randomizeMap(range, 0, 500, 500, 0);
    EntityManager::getSingleton().attach(this->map);

    aiController = new AiTankController;
    aitankers.push_back(aiController);
    aiController->attacheMap(this->map);
    addAiTank(Point2f(0, 0), aiController);
}

void EditState::addAiTank(const Point2f &pos, AiTankController *controller/* = 0*/)
{
    Tank *tank = new Tank(*animResManager, font);
    tank->pos = pos;
    tank->SetBarbette(*Barbette::GetData(0), *Barbette::GetData(4));
    tank->SetCannon(*Cannon::GetData(0));
    tank->SetGun(*Gun::GetData(0));
    if (!controller)
    {
        controller = new AiTankController;
        aitankers.push_back(controller);
    }
    ((Tank::ControlEntity *)tank->getEntityInterface(Entity::EII_ControlEntity))->attach(*controller);
    EntityManager::getSingleton().attach(tank);
}

void EditState::OnLeave()
{
    aiController = 0;
    EntityManager::getSingleton().dettach(this->map);
    EntityManager::getSingleton().setCollisionChecker(0);
    SAFE_DELETE(this->map);
    for (list<AiTankController*>::iterator ia = aitankers.begin(); ia != aitankers.end(); ++ia)
    {
        delete *ia;
    }
    aitankers.clear();
    EntityManager::getSingleton().clear();
    SAFE_DELETE(this->gui);
    SAFE_DELETE(this->font);
    iSystem::GetInstance()->release(this->animResManager);
    SAFE_DELETE(this->system);
    if (this->texGui)
    {
        this->hge->Texture_Free(this->texGui);
        this->texGui = 0;
    }
    this->hge->Release();
    this->hge = 0;
}

int checkCount;
void EditState::OnFrame()
{
    PROFILE_BEGINPOINT(Frame);
    if (this->hge->Input_GetKeyState(HGEK_ESCAPE))
    {
        RequestState("exit");
    }
    hgeInputEvent event;
    while(this->hge->Input_GetEvent(&event))
    {
        switch(event.type)
        {
        case INPUT_KEYDOWN:
            switch(event.key)
            {
            case HGEK_LEFT:
                bLeft = true;
                break;
            case HGEK_RIGHT:
                bRight = true;
                break;
            case HGEK_UP:
                bUp = true;
                break;
            case HGEK_DOWN:
                bDown = true;
                break;
            }
            break;
        case INPUT_KEYUP:
            switch(event.key)
            {
            case HGEK_LEFT:
                bLeft = false;
                break;
            case HGEK_RIGHT:
                bRight = false;
                break;
            case HGEK_UP:
                bUp = false;
                break;
            case HGEK_DOWN:
                bDown = false;
                break;
            }
            break;
        case INPUT_MBUTTONDOWN:
            switch(event.key)
            {
            case HGEK_LBUTTON:
                if (map)
                {
                    switch(pathFindingOp)
                    {
                    case 0:
                        pathFindingFrom = Point2f(event.x, event.y);
                        RenderQueue::getSingleton().screenToGame(pathFindingFrom);
                        pathFindingOp = 1;
                        break;
                    case 1:
                        pathFindingTo = Point2f(event.x, event.y);
                        RenderQueue::getSingleton().screenToGame(pathFindingTo);
                        pathFindingOp = 0;
                        pathFindingResult.clear();
                        map->findPath(pathFindingResult, pathFindingFrom, pathFindingTo);
                        break;
                    }
                }
                break;
            case HGEK_RBUTTON:
                {
                    Point2f gotoPos(event.x, event.y);
                    RenderQueue::getSingleton().screenToGame(gotoPos);
                    aiController->resetOperates();

                    PROFILE_BEGINPOINT(AiGoto);
                    aiController->goTo(gotoPos);
                    PROFILE_ENDPOINT();
                }
                break;
            case HGEK_MBUTTON:
                break;
            }
            break;
        case INPUT_MBUTTONUP:
            break;
        case INPUT_MOUSEMOVE:
            bMouseLeft = event.x < 10;
            bMouseRight = event.x > hge->System_GetState(HGE_SCREENWIDTH) - 10;
            bMouseUp = event.y < 10;
            bMouseDown = event.y > hge->System_GetState(HGE_SCREENHEIGHT) - 10;
            break;
        case INPUT_MOUSEWHEEL:
            break;
        }
    }
    if (bLeft || bRight || bUp || bDown ||
        bMouseLeft || bMouseRight || bMouseUp || bMouseDown)
    {
        Point2f vp = RenderQueue::getSingleton().getViewerPos();
        if (bLeft || bMouseLeft)
        {
            vp.x -= 2;
        }
        if (bRight || bMouseRight)
        {
            vp.x += 2;
        }
        if (bUp || bMouseUp)
        {
            vp.y -= 2;
        }
        if (bDown || bMouseDown)
        {
            vp.y += 2;
        }
        RenderQueue::getSingleton().setViewerPos(vp);
    }
    int id = this->gui->Update(this->hge->Timer_GetDelta());
    switch(id)
    {
    case GID_BtnRand:
        break;
    case GID_BtnStart:
        break;
    }
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
	int timeConsumingMethodPerFrame = 3;
    for (list<AiTankController*>::iterator ic = aitankers.begin(); ic != aitankers.end(); ++ic)
    {
        AiTankController *controller = *ic;
        if (controller->think(gameTime, deltaTime, timeConsumingMethodPerFrame == 0))
		{
			timeConsumingMethodPerFrame--;
		}
        if (!controller->isAttached())
        {
            addAiTank(Point2f(rand() % 3800 - 1900.f, rand() % 3800 - 1900.f), controller);
        }
    }
    PROFILE_BEGINPOINT(EntityManagerStep);
    EntityManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_ENDPOINT();

    SoundManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_ENDPOINT();
}

void EditState::OnRender()
{
    PROFILE_BEGINPOINT(Render);
    this->hge->Gfx_BeginScene(0);
    this->hge->Gfx_Clear(0);//黑色背景
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    this->hge->Gfx_SetTransform();
    //this->map->render();

    switch(pathFindingOp)
    {
    case 1:
        RenderQueue::getSingleton().render(pathFindingFrom + Point2f(-2, 0), pathFindingFrom + Point2f(2, 0), ARGB(255, 255, 255, 255));
        RenderQueue::getSingleton().render(pathFindingFrom + Point2f(0, -2), pathFindingFrom + Point2f(0, 2), ARGB(255, 255, 255, 255));
        break;
    }
    if (!pathFindingResult.empty())
    {
        Point2f lastPoint = pathFindingResult.front();
        list<Point2f>::const_iterator ip = pathFindingResult.begin();
        for (++ip; ip != pathFindingResult.end(); ++ip)
        {
            RenderQueue::getSingleton().render(lastPoint, *ip, ARGB(192, 255, 64, 64));
            lastPoint = *ip;
        }
        RenderQueue::getSingleton().render(pathFindingFrom + Point2f(-2, 0), pathFindingFrom + Point2f(2, 0), ARGB(255, 255, 255, 255));
        RenderQueue::getSingleton().render(pathFindingFrom + Point2f(0, -2), pathFindingFrom + Point2f(0, 2), ARGB(255, 255, 255, 255));
        RenderQueue::getSingleton().render(pathFindingTo+ Point2f(-2, 0), pathFindingTo + Point2f(2, 0), ARGB(255, 255, 255, 255));
        RenderQueue::getSingleton().render(pathFindingTo + Point2f(0, -2), pathFindingTo + Point2f(0, 2), ARGB(255, 255, 255, 255));
    }

    EntityManager::getSingleton().render(gameTime, deltaTime);
    RenderQueue::getSingleton().flush();
    static char buf[256];
    sprintf(buf, "entity: %d", EntityManager::getSingleton().getCount());
    font->Render(100, 70, TA_LEFT, buf);
    sprintf(buf, "cpf: %d", checkCount);
    font->Render(100, 100, TA_LEFT, buf);
    sprintf(buf, "fps: %d", hge->Timer_GetFPS());
    font->Render(100, 130, TA_LEFT, buf);

    this->gui->Render();
#if USE_PROFILER
    PROFILE_CHECKPOINT(ProfileRender);
    const vector<Profile::SumNode> &sn = Profile::getSingleton().getSummary();
    float x = 0.f;
    float y = 0.f;
    sprintf(buf, "totalTime(s) max(ms) logCount averTime(ms) lastTime(ms)");
    font->Render(x, y, TA_LEFT, "name");
    font->Render(x + 100.f, y, TA_LEFT, buf);
    y += 20.f;
    for (vector<Profile::SumNode>::const_iterator isn = sn.begin(); isn != sn.end(); ++isn)
    {
        sprintf(buf, "%10.2f%10.2f%10d%10.2f%10.2f", isn->totalTime, isn->maxTime * 1000, isn->logCount, isn->totalTime * 1000 / isn->logCount, isn->lastTime * 1000);
        size_t id = isn->parentIndex;
        int depth = 0;
        while(id < sn.size())
        {
            id = sn[id].parentIndex;
            depth++;
        }
        font->Render(x + depth * 5, y, TA_LEFT, isn->name.c_str());
        font->Render(x + 100.f, y, TA_LEFT, buf);
        y += 20.f;
    }
#endif
    // 游戏状态信息
    this->hge->Gfx_EndScene();
    PROFILE_ENDPOINT();
}

void EditState::findPath(list<Point2f> &result, const Point2f &fromPos, const Point2f &toPos) const
{
    if (!map)
        return;

    map->findPath(result, fromPos, toPos);
}