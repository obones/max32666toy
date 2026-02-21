#include "activity.h"
//#include "fd/delegate.hpp"

#include "FastFunc.hpp"

#define SRUTIL_DELEGATE_PREFERRED_SYNTAX
#include "srutil/delegate/delegate.hpp"
#include "CppDelegates/Delegate.h"

typedef ssvu::FastFunc<void()> FFLoopDelegate;
typedef srutil::delegate<void()> LoopDelegate;
typedef SA::delegate<void()> SALoopDelegate;

class EmptyActivity : public Activity
{
private:
    int index;
public:
    EmptyActivity(int index);

    void loop();
    void nonVirtualLoop();

    LoopDelegate getLoopMethod();

    FFLoopDelegate getFFLoopMethod();

    SALoopDelegate getSALoopMethod();
};