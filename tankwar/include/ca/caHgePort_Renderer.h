#ifndef _caHgePort_Renderer_H_
#define _caHgePort_Renderer_H_

#include "caHgePort_define.h"
#include "caHgePort_ClipState.h"

class hgeRenderer : public iRenderer
{
public:
    hgeRenderer();
    virtual ~hgeRenderer();

    /// @fn getName
    /// @brief get renderer's unique name
    const char *getName() const
    {
        return "HGE";
    }

    /// @fn getStateId
    /// @brief return id from name
    StateId getStateId(const char *stateName) const;
    /// @fn getStateName
    /// @brief return name from id
    const char *getStateName(StateId statusId) const;

    /// @fn getInterpolateStyleId
    /// @brief return id from name
    virtual InterpolateStyleId getInterpolateStyleId(const char *isName) const;

    /// @fn getInterpolateStyleName
    /// @brief return name from id
    virtual const char *getInterpolateStyleName(InterpolateStyleId isId) const;

    /// @fn getClipState
    /// @brief get a working instance
    virtual iClipState *getClipState()
    {
        return &cs;
    }
    /// @fn getInterpolator
    /// @brief get a working instance
    virtual iCurveInterpolater *getInterpolator(StateId stateId, InterpolateStyleId isId) const;

    virtual void render(const iClipState &rClipState, const iClipState &rParentClipState);

	/// @fn createCurveDataSet
	/// @brief create a curved data set of this renderer
	virtual iCurveDataSet *createCurveDataSet(const char *name);

	/// @fn releaseCurveDataSet
	/// @brief release a curved data set
	virtual void releaseCurveDataSet(iCurveDataSet *cds);
protected:
    const char *_getStateTypeName(StateId statusId) const;

    hgeClipState cs;
    static HGE *hge;
    mutable std::map<std::pair<StateId, InterpolateStyleId>, iCurveInterpolater *> mapInterpolaters;
	//std::map<MergeStyleId, iStateMergerIf*> mapMergers;
};
#endif//_caHgePort_Renderer_H_