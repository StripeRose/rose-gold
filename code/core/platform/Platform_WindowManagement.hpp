#pragma once

#include <rose-common/EventSlot.hpp>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace RoseGold::Core::Platform
{
	class Window
	{
	public:
		virtual ~Window() = default;

	public:
		RoseCommon::EventSlot<> OnClosed;
		RoseCommon::EventSlot<bool&> OnClosing;
		RoseCommon::EventSlot<bool> OnFocusChanged;
		RoseCommon::EventSlot<int, int> OnMove;
		RoseCommon::EventSlot<int, int> OnSizeChanged;

	public:
		/// <summary>
		/// Requests the window to close.
		/// Can be cancelled by the OnClosing event.
		/// </summary>
		virtual void Close() = 0;

		/// <summary>
		/// Fetches a platform's native windowhandle, if any, for use with systems that require such.
		/// </summary>
		/// <returns>An std::any containing the native handle.</returns>
		virtual std::any GetNativeHandle() const = 0;

		virtual void GetSize(int& outWidth, int& outHeight) const = 0;
	};

	class WindowManager
	{
	public:
		struct CreationParameters
		{
			std::string Title = "New window";
			std::optional<std::pair<unsigned int, unsigned int>> Size;
		};

	public:
		virtual ~WindowManager() = default;

		virtual std::shared_ptr<Window> NewWindow(const CreationParameters& someParameters) = 0;

		virtual std::vector<std::shared_ptr<Window>> GetWindows() const = 0;

		virtual void Update() = 0;

		virtual bool SupportsMultipleWindows() const = 0;
	};
}