/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic outline for a simple desktop window.

  ==============================================================================
*/

#include "MainWindow.h"
#include "MainComponent.h"


//==============================================================================
MainAppWindow::MainAppWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colours::lightgrey,
                      DocumentWindow::allButtons)
{
	MainComponent* const contentComponent = new MainComponent();
	setContentComponent (contentComponent, true, true);
    centreWithSize (1024, 768);
    setVisible (true);
	
}

MainAppWindow::~MainAppWindow()
{
	//delete(contentComponent);
}

void MainAppWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}