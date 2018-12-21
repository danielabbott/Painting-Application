#include <MainWindow.h>
#include <iostream>
#include <Window.h>

using namespace std;



bool MainWindow::MyButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		// clear_layer(get_first_layer()->next);
		mainWindow->canvas->fillLayer(mainWindow->canvas->getFirstLayer(), 0xffffffff);
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
		bool isActive = layer == mainWindow->canvas->getActiveLayer();

		Layer * above = layer->next;
		mainWindow->canvas->removeLayer(*layer);
		mainWindow->canvas->addLayerAfter(*above, *layer);
		
		if(isActive) {
			mainWindow->canvas->setActiveLayer(layer);
		}

		mainWindow->canvas->forceRedraw();
		mainWindow->setNeedsRecreating();
	}
	return true; 
}

MainWindow::LayerMoveUpButton::LayerMoveUpButton(MainWindow * mw, Layer * layer_) 
: UI::Button("^", 1, 0, 20, 0), mainWindow(mw), layer(layer_) {}


bool MainWindow::LayerMoveDownButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button && layer->prev) {
		bool isActive = layer == mainWindow->canvas->getActiveLayer();

		Layer * below = layer->prev;
		mainWindow->canvas->removeLayer(*layer);
		mainWindow->canvas->addLayerBefore(*below, *layer);

		if(isActive) {
			mainWindow->canvas->setActiveLayer(layer);
		}

		mainWindow->canvas->forceRedraw();
		mainWindow->setNeedsRecreating();
	}
	return true; 
}

MainWindow::LayerMoveDownButton::LayerMoveDownButton(MainWindow * mw, Layer * layer_)
: UI::Button("\\/", 1, 0, 20, 0), mainWindow(mw), layer(layer_) {}


bool MainWindow::LayerButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Label::onMouseButtonReleased(button);
	if(!button) {
		mainWindow->canvas->setActiveLayer(layer);
	}
	return true; 
}

uint32_t MainWindow::LayerButton::getBackGroundColour()
{
	if(layer == mainWindow->canvas->getActiveLayer()) {
		return 0x30300000;
	}
	else {
		return 0;
	}
}

MainWindow::LayerButton::LayerButton(MainWindow * mw, Layer * layer_)
: UI::Label(layer_->name, 0, 0, 100, 0), mainWindow(mw), layer(layer_) {}

bool MainWindow::LayerVisibilityButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Label::onMouseButtonReleased(button);
	if(!button) {
		layer->visible = !layer->visible;
		mainWindow->canvas->forceRedraw();
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

MainWindow::LayerVisibilityButton::LayerVisibilityButton(MainWindow * mw, Layer * layer_)
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

MainWindow::QuitButton::QuitButton(string text_)
: UI::Button(text_) {}

bool MainWindow::QuitButton::onMouseButtonReleased(unsigned int button)
{ 
	UI::Button::onMouseButtonReleased(button);
	if(!button) {
		quit_application();
	}
	return true; 
}

MainWindow::MainWindow(Canvas * canvas_) : canvas(canvas_)
{
	vector<UI::Widget *> layerLabels;

	Layer * layer = canvas->getFirstLayer();
	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			UI::Widget * layerNameButton = new LayerButton(this, layer);
			UI::Widget * layerVisibilityButton = new LayerVisibilityButton(this, layer);
			UI::Widget * layerMoveUp = new LayerMoveUpButton(this, layer);
			UI::Widget * layerMoveDown = new LayerMoveDownButton(this, layer);


			UI::Container * c = new UI::Container(vector<UI::Widget *>
				{layerNameButton, layerVisibilityButton, layerMoveUp, layerMoveDown},
				0, layerLabels.size(), 160+3*UI::get_widget_padding(), 0, 0, UI::Container::LayoutManager::FLOW_ACROSS);


			widgets.push_back(layerNameButton);
			widgets.push_back(layerVisibilityButton);
			widgets.push_back(layerMoveUp);
			widgets.push_back(layerMoveDown);
			widgets.push_back(c);

			layerLabels.insert(layerLabels.begin(), c); 
		}

		if(layer->next) {
			layer = layer->next;
		}
		else {
			break;
		}
	}

	UI::Container * sep1 = new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE);
	layerLabels.insert(layerLabels.begin(), sep1);
	widgets.push_back(sep1);

	UI::Container * sep2 = new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE);
	layerLabels.push_back(sep2);
	widgets.push_back(sep2);

	UI::Container * layersContainer = new UI::Container(layerLabels, 0, 1, 0, 0, 0xff202020, UI::Container::LayoutManager::FLOW_DOWN);
	widgets.push_back(layersContainer);

	// Menu bar (top)

	QuitButton * bQuit = new QuitButton("Quit");
	UI::Menu * fileMenu = new UI::Menu(vector<UI::Widget *> { bQuit });

	UI::MenuItem * fileMenuButton = new UI::MenuItem("File", fileMenu);
	UI::MenuBar * topMenuBar = new UI::MenuBar(vector<UI::Widget *> { fileMenuButton }, 1, 0, 0, 0, 0xff404040, UI::Container::LayoutManager::FLOW_ACROSS);



	root = new UI::Container(vector<UI::Widget *> {topMenuBar, layersContainer, canvas}, 0, 0, 0, 0, 0, UI::Container::LayoutManager::BORDER);	
	widgets.push_back(root);
}

UI::Container * MainWindow::getRoot()
{
	return root;
}
