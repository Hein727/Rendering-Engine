#include "Skinned_Mesh.h"
#include "Misc.h"
#include "Graphics.h"
#include <functional>
#include <sstream>

Skinned_Mesh::Skinned_Mesh(const char* fbx_filename, bool triangulate)
{
	//Calling in device from graphics
	auto device = graphics::getInstance().GetDevice();
	
	//Create FbxManager 
	FbxManager* fbx_manager{ FbxManager::Create()};

	//Create FbxScene
	FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

	//Create Importer
	FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
	bool import_status{ false };
	import_status = fbx_importer->Initialize(fbx_filename);
	_ASSERT_EXPR(import_status, fbx_importer->GetStatus().GetErrorString());

	//Import to scene
	import_status = fbx_importer->Import(fbx_scene);
	_ASSERT_EXPR(import_status, fbx_importer->GetStatus().GetErrorString());

	//Triangulate (if needed)
	FbxGeometryConverter fbx_converter(fbx_manager);
	if (triangulate)
	{
		fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
		fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
	}

	//Recursive function to find node datas
	std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbx_node) {
		scene::node& node{ scene_view.nodes.emplace_back()};
		node.attribute = fbx_node->GetNodeAttribute() ?
			fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
		node.name = fbx_node->GetName();
		node.unique_id = fbx_node->GetUniqueID();
		node.parent_index = scene_view.indexof(fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0);
		for (int child_index = 0; child_index < fbx_node->GetChildCount(); child_index++)
		{
			traverse(fbx_node->GetChild(child_index));
		}
	} };

	//Getting the root node 
	traverse(fbx_scene->GetRootNode());

	//For testing delete this afterwards	
#if 1 
	for (const scene::node& node : scene_view.nodes)
	{
		FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };
		//Display node data in the output window as debug
		std::string node_name = fbx_node->GetName();
		uint64_t uid = fbx_node->GetUniqueID();
		uint64_t parent_uid = fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0;
		int32_t type = fbx_node->GetNodeAttribute() ? fbx_node->GetNodeAttribute()->GetAttributeType() : 0;
		std::stringstream debug_string;
		debug_string << node_name << ":" << uid << ":" << parent_uid << ":" << type << "\n";
		OutputDebugStringA(debug_string.str().c_str());
	}
#endif 
	//Destroy the FbxManager after use
	fbx_manager->Destroy();
}	