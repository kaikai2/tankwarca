#include <hge.h>
#include <algorithm>

#include "common/soundManager.h"
#include "common/graphicEntity.h"
#include "common/iRenderQueue.h"

using std::sort;
using std::pair;

SoundManager::SoundManager() : hge(0)
{
    hge = hgeCreate(HGE_VERSION);
}

SoundManager::~SoundManager()
{
    for(map<string, HEFFECT>::iterator is = sounds.begin(); is != sounds.end(); ++is)
    {
        hge->Effect_Free(is->second);
    }
    sounds.clear();
    hge->Release();
}

HEFFECT SoundManager::load(const char *name)
{
    assert(name);
    assert(hge);

    string s = name;
    map<string, HEFFECT>::iterator is = sounds.find(s);
    if (is == sounds.end())
    {
        HEFFECT h = hge->Effect_Load(name);
        if (!h)
            return 0;
        sounds.insert(is, pair<string, HEFFECT>(s, h));
        return h;
    }
    else
        return is->second;
}

void SoundManager::play(const char *name, const Point2f &pos, float volume/* = 1.0f*/)
{
    Point2f viewPos = iRenderQueue::getSingleton().getViewerPos() + Point2f(400, 300);
    Point2f dpos = viewPos - pos;
    const float cMaxLen = 400;
    if (dpos.x > cMaxLen || dpos.x < -cMaxLen || dpos.y > cMaxLen || dpos.y < -cMaxLen)
        return;
    float len = dpos.Length();
    if (len > cMaxLen)
        return;
    HEFFECT h = load(name);
    ChannelInfo ci;
    ci.volume = ((cMaxLen - len) / 1) * volume;
    ci.channel = hge->Effect_PlayEx(h, (int)ci.volume);
    //hge->Channel_Pause(ci.channel);
    //ci.bOnline = true;
    ci.startTime = curTime;
    ci.length = hge->Channel_GetLength(ci.channel);
    channels.push_back(ci);
}

void SoundManager::step(float gameTime, float deltaTime)
{
    deltaTime;
    curTime = gameTime;

    vector<ChannelInfo>::iterator ic;
    for (ic = channels.begin(); ic != channels.end(); )
    {
        ChannelInfo &ci = *ic;
        if (gameTime > ci.startTime + ci.length)
            ic = channels.erase(ic);
        else
            ++ic;
    }
    /*
    sort(channels.begin(), channels.end(), ChannelInfo::CompareByVolume);
    size_t count = 2;
    for (ic = channels.begin(); count > 0 && ic != channels.end(); ++ic, --count)
    {
        ChannelInfo &ci = *ic;
        //BASS_ChannelIsActive(ci.channel)
        if (!hge->Channel_IsPlaying(ci.channel))
        {
            //ci.bOnline = true;
            hge->Channel_SetPos(ci.channel, gameTime - ci.startTime);
            hge->Channel_Resume(ci.channel);
            bool a = hge->Channel_IsPlaying(ci.channel);
            a = a;
        }
    }
    for (; ic != channels.end(); ++ic)
    {
        ChannelInfo &ci = *ic;
        if (hge->Channel_IsPlaying(ci.channel))
        {
            //ci.bOnline = false;
            hge->Channel_Pause(ci.channel);
        }
    }
    */
}