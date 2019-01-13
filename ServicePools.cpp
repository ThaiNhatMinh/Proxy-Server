#include "pch.h"
#include "ServicePools.h"

ServicePool::ServicePool(int numpool)
{
	m_List.resize(numpool);
	int i=0;
	for (auto& el : m_List)
	{
		el.reset(new ClientConnection(i++));
	}

}

bool ServicePool::AddSerivce(Socket && s)
{
	auto iterator = FindSlot();
	if(iterator==m_List.end()) return false;

	(*iterator)->Active(std::move(s));

	return true;
}

std::vector<ClientConnection::Ptr>::iterator ServicePool::FindSlot()
{
	for (auto iterator = m_List.begin(); iterator != m_List.end(); iterator++)
		if ((*iterator)->GetState() == ClientConnection::STOP) return iterator;

	return m_List.end();
}
