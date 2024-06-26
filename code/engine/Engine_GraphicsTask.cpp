// Filter "Graphics"
#include "stdafx.hpp"

#include "Engine_GraphicsTask.hpp"

#include "Common_Diagnostics.hpp"

#include <algorithm>

//namespace RoseGold::Core::Graphics
//{
//	GraphicsTask::GraphicsTask(const char* aName)
//		: myName(aName)
//	{ }
//
//	void GraphicsTask::ResolveWorkTasks(std::vector<const GraphicsTask*>& outTasks) const
//	{
//		GetWorkTasks(outTasks);
//		std::sort(outTasks.begin(), outTasks.end(), [](const GraphicsTask* a, const GraphicsTask* b) {
//			return b->DependsOn(*a);
//			});
//	}
//
//	void GraphicsTask::AddDependency(GraphicsTask& aTask)
//	{
//		Debug::Assert(!aTask.DependsOn(*this), "Tasks doesn't create a circular dependency.");
//		myDependencies.push_back(&aTask);
//	}
//
//	CommandBuffer& GraphicsTask::AddWork()
//	{
//		Debug::Assert(myTasks.empty(), "The task doesn't contain sub-tasks.");
//		return *myCommandBuffers.emplace_back(new CommandBuffer()).get();
//	}
//
//	GraphicsTask& GraphicsTask::CreateTask()
//	{
//		Debug::Assert(myCommandBuffers.empty(), "The task doesn't contain work.");
//		return *myTasks.emplace_back(std::make_unique<GraphicsTask>()).get();
//	}
//
//	GraphicsTask& GraphicsTask::CreateTask(const char* aTaskName)
//	{
//		Debug::Assert(myCommandBuffers.empty(), "The task doesn't contain work.");
//		return *myTasks.emplace_back(std::make_unique<GraphicsTask>(aTaskName)).get();
//	}
//
//	bool GraphicsTask::DependsOn(const GraphicsTask& aTask) const
//	{
//		for (const GraphicsTask* dependency : myDependencies)
//		{
//			if (dependency == &aTask)
//				return true;
//
//			if (dependency->DependsOn(aTask))
//				return true;
//		}
//
//		for (const std::unique_ptr<GraphicsTask>& subTask : myTasks)
//		{
//			if (subTask.get() == &aTask)
//				return true;
//
//			if (subTask->DependsOn(aTask))
//				return true;
//		}
//
//		return false;
//	}
//
//	GraphicsTask* GraphicsTask::FindTask(const char* aTaskName)
//	{
//		if (myName == aTaskName)
//			return this;
//
//		for (const std::unique_ptr<GraphicsTask>& subTask : myTasks)
//		{
//			GraphicsTask* foundTask = subTask->FindTask(aTaskName);
//			if (foundTask != nullptr)
//				return foundTask;
//		}
//
//		return nullptr;
//	}
//
//	const GraphicsTask* GraphicsTask::FindTask(const char* aTaskName) const
//	{
//		if (myName == aTaskName)
//			return this;
//
//		for (const std::unique_ptr<GraphicsTask>& subTask : myTasks)
//		{
//			const GraphicsTask* foundTask = subTask->FindTask(aTaskName);
//			if (foundTask != nullptr)
//				return foundTask;
//		}
//
//		return nullptr;
//	}
//
//	bool GraphicsTask::HasWork() const
//	{
//		if (!myCommandBuffers.empty())
//			return true;
//
//		for (const std::unique_ptr<GraphicsTask>& task : myTasks)
//		{
//			if (task->HasWork())
//				return true;
//		}
//
//		return false;
//	}
//
//	void GraphicsTask::GetWorkTasks(std::vector<const GraphicsTask*>& outTasks) const
//	{
//		for (const std::unique_ptr<GraphicsTask>& task : myTasks)
//		{
//			if (!task->myCommandBuffers.empty())
//				outTasks.push_back(task.get());
//			else
//				task->GetWorkTasks(outTasks);
//		}
//	}
//}
