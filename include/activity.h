#ifndef ACTIVITY_H
#define ACTIVITY_H

#define SRUTIL_DELEGATE_PREFERRED_SYNTAX
#include "srutil/delegate/delegate.hpp"

typedef srutil::delegate<void()> LoopDelegate;

class Activity
{
public:
    Activity();
    virtual ~Activity();

    virtual LoopDelegate getLoopDelegate() = 0;
};

#endif