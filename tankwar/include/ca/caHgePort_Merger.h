#ifndef _caHgePort_Merger_H_
#define _caHgePort_Merger_H_

#include "caHgePort_define.h"

/// 实现下列接口
class hgeStateMergerComposition : public iStateMergerComposition
{
public:
	hgeStateMergerComposition();
	virtual ~hgeStateMergerComposition();

	// 更换和设置融合方式
	virtual void setMergerStyle(cAni::StateId sid, cAni::MergeStyleId mid);
protected:
	// 容器类元素访问接口
	virtual iStateMergerIf *getMerger(cAni::StateId sid) const;
private:
	iStateMergerIf *mergers[NumOfHgeSI];
};

#endif//_caHgePort_Merger_H_
