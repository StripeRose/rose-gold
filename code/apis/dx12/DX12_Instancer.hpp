#pragma once

#include "Graphics_Manager.hpp"

#include <memory>

namespace RoseGold::DirectX12
{
	std::unique_ptr<Core::Graphics::Manager> CreateDX12Manager();
}