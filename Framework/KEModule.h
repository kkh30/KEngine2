#ifndef _COMPONENT_H__
#define _COMPONENT_H__
#include <array>
#include <unordered_map>
#include <assert.h>
using ComponentID = uint32_t;
using Entity = uint32_t;



template<typename T>
class Module
{
	
	

protected:
	Module() {
	
	};

public:

	static T& Instance() {
		assert(IsStartUp() && "Module Not StartUp Yet!\n");
		assert(!IsShutDown() && "Module ShutDown Already!\n");
		return *InstancePtr();
	}
	static T*& InstancePtr() {
		static T* inst_ptr = nullptr;
		return inst_ptr;
	}

	Module(const Module&) = delete;
	Module operator=(const Module&) = delete;

	virtual ~Module() {
	
	};

	//template<typename T>
	__forceinline void AddEntityComponent(Entity e,const T& p_t) {
		m_components.insert(std::pair<Entity, T>(e,p_t));
	};

	template <typename ...Args>
	static void StartUp(Args&& ...p_args) {
		assert(!IsStartUp() && "Module StartUp Already!\n");
		InstancePtr() = new T(std::forward<Args>(p_args)...);
		InstancePtr()->OnStartUp();
		IsStartUp() = true;
	}


	//template<typename T>
	__forceinline T& GetEntityComponent(Entity e) {
		return m_components[e];
	};

	
	static void ShutDown() {
		assert(IsStartUp() && "Module Not StartUp Yet!\n");
		assert(!IsShutDown() && "Module ShutDown Already!\n");
		InstancePtr()->OnShutDown();
		delete InstancePtr();
		IsShutDown() = true;
	};

	virtual void Update() {
		//assert(IsStartUp() && "Module Not StartUp Yet!\n");
		//assert(!IsShutDown() && "Module ShutDown Already!\n");
	};

	


protected:

	virtual void OnStartUp() {

	}
	virtual void OnShutDown() {

	}

	static bool& IsStartUp() {
		static bool is_start_up = false;
		return is_start_up;
	}

	static bool& IsShutDown() {
		static bool is_shut_down = false;
		return is_shut_down;
	}

	
	static std::unordered_map<Entity, T> m_components;
};
#endif // !_COMPONENT_H__
