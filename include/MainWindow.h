#pragma once

#include <UI.h>
#include <Canvas.h>
#include <string>

class MainWindow : public UI::GUI 
{
	UI::Container * root;
	Canvas * canvas;


	class MyButton : public UI::Button
	{
		MainWindow * mainWindow;
		virtual bool onMouseButtonReleased(unsigned int button) override;
	public:
		MyButton(MainWindow * mw, std::string text_);
	};

	class InputToggleButton : public UI::Button
	{
		bool useMouse = true;

		virtual bool onMouseButtonReleased(unsigned int button) override;

		virtual std::string const& getText();
	public:
		InputToggleButton();
	};

	class LayerMoveUpButton : public UI::Button
	{
		MainWindow * mainWindow;
		Layer * layer;

		virtual bool onMouseButtonReleased(unsigned int button) override;
	public:
		LayerMoveUpButton(MainWindow * mw, Layer * layer_);
	};

	class LayerMoveDownButton : public UI::Button
	{
		MainWindow * mainWindow;
		Layer * layer;

		virtual bool onMouseButtonReleased(unsigned int button) override;
	public:
		LayerMoveDownButton(MainWindow * mw, Layer * layer_);
	};


	class LayerButton : public UI::Label
	{
		MainWindow * mainWindow;
		Layer * layer;

		virtual bool onMouseButtonReleased(unsigned int button) override;

		virtual uint32_t getBackGroundColour() override;
	public:
		LayerButton(MainWindow * mw, Layer * layer_);
	};

	class LayerVisibilityButton : public UI::Label
	{
		MainWindow * mainWindow;
		Layer * layer;

		virtual bool onMouseButtonReleased(unsigned int button) override;

		virtual std::string const& getText();
	public:
		LayerVisibilityButton(MainWindow * mw,Layer * layer_);
	};

	class ColourSetter : public UI::Button
	{
		float colour[3];

		virtual bool onMouseButtonReleased(unsigned int button) override;

	public:
		ColourSetter(std::string t_, float r, float g, float b);
	};

	class QuitButton : public UI::Button
	{
		virtual bool onMouseButtonReleased(unsigned int button) override;
	public:
		QuitButton(std::string text_);
	};

public:
	MainWindow(Canvas * canvas);
	UI::Container * getRoot() override;
};
