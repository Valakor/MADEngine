#include "angermanagement.h"

// Test includes to make sure that we can include files from other projects
#include <Core/GameEngine.h>
#include <EASTL/algorithm.h>
#include <yojimbo/yojimbo_allocator.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <rapidjson/document.h>
#include <assimp/scene.h>

AngerManagement::AngerManagement(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

AngerManagement::~AngerManagement()
{

}
