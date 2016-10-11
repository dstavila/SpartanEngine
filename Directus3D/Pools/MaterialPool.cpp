/*
Copyright(c) 2016 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//= INCLUDES =================
#include "MaterialPool.h"
#include <vector>
#include "../IO/Log.h"
#include "../IO/Serializer.h"
#include "../IO/FileSystem.h"
//===========================

//= NAMESPACES ================
using namespace std;
using namespace Directus::Math;
//=============================

MaterialPool::MaterialPool(TexturePool* texturePool, ShaderPool* shaderPool)
{
	m_texturePool = texturePool;
	m_shaderPool = shaderPool;

	GenerateDefaultMaterials();
}

MaterialPool::~MaterialPool()
{
	Clear();
}

/*------------------------------------------------------------------------------
								[MISC]
------------------------------------------------------------------------------*/
// Adds a material to the pool directly from memory
Material* MaterialPool::Add(Material* materialIn)
{
	if (!materialIn)
	{
		LOG_WARNING("The material is null, it can't be added to the pool.");
		return nullptr;
	}

	// check for existing material from the same model
	for (Material* material : m_materials)
	{
		if (material->GetName() == materialIn->GetName())
			if (material->GetModelID() == materialIn->GetModelID())
				return material;
	}

	m_materials.push_back(materialIn);
	return m_materials.back();
}

// Adds multiple materials to the pool by reading them from files
void MaterialPool::Add(const vector<string>& filePaths)
{
	for (const string& filePath : filePaths)
	{
		// Make sure the path is valid
		if (!FileSystem::FileExists(filePath))
			continue;

		// Make sure it's actually a material file
		if (FileSystem::GetExtensionFromPath(filePath) != MATERIAL_EXTENSION)
			continue;

		// Create and load the material
		Material* material = new Material(m_texturePool, m_shaderPool);
		if (material->LoadFromFile(filePath))
			m_materials.push_back(material);
		else
			delete material;
	}
}

// Removes all the materials
void MaterialPool::Clear()
{
	for (Material* material : m_materials)
		delete material;

	m_materials.clear();
	m_materials.shrink_to_fit();
}

void MaterialPool::RemoveMaterial(Material* material)
{
	// make sure the material is not null
	if (!material)
		return;

	for (auto it = m_materials.begin(); it != m_materials.end();)
	{
		Material* mat = *it;
		if (mat->GetID() == material->GetID())
		{
			delete mat;
			it = m_materials.erase(it);
			return;
		}
		++it;
	}
}

void MaterialPool::RemoveMaterial(const string& materialID)
{
	Material* material = GetMaterialByID(materialID);
	RemoveMaterial(material);
}

Material* MaterialPool::GetMaterialByID(const string& materialID)
{
	if (materialID == MATERIAL_DEFAULT_ID)
		return m_materialDefault;

	if (materialID == MATERIAL_DEFAULT_SKYBOX_ID)
		return m_materialDefaultSkybox;

	for (Material* material : m_materials)
		if (material->GetID() == materialID)
			return material;

	return m_materialDefault;
}

Material* MaterialPool::GetMaterialStandardDefault()
{
	return m_materialDefault;
}

Material* MaterialPool::GetMaterialStandardSkybox()
{
	return m_materialDefaultSkybox;
}

vector<string> MaterialPool::GetAllMaterialFilePaths()
{
	vector<string> paths;

	for (Material* material : m_materials)
		paths.push_back(material->GetFilePath());

	return paths;
}

const vector<Material*>& MaterialPool::GetAllMaterials()
{
	return m_materials;
}

/*------------------------------------------------------------------------------
							[HELPER FUNCTIONS]
------------------------------------------------------------------------------*/
void MaterialPool::GenerateDefaultMaterials()
{
	m_materialDefault = new Material(m_texturePool, m_shaderPool);
	m_materialDefault->SetID(MATERIAL_DEFAULT_ID);
	m_materialDefault->SetName("Standard_Default");
	m_materialDefault->SetID("Standard_Material_0");
	m_materialDefault->SetColorAlbedo(Vector4(1, 1, 1, 1));

	// A texture must be loaded for that one, if all goes smooth
	// it's done by the skybox component
	m_materialDefaultSkybox = new Material(m_texturePool, m_shaderPool);
	m_materialDefaultSkybox->SetID(MATERIAL_DEFAULT_SKYBOX_ID);
	m_materialDefaultSkybox->SetName("Standard_Skybox");
	m_materialDefaultSkybox->SetID("Standard_Material_1");
	m_materialDefaultSkybox->SetFaceCullMode(CullNone);
	m_materialDefaultSkybox->SetColorAlbedo(Vector4(1, 1, 1, 1));
}
