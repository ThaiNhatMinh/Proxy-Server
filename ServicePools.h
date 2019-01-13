#pragma once
#include "ClientConnection.h"


class ServicePool
{
private:
	std::vector<ClientConnection::Ptr> m_List;
public:
	ServicePool(int numpool);

	bool AddSerivce(Socket && s);

private:
	std::vector<ClientConnection::Ptr>::iterator FindSlot();
};