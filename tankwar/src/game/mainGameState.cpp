#pragma warning(disable: 4996)

#include <cmath>
#include <cassert>
#include <cctype>

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

#include "tank.h"
#include "weapon.h"
#include "cannonBall.h"
#include "bullet.h"
#include "items.h"
#include "map.h"
#include "aiTankController.h"
#include "radarMap.h"
#include "targetIndicator.h"
#include "cooldownIcon.h"

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

bool TankCollisionChecker::checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo)
{
    TankContactInfo &tankContactInfo = (TankContactInfo &)contactInfo;
    size_t n = a->getBody().Collide(b->getBody(), deltaTime, tankContactInfo.getData(), 8);
    tankContactInfo.setNumber(n);
    checkCount++;
    return n > 0;
}
// FIXME: hacker solution
Tank *gTank = 0;

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
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnRand, 10, 100, 36, 19, texGui, 0, 0));
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnStart, 10, 140, 39, 19, texGui, 0, 19));
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnToggleProfiler, 10, 180, 39, 19, texGui, 0, 38));
    
    Rectf playRange;
    playRange.leftTop = Point2f(-2500, -2500);
    playRange.rightBottom = Point2f(2500, 2500);
    EntityManager::getSingleton().setPlayRange(playRange);
    EntityManager::getSingleton().setCollisionChecker(&tankCollisionChecker);
    EntityManager::getSingleton().setContactInfoBuffer(tankContactInfo);
    EntityManager::getSingleton().clear();

    targetIndicator = new TargetIndicator(*animResManager);
    EntityManager::getSingleton().attach(targetIndicator);
    targetIndicator->init("data/tank/indicator.xml");

    gunCooldown = new CooldownIcon(*animResManager);
    cannonCooldown = new CooldownIcon(*animResManager);
    gunCooldown->init("data/ui/gun.xml", "data/ui/mask.xml");
    gunCooldown->pos = Point2f(50, 50);
    cannonCooldown->init("data/ui/cannon.xml", "data/ui/mask.xml");
    cannonCooldown->pos = Point2f(50 + 65, 50);
    EntityManager::getSingleton().attach(gunCooldown);
    EntityManager::getSingleton().attach(cannonCooldown);

    // add tank
    resetMyTank();

    this->radar = new RadarMap(*animResManager);
    EntityManager::getSingleton().attach(radar);
    this->radar->init("data/radar/radar.xml");

    float windowWidth = iRenderQueue::getSingleton().getWindowSize().x;
    this->radar->pos = Point2f(windowWidth - 110, 110);

    this->map = new Map(*animResManager);
    
    Rectf range;
    range.leftTop = Point2f(-1000.0f, -700.0f);
    range.rightBottom = Point2f(1000.0f, 700.0f);

    this->map->randomizeMap(range, 0, 100, 250, 100);
    EntityManager::getSingleton().attach(this->map);

    // add ai tank
    //addAiTank(Point2f(100, 100));

    Point2f pos;
    for (int i = 0; i < 50; i++)
    {
        map->getRandomPos(pos);
        addAiTank(pos);
    }
    for (int i = 0; i < 100; i++)
    {
        map->getRandomPos(pos);
        addRandomItem(pos);
    }
}
void MainGameState::resetMyTank()
{
    // add tank
    Tank *tank = new Tank(*animResManager, font);
    tank->pos = Point2f(400, 300);
    iRenderQueue::getSingleton().setViewer(tank);

    tank->SetBarbette(*Barbette::GetData(0), *Barbette::GetData(4));
    tank->SetCannon(*Cannon::GetData(0));
    tank->SetGun(*Gun::GetData(0));
    tank->bShowAim = true;

    ((Tank::ControlEntity *)tank->getEntityInterface(Entity::EII_ControlEntity))->attach(tankController);

    EntityManager::getSingleton().attach(tank);
    tank->attachCooldownIcon(gunCooldown, cannonCooldown);
    gTank = tank;
}
void MainGameState::addRandomItem(const Point2f &pos)
{
    Item *item = 0;
    switch(rand() % 8)
    {
    case 0:
        item = new ItemHP(*animResManager);
        break;
    case 1:
        item = new ItemArmor(*animResManager);
        break;
    case 2:
        item = new ItemC1(*animResManager);
        break;
    case 3:
        item = new ItemC2(*animResManager);
        break;
    case 4:
        item = new ItemC3(*animResManager);
        break;
    case 5:
        item = new ItemC4(*animResManager);
        break;
    case 6:
        item = new ItemG1(*animResManager);
        break;
    case 7:
        item = new ItemG2(*animResManager);
        break;
    }
    if (item)
    {
        item->setPosition(pos);
        EntityManager::getSingleton().attach(item);
    }
}

void MainGameState::addAiTank(const Point2f &pos, AiTankController *controller/* = 0*/)
{
    Tank *tank = new Tank(*animResManager, font);
    tank->pos = pos;
    ((iAttacheeEntity *)tank->getEntityInterface(Tank::EII_RadarEntity))->attach(*this->radar);
    ((iAttacheeEntity *)tank->getEntityInterface(Tank::EII_TargetEntity))->attach(*this->targetIndicator);
    tank->SetBarbette(*Barbette::GetData(0), *Barbette::GetData(4));
    tank->SetCannon(*Cannon::GetData(0));
    tank->SetGun(*Gun::GetData(0));
    if (!controller)
    {
        controller = new AiTankController;
        aitankers.push_back(controller);
        controller->attacheMap(this->map);
    }
    ((Tank::ControlEntity *)tank->getEntityInterface(Entity::EII_ControlEntity))->attach(*controller);
    EntityManager::getSingleton().attach(tank);
}

void MainGameState::OnLeave()
{
    EntityManager::getSingleton().dettach(this->map);
    EntityManager::getSingleton().dettach(this->targetIndicator);
    EntityManager::getSingleton().dettach(this->radar);
    EntityManager::getSingleton().dettach(this->gunCooldown);
    EntityManager::getSingleton().dettach(this->cannonCooldown);
    SAFE_DELETE(this->gunCooldown);
    SAFE_DELETE(this->cannonCooldown);
    SAFE_DELETE(this->targetIndicator);
    SAFE_DELETE(this->radar);
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

void MainGameState::ProcessTankControl(const hgeInputEvent &event)
{
    static bool turn = false;
    switch(event.type)
    {
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
    case INPUT_MOUSEMOVE:
        {
            UpdateTankAim(turn);
        }
        break;
    case INPUT_MOUSEWHEEL:
        break;
    }
}

void MainGameState::UpdateTankAim( bool turn ) 
{
    float x, y;
    hge->Input_GetMousePos(&x, &y);
    Point2f p(x, y);
    iRenderQueue::getSingleton().screenToGame(p);
    this->tankController.sendCommand(Tank::TCI_Aim, &p);
    if (turn)
    {
        this->tankController.sendCommand(Tank::TCI_Turn, &p);
    }
}
int checkCount;
bool g_bProfiler = false;
void MainGameState::OnFrame()
{
    ++frameCount;
        
    PROFILE_BEGINPOINT(Frame);
    if (this->hge->Input_GetKeyState(HGEK_ESCAPE))
    {
        // RequestState("mainmenu");
        RequestState("exit");
    }
    hgeInputEvent event;
    while(this->hge->Input_GetEvent(&event))
    {
        ProcessTankControl(event);
    }

    //float fCurTime = this->hge->Timer_GetTime();

    int id = this->gui->Update(this->hge->Timer_GetDelta());
    switch(id)
    {
    case GID_BtnRand:
        if (!tankController.isAttached())
        {
            resetMyTank();
        }
        break;
    case GID_BtnStart:
        if (!tankController.isAttached())
        {
            resetMyTank();
        }
        break;
    case GID_BtnToggleProfiler:
        g_bProfiler = !g_bProfiler;
        break;
    }
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    //if (frameCount % 2)
    {
        tankCollisionChecker.deltaTime = deltaTime;
        PROFILE_BEGINPOINT(AiTankThink);
        int nTimeconsumingMethodPerFrame = 3;
        for (list<AiTankController*>::iterator ic = aitankers.begin(); ic != aitankers.end(); ++ic)
        {
            AiTankController *controller = *ic;
            if (controller->think(gameTime, deltaTime, nTimeconsumingMethodPerFrame == 0))
                nTimeconsumingMethodPerFrame--;
            if (!controller->isAttached())
            {
                Point2f pos;
                map->getRandomPos(pos);
                addAiTank(pos, controller);
            }
        }
        PROFILE_ENDPOINT();
        if (Item::getInstanceCount() < 100)
        {
            Point2f pos;
            map->getRandomPos(pos);
            addRandomItem(pos);
        }
        checkCount = tankCollisionChecker.checkCount;
        tankCollisionChecker.checkCount = 0;
        PROFILE_BEGINPOINT(EntityManagerStep);
        EntityManager::getSingleton().step(gameTime, deltaTime);
        PROFILE_ENDPOINT();
    }

    if (!tankController.isAttached())
    {
        gTank = 0;
        iRenderQueue::getSingleton().setViewer(0);
    }
    SoundManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_ENDPOINT();
}

void MainGameState::OnRender()
{
    PROFILE_BEGINPOINT(Render);
    this->hge->Gfx_BeginScene(0);
    //this->hge->Gfx_Clear(0);//黑色背景
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    this->hge->Gfx_SetTransform();
    //this->map->render();

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

CannonBall *MainGameState::createCannonBall()
{
    CannonBall *cannonBall = new CannonBall(*this->animResManager);
    EntityManager::getSingleton().attach(cannonBall);
    return cannonBall;
}

Bullet *MainGameState::createBullet()
{
    Bullet *bullet = new Bullet(*this->animResManager);
    EntityManager::getSingleton().attach(bullet);
    return bullet;
}
void MainGameState::findPath(list<Point2f> &result, const Point2f &fromPos, const Point2f &toPos) const
{
    if (!map)
        return;

    map->findPath(result, fromPos, toPos);
}
