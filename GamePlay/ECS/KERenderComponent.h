//#ifndef _KERENDERER_COMPONENT_H__
//#define _KERENDERER_COMPONENT_H__
//#include <stdint.h>
//#include "KEMemorySystem.h"
//#define GLM_FORCE_AVX2
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include "glm\glm.hpp"
//#include "glm\matrix.hpp"
//#include "glm\gtx\matrix_operation.hpp"
//#include "glm\gtc\matrix_transform.hpp"
//
//struct KEVertex
//{
//	float position[3];
//	float color[3];
//};
//
//
//
//
//struct KERenderComponent
//{
//	//uint64_t vertex_offset_in_byte;
//	//uint64_t index_offset_in_byte;
//	uint64_t vertex_size;
//	uint64_t index_size;
//	uint64_t vertex_count;
//	uint64_t index_count;
//	//Todo::Replace With GameScene Info
//	static uint64_t total_vertex_num;
//	static uint64_t total_index_num;
//	uint64_t first_vertex;
//	uint64_t first_index;
//
//	
//	float material[4];
//
//	
//
//	KERenderComponent(std::vector<KEVertex>& p_vertices, std::vector<uint32_t>& p_indices) :
//		vertex_size(sizeof(KEVertex) * p_vertices.size()),
//		index_size(sizeof(uint32_t) * p_indices.size()),
//		index_count(p_indices.size()),
//		vertex_count(uint32_t(p_vertices.size())),
//		first_index(0),
//		first_vertex(0),
//		//local_transform(),
//		material()
//	{
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		l_memory_system.UploadIndexDataToVRAM(p_indices.data(), index_size);
//		first_index = total_index_num;
//		first_vertex = total_vertex_num;
//		total_index_num += index_count;
//		total_vertex_num += vertex_count;
//	}
//
//	KERenderComponent(std::vector<KEVertex>& p_vertices) :
//		vertex_size(sizeof(KEVertex) * p_vertices.size()),
//		index_size(0),
//		index_count(0),
//		vertex_count(uint32_t(p_vertices.size())),
//		first_index(0),
//		first_vertex(0),
//		//local_transform(),
//		material()
//
//	{
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		first_vertex = total_vertex_num;
//		total_vertex_num += vertex_count;
//	}
//
//
//	KERenderComponent(std::vector<KEVertex>&& p_vertices,std::vector<uint32_t>&& p_indices) :
//		vertex_size(sizeof(KEVertex) * p_vertices.size()),
//		index_size(sizeof(uint32_t) * p_indices.size()),
//		index_count(p_indices.size()),
//		vertex_count(uint32_t(p_vertices.size())),
//		first_index(0),
//		first_vertex(0),
//		//local_transform(),
//		material()
//
//	{
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		l_memory_system.UploadIndexDataToVRAM(p_indices.data(), index_size);
//		first_index = total_index_num;
//		first_vertex = total_vertex_num;
//		total_index_num += index_count;
//		total_vertex_num += vertex_count;
//	}
//
//	KERenderComponent(std::vector<KEVertex>&& p_vertices) :
//		vertex_size(sizeof(KEVertex) * p_vertices.size()),
//		index_size(0),
//		index_count(0),
//		vertex_count(uint32_t(p_vertices.size())),
//		first_index(0),
//		first_vertex(0),
//		//local_transform(),
//		material()
//
//	{
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		first_vertex = total_vertex_num;
//		total_vertex_num += vertex_count;
//	}
//
//
//	KERenderComponent():
//		vertex_size(0),
//		index_size(0),
//		index_count(0),
//		vertex_count(0),
//		first_index(0),
//		first_vertex(0),
//		//local_transform(),
//		material()
//
//	{
//	}
//
//	void AddVertices(std::vector<KEVertex>&& p_vertices) {
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		vertex_count = p_vertices.size();
//		vertex_size = vertex_count * sizeof(KEVertex);
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		first_vertex = total_vertex_num;
//		total_vertex_num += vertex_count;
//
//	}
//
//	void AddIndices(std::vector<uint32_t>&& p_indices) {
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		index_count = p_indices.size();
//		index_size = index_count * sizeof(uint32_t);
//		l_memory_system.UploadIndexDataToVRAM(p_indices.data(), index_size);
//		first_index = total_index_num;
//		total_index_num += index_count;
//
//	}
//
//	void AddVertices(std::vector<KEVertex>& p_vertices) {
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		vertex_count = p_vertices.size();
//		vertex_size = vertex_count * sizeof(KEVertex);
//		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
//		first_vertex = total_vertex_num;
//		total_vertex_num += vertex_count;
//
//	}
//
//	void AddIndices(std::vector<uint32_t>& p_indices) {
//		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
//		index_count = p_indices.size();
//		index_size = index_count * sizeof(uint32_t);
//		l_memory_system.UploadIndexDataToVRAM(p_indices.data(), index_size);
//		first_index = total_index_num;
//		total_index_num += index_count;
//
//	}
//
//	
//	void SetMaterial(float* p_material) {
//		memcpy(material, p_material, sizeof(float) * 4);
//	}
//};
//
//
//
//
//
//
//
//
//
//
//
//
//#endif