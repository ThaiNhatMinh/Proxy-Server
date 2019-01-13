#include "EventManager.h"


int EventManager::Poll(int timeout)
{
    int r = poll(m_polls.data(), m_polls.size(),timeout);

    if(r<=0) return r;

    for(int i=0; i< m_polls.size(); i++)
    {
        if(m_polls[i].revents & POLLWRBAND)
        {
            m_Sks[i].callback(WRITE);
        }
        if(m_polls[i].revents & POLLIN)
        {
            m_Sks[i].callback(READ);
        }
        if(m_polls[i].revents & POLLOUT)
        {
            m_Sks[i].callback(WRITE);
        }
    }
}

void EventManager::Add(Socket* sk,int flag, std::function<void(EventFlag)>callback)
{
    InternalData data;
    data.sk = sk;
    data.callback = callback;
    m_Sks.push_back(data);

    pollfd fd;
    fd.fd = sk->GetHandle();
    fd.events = flag;

    m_polls.push_back(fd);
}

void EventManager::Remove(Socket* sk)
{
    std::vector<pollfd>::iterator rm2 = m_polls.begin();

    for(auto iterator = m_Sks.begin(); iterator!=m_Sks.end(), rm2!=m_polls.end(); iterator++,rm2++)
    {
        if(iterator->sk==sk)
        {
            m_Sks.erase(iterator);
            m_polls.erase(rm2);
            break;
        }
    }
}

void EventManager::Clear()
{
    m_polls.clear();
    m_Sks.clear();
}