// Filter "Graphics"

#pragma once

#include "Graphics_Buffer.hpp"
#include "Graphics_MeshPrimitives.hpp"
#include "Graphics_Pipeline.hpp"

#include "Common_Color.hpp"
#include "Common_Math.hpp"

#include <cstdint>
#include <span>

namespace RoseGold::Core::Graphics
{
	struct Triangle
	{
		std::uint32_t V1;
		std::uint32_t V2;
		std::uint32_t V3;
	};

	class Manager;
	class Mesh
	{
	public:
		Mesh(Manager& aGraphicsManager)
			: myGraphicsManager(aGraphicsManager)
			, myVertexCount(0)
		{ }
		virtual ~Mesh() = default;

		virtual std::vector<PipelineStateDescription::InputLayoutEntry> GetInputLayout() const = 0;

		std::shared_ptr<const GraphicsBuffer> GetVertexBuffer() const { return myVertexBuffer; }
		std::uint32_t GetVertexCount() { return myVertexCount; }

		virtual void SetFromPrimitive(MeshPrimitiveType aPrimitive) = 0;

	protected:
		std::shared_ptr<GraphicsBuffer> myVertexBuffer;
		std::uint32_t myVertexCount;

		Manager& myGraphicsManager;
	};

	class GenericMesh : public Mesh
	{
	public:
		struct Vertex
		{
			Math::Vector3 Position;
			Math::Vector3 Normal;
			Math::Vector3 Binormal;
			Math::Vector3 Tangent;
			Math::Vector2 UV;
		};

	public:
		GenericMesh(Manager& aGraphicsManager);

		std::vector<PipelineStateDescription::InputLayoutEntry> GetInputLayout() const override;

		void SetFromList(const std::span<Vertex> someVertices);
		void SetFromList(const std::span<Vertex> someVertices, const std::span<Triangle> someTriangles);

		void SetFromPrimitive(MeshPrimitiveType aType) override;
	};

	class ColoredMesh : public Mesh
	{
	public:
		struct Vertex
		{
			Math::Vector3 Position;
			Math::Vector3 Normal;
			Math::Vector3 Binormal;
			Math::Vector3 Tangent;
			Math::Vector2 UV;
			Color Color;
		};

	public:
		ColoredMesh(Manager& aGraphicsManager);

		std::vector<PipelineStateDescription::InputLayoutEntry> GetInputLayout() const override;

		void SetFromList(const std::span<Vertex> someVertices);
		void SetFromList(const std::span<Vertex> someVertices, const std::span<Triangle> someTriangles);

		void SetFromPrimitive(MeshPrimitiveType aType) override;
	};

	class SimpleMesh : public Mesh
	{
	public:
		struct Vertex
		{
			Math::Vector3 Position;
		};

	public:
		SimpleMesh(Manager& aGraphicsManager);

		std::vector<PipelineStateDescription::InputLayoutEntry> GetInputLayout() const override;

		void SetFromList(const std::span<Vertex> someVertices);
		void SetFromList(const std::span<Vertex> someVertices, const std::span<Triangle> someTriangles);

		void SetFromPrimitive(MeshPrimitiveType aType) override;
	};

	/*class FullscreenMesh : public SimpleMesh
	{
	public:
		FullscreenMesh(Manager& aGraphicsManager);
	};*/
}