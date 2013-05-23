#ifndef USE_PROFILER
#pragma message("USE_PROFILER not defined, assuming 0")
#define USE_PROFILER 0
#endif

#ifndef COMMON_PROFILE_H
#define COMMON_PROFILE_H

#if USE_PROFILER
#include <windows.h>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include "common/utils.h"

using std::string;
using std::vector;
using std::list;
using std::stack;
using std::map;
using std::pair;

class Profile : public Singleton<Profile>
{
public:
    Profile() : totalLogCount(0)
    {
        QueryPerformanceFrequency(&freq);
    }

    struct Node
    {
        string name;
        LARGE_INTEGER start;
        LARGE_INTEGER end;
        size_t sid;
        size_t logCount;
        size_t parentId;
    };
    struct SumNode
    {
        string name;
        float totalTime;
        float maxTime;
        float lastTime;
        size_t logCount;
        size_t parentIndex;
    };
    void EndLast();
    void Log(const char *name);
    void flushFrame()
    {
        assert(nodeStack.empty());
        totalLogCount++;
        nodes.clear();
    }
    void getResult(vector<pair<string, float> > &result) const;
    const vector<SumNode> &getSummary() const
    {
        return summary;
    }
protected:
    vector<Node> nodes;
    LARGE_INTEGER freq;
    size_t totalLogCount;

    map<string, size_t> maps;
    vector<SumNode> summary;

    stack<pair<size_t, size_t> > nodeStack; // sumid, nodeid
};

#define PROFILE_CHECKPOINT(a) Profile::getSingleton().EndLast(); Profile::getSingleton().Log(#a)
#define PROFILE_BEGINPOINT(a) Profile::getSingleton().Log(#a)
#define PROFILE_ENDPOINT() Profile::getSingleton().EndLast()

#else //USE_PROFILER
#define PROFILE_CHECKPOINT(a)
#define PROFILE_BEGINPOINT(a)
#define PROFILE_ENDPOINT()
#endif // USE_PROFILER

#endif//COMMON_PROFILE_H
