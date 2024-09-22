#pragma once

namespace pge
{
	class Application;

	class ApplicationLayer
	{
	protected:
		Application* App = nullptr;

		friend class Application;
	public:
		ApplicationLayer();
		virtual ~ApplicationLayer();

		virtual void Setup() = 0;
		virtual void Update() = 0;
	};
}