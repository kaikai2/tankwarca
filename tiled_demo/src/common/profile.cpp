#include "common/profile.h"
#if USE_PROFILER

void Profile::EndLast()
{
    if (nodes.empty())
        return;
    assert(!nodeStack.empty());
    size_t nid = nodeStack.top().second;
#ifndef NDEBUG
    size_t sid = nodeStack.top().first;
#endif
    nodeStack.pop();

    Node &n = nodes[nid];
    assert(sid == n.sid);
    QueryPerformanceCounter(&n.end);
    assert(n.sid < summary.size());
    SumNode &sn = summary[n.sid];
    sn.logCount++;
    float time = float(double(n.end.QuadPart - n.start.QuadPart) / double(freq.QuadPart));
    sn.lastTime = time;
    sn.totalTime += time;
    if (time > sn.maxTime)
        sn.maxTime = time;
}

void Profile::Log(const char *name)
{
    //EndLast();
    Node n;
    n.parentId = size_t(-1);
    if (!nodeStack.empty())
    {
        n.parentId = nodeStack.top().second;
    }
    n.name = name;
    QueryPerformanceCounter(&n.start);
    map<string, size_t>::iterator im = maps.find(n.name);
    if (im == maps.end())
    {
        SumNode sn;
        sn.name = n.name;
        sn.totalTime = 0;
        sn.maxTime = 0;
        sn.logCount = 0;
        sn.lastTime = 0;
        maps[sn.name] = n.sid = summary.size();
        if (n.parentId == size_t(-1))
            sn.parentIndex = size_t(-1);
        else
            sn.parentIndex = nodes[n.parentId].sid;
        summary.push_back(sn);
    }
    else
    {
        n.sid = im->second;
        if (n.parentId == size_t(-1))
        {
            if (summary[n.sid].parentIndex != size_t(-1))
                summary[n.sid].parentIndex = size_t(-2);
        }
        else
        {
            if (summary[n.sid].parentIndex != nodes[n.parentId].sid)
                summary[n.sid].parentIndex = size_t(-2);
        }
    }
    nodeStack.push(pair<size_t, size_t>(n.sid, nodes.size()));
    nodes.push_back(n);
}

void Profile::getResult(vector<pair<string, float> > &result) const
{
    result.clear();
    result.reserve(nodes.size());
    for (vector<Node>::const_iterator in = nodes.begin(); in != nodes.end(); ++in)
    {
        float time = float(double(in->end.QuadPart - in->start.QuadPart) / double(freq.QuadPart));
        result.push_back(pair<string, float>(in->name, time));
    }
}
#endif