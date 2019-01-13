
#if __linux__
#include <errno.h>
#include <string.h>
#include <poll.h>
#endif
#include <vector>
#include <functional>
#include "Socket.h"
class EventManager
{
public:
    enum EventFlag
    {
        READ = 1,
        WRITE = 2,
    };
private:
    struct InternalData
    {
        Socket* sk;
        std::function<void(EventFlag)> callback;
    };
    std::vector<pollfd> m_polls;
    std::vector<InternalData> m_Sks;
public:
    EventManager()=default;

    int Poll(int timeout);
    void Add(Socket* sk,int flag, std::function<void(EventFlag)>callback);
    void Remove(Socket* sk);
    void Clear();
};
