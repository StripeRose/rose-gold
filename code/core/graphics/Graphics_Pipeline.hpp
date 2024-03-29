#pragma once

#include "Graphics_Enums.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace RoseGold::Core::Graphics
{
	class Shader
	{
	public:
		enum class Type
		{
			Vertex,
			Pixel
		};
	};

	class PipelineStateDescription
	{
	public:
		struct InputLayoutEntry
		{
		public:
			InputLayoutEntry(std::string aSemanticName, GraphicsFormat aFormat)
				: SemanticName(aSemanticName)
				, Format(aFormat)
			{ }

			std::string SemanticName;
			unsigned int SemanticIndex = 0;
			GraphicsFormat Format;
		};

	public:
		inline bool IsValid() const { return VertexShader && PixelShader; }

	public:
		std::vector<InputLayoutEntry> InputLayout;

		// (IA) Input assembler

		// (VS) Vertex shader
		std::shared_ptr<Shader> VertexShader;

		// (HS) Hull shader

		// (TS) Tesselation

		// (DS) Domain shader

		// (GS) Geometry shader

		// (SO) Stream output

		// (RS) Rasterizer

		// (PS) Pixel shader
		std::shared_ptr<Shader> PixelShader;

		// (OM) Output merger
		std::vector<GraphicsFormat> OutputFormats;
		std::optional<GraphicsFormat> DepthTargetFormat;
	};

	class PipelineState
	{

	};
}