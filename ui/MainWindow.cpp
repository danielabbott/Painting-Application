#include <MainWindow.h>
#include <iostream>
#include <Window.h>

using namespace std;



bool MainWindow::MyButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		// clear_layer(get_first_layer()->next);
		mainWindow->canvasView->getCanvas()->fillLayer(mainWindow->canvasView->getCanvas()->getFirstLayer(), 0xffffffff);
	}
	return true; 
}

MainWindow::MyButton::MyButton(MainWindow * mw, string text_)
: UI::Button(text_, LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment::TOP_BOTTOM_CENTRE), mainWindow(mw) {}



bool MainWindow::InputToggleButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		useMouse = !useMouse;
		if(!tablet_detected()) {
			useMouse = true;
		}
		set_canvas_input_device(useMouse);
	}
	return true; 
}

string const& MainWindow::InputToggleButton::getText()
{
	static string mouseString = "Mouse";
	static string tabletString = "Tablet";
	if(useMouse) {
		return mouseString;
	}
	else {
		return tabletString;
	}
}

MainWindow::InputToggleButton::InputToggleButton()
: UI::Button("Mouse", LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment::TOP_BOTTOM_CENTRE) {
	set_canvas_input_device(true);
}



bool MainWindow::LayerMoveUpButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button && layer->next) {
		bool isActive = layer == mainWindow->canvasView->getCanvas()->getActiveLayer();

		LayerPtr above = layer->next;
		mainWindow->canvasView->getCanvas()->removeLayer(layer);
		mainWindow->canvasView->getCanvas()->addLayerAfter(above, layer);

		if(isActive) {
			mainWindow->canvasView->getCanvas()->setActiveLayer(layer);
		}

		mainWindow->canvasView->getCanvas()->forceRedraw();
		mainWindow->setNeedsRecreating();
	}
	return true; 
}

MainWindow::LayerMoveUpButton::LayerMoveUpButton(MainWindow * mw, LayerPtr layer_) 
: UI::Button("^", 1, 0, 20, 0), mainWindow(mw), layer(layer_) {}


bool MainWindow::LayerMoveDownButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button && layer->prev) {
		bool isActive = layer == mainWindow->canvasView->getCanvas()->getActiveLayer();

		LayerPtr below = layer->prev;
		mainWindow->canvasView->getCanvas()->removeLayer(layer);
		mainWindow->canvasView->getCanvas()->addLayerBefore(below, layer);

		if(isActive) {
			mainWindow->canvasView->getCanvas()->setActiveLayer(layer);
		}

		mainWindow->canvasView->getCanvas()->forceRedraw();
		mainWindow->setNeedsRecreating();
	}
	return true; 
}

MainWindow::LayerMoveDownButton::LayerMoveDownButton(MainWindow * mw, LayerPtr layer_)
: UI::Button("\\/", 1, 0, 20, 0), mainWindow(mw), layer(layer_) {}


bool MainWindow::LayerButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Label::onMouseButtonReleased(button);
	if(!button) {
		mainWindow->canvasView->getCanvas()->setActiveLayer(layer);
	}
	return true; 
}

uint32_t MainWindow::LayerButton::getBackGroundColour()
{
	if(layer == mainWindow->canvasView->getCanvas()->getActiveLayer()) {
		return 0x30300000;
	}
	else {
		return 0;
	}
}

MainWindow::LayerButton::LayerButton(MainWindow * mw, LayerPtr layer_)
: UI::Label(layer_->name, 0, 0, 100, 0), mainWindow(mw), layer(layer_) {}

bool MainWindow::LayerVisibilityButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Label::onMouseButtonReleased(button);
	if(!button) {
		layer->visible = !layer->visible;
		mainWindow->canvasView->getCanvas()->forceRedraw();
	}
	return true; 
}

string const& MainWindow::LayerVisibilityButton::getText()
{
	static string plusString = "+";
	static string hyphenString = "-";
	if(layer->visible) {
		return plusString;
	}
	else {
		return hyphenString;
	}
}

MainWindow::LayerVisibilityButton::LayerVisibilityButton(MainWindow * mw, LayerPtr layer_)
: UI::Label("+", 1, 0, 20, 0), mainWindow(mw), layer(layer_) {}


bool MainWindow::ColourSetter::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		set_active_colour(colour[0], colour[1], colour[2], 0.3f);
	}
	return true; 
}

MainWindow::ColourSetter::ColourSetter(string t_, float r, float g, float b)
: UI::Button(t_) 
{
	colour[0] = r;
	colour[1] = g;
	colour[2] = b;
}

MainWindow::QuitButton::QuitButton()
: UI::Button("Quit") {}

bool MainWindow::QuitButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		quit_application();
	}
	return false; 
}

MainWindow::NewLayerButton::NewLayerButton(MainWindow * mw)
: UI::Button("New Layer"), mainWindow(mw) {}

bool MainWindow::NewLayerButton::onMouseButtonReleased(unsigned int button)
{ 
	bool redraw = UI::Button::onMouseButtonReleased(button);
	if(!button) {
		LayerPtr newLayer = make_shared<Layer>();
		newLayer->name = "New Layer";
		mainWindow->canvasView->getCanvas()->addLayer(newLayer);
	}
	return redraw; 
}

void MainWindow::create(::Canvas * canvas)
{
	vector<UI::Widget *> layersWidgets;

	LayerPtr layer = canvas->getFirstLayer();
	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			UI::Widget * layerNameButton = new LayerButton(this, layer);
			UI::Widget * layerVisibilityButton = new LayerVisibilityButton(this, layer);
			UI::Widget * layerMoveUp = new LayerMoveUpButton(this, layer);
			UI::Widget * layerMoveDown = new LayerMoveDownButton(this, layer);


			UI::Container * c = new UI::Container(vector<UI::Widget *>
				{layerNameButton, layerVisibilityButton, layerMoveUp, layerMoveDown},
				0, layersWidgets.size(), 160+3*UI::get_widget_padding(), 0, 0, UI::Container::LayoutManager::FLOW_ACROSS);


			widgets.push_back(layerNameButton);
			widgets.push_back(layerVisibilityButton);
			widgets.push_back(layerMoveUp);
			widgets.push_back(layerMoveDown);
			widgets.push_back(c);

			layersWidgets.insert(layersWidgets.begin(), c); 
		}

		if(layer->next) {
			layer = layer->next;
		}
		else {
			break;
		}
	}

	UI::Container * sep1 = new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE);
	layersWidgets.insert(layersWidgets.begin(), sep1);
	widgets.push_back(sep1);

	UI::Container * sep2 = new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE);
	layersWidgets.push_back(sep2);
	widgets.push_back(sep2);

	NewLayerButton * newLayerButton = new NewLayerButton(this);
	widgets.push_back(newLayerButton);
	layersWidgets.push_back(newLayerButton);


	UI::Container * layersContainer = new UI::Container(layersWidgets, 0, 1, 0, 0, 0xff202020, UI::Container::LayoutManager::FLOW_DOWN);
	widgets.push_back(layersContainer);

	// Menu bar (top)

	QuitButton * bQuit = new QuitButton();
	UI::Menu * fileMenu = new UI::Menu(vector<UI::Widget *> { bQuit });

	UI::MenuItem * fileMenuButton = new UI::MenuItem("File", fileMenu);
	UI::MenuBar * topMenuBar = new UI::MenuBar(vector<UI::Widget *> { fileMenuButton }, 1, 0, 0, 0, 0xff404040, UI::Container::LayoutManager::FLOW_ACROSS);


	canvasView = new CanvasViewPort(canvas, 1, 1, 0, 0);


	root = new UI::Container(vector<UI::Widget *> {topMenuBar, layersContainer, canvasView}, 0, 0, 0, 0, 0, UI::Container::LayoutManager::BORDER);	
	widgets.push_back(root);
}

MainWindow::MainWindow(Canvas * canvas)
{
	create(canvas);
}

void MainWindow::centerCanvas()
{

	unsigned int x, y, canvasWidth, canvasHeight;
	canvasView->getArea(x, y, canvasWidth, canvasHeight);
	canvasView->setCanvasPosition(canvasWidth/2, canvasHeight/2);
}

UI::Container * MainWindow::getRoot()
{
	return root;
}

MainWindow::MainWindow(MainWindow const& m)
{
	create(m.canvasView->getCanvas());

	canvasView->setCanvasPosition(m.canvasView->getCanvasPositionX(), m.canvasView->getCanvasPositionY());
	canvasView->setCanvasZoom(m.canvasView->getCanvasZoom());
}
