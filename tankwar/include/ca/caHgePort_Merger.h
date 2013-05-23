#ifndef _caHgePort_Merger_H_
#define _caHgePort_Merger_H_

#include "caHgePort_define.h"

/// ʵ�����нӿ�
class hgeStateMergerComposition : public iStateMergerComposition
{
public:
	hgeStateMergerComposition();
	virtual ~hgeStateMergerComposition();

	// �����������ںϷ�ʽ
	virtual void setMergerStyle(cAni::StateId sid, cAni::MergeStyleId mid);
protected:
	// ������Ԫ�ط��ʽӿ�
	virtual iStateMergerIf *getMerger(cAni::StateId sid) const;
private:
	iStateMergerIf *mergers[NumOfHgeSI];
};

#endif//_caHgePort_Merger_H_
