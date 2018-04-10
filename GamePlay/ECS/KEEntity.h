#ifndef _ENTITY_H__
#define _ENTITY_H__
#include <unordered_set>
#include <stdint.h>
#include "KEModule.h"


class EntityManager
{

public:
	static EntityManager& GetEntityManager(){
		static EntityManager l_manager;
		return l_manager;
	}

	__forceinline Entity CreateEntity() {
		while (IsEntityAlive(m_newbee))
		{
			m_newbee++;
		}
		m_entites.insert(m_newbee);
		return m_newbee;
		
	}

	__forceinline bool IsEntityAlive(Entity e) const{
 		return m_entites.find(e) != m_entites.end();
	}

	__forceinline void DestoryEntity(Entity e) {
		m_entites.erase(e);
	}

	__forceinline std::unordered_set<Entity>& GetAllEntities() {
		return m_entites;
	}

	~EntityManager() {
	
	}

	EntityManager(const EntityManager&) = delete;
	EntityManager& operator =(const EntityManager&) = delete;

private:
	EntityManager():m_newbee(0) {
	
	};
	

	

private:
	std::unordered_set<Entity> m_entites;
	Entity m_newbee;
};


#endif