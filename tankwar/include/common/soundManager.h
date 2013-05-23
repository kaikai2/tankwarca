#ifndef COMMON_SOUND_MANAGER_H
#define COMMON_SOUND_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "caPoint2d.h"
#include "common/entity.h"
#include "common/utils.h"

using std::map;
using std::string;
using std::vector;
using cAni::Point2f;

class HGE;
class SoundManager : public Singleton<SoundManager>
{
public:
    SoundManager();
    virtual ~SoundManager();

    HEFFECT load(const char *name);
    void play(const char *name, const Point2f &pos, float volume = 1.0f);
    void step(float gameTime, float deltaTime);
protected:
    HGE *hge;

    map<string, HEFFECT> sounds;
    struct ChannelInfo
    {
        float startTime;
        float length;
        float volume;
        HCHANNEL channel;
        //bool bOnline;

        static bool CompareByVolume(const ChannelInfo &a, const ChannelInfo &b)
        {
            return a.volume > b.volume;
        }
    };
    float curTime;
    vector<ChannelInfo> channels;
};

#endif//COMMON_SOUND_MANAGER_H