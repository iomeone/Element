
#include "controllers/DevicesController.h"
#include "controllers/EngineController.h"
#include "controllers/GraphController.h"
#include "controllers/GuiController.h"
#include "controllers/MappingController.h"
#include "controllers/PresetsController.h"
#include "session/Session.h"
#include "DataPath.h"
#include "Globals.h"

namespace Element {

void GraphController::activate()
{
    document.setLastDocumentOpened (
        DataPath::defaultGraphDir().getChildFile ("Untitled.elg"));
}

void GraphController::deactivate()
{

}

void GraphController::openDefaultGraph()
{
    if (auto* gc = findSibling<GuiController>())
        gc->closeAllPluginWindows();
        
    getWorld().getSession()->clear();
    auto newGraph = Node::createDefaultGraph();
    document.setGraph (newGraph);
    getWorld().getSession()->addGraph (document.getGraph(), true);
    refreshOtherControllers();
    findSibling<GuiController>()->stabilizeContent();
}

void GraphController::openGraph (const File& file)
{
    document.saveIfNeededAndUserAgrees();
    auto result = document.loadFrom (file, true);
    
    if (result.wasOk())
    {
        findSibling<GuiController>()->closeAllPluginWindows();
        getWorld().getSession()->clear();
        getWorld().getSession()->addGraph (document.getGraph(), true);
        refreshOtherControllers();
        document.setChangedFlag (false);
    }
}

void GraphController::newGraph()
{
    // - 0 if the third button was pressed ('cancel')
    // - 1 if the first button was pressed ('yes')
    // - 2 if the middle button was pressed ('no')
    int res = 2;
    if (document.hasChangedSinceSaved())
        res = AlertWindow::showYesNoCancelBox (AlertWindow::InfoIcon, "Save Graph?",
                                               "The current graph has changes. Would you like to save it?",
                                               "Save Graph", "Don't Save", "Cancel");
    if (res == 1)
        document.save (true, true);
    
    if (res == 1 || res == 2)
    {
        findSibling<GuiController>()->closeAllPluginWindows();
        getWorld().getSession()->clear();
        auto newGraph = Node::createDefaultGraph();
        document.setGraph (newGraph);
        getWorld().getSession()->addGraph (document.getGraph(), true);
        refreshOtherControllers();
        findSibling<GuiController>()->stabilizeContent();
    }
}

void GraphController::saveGraph (const bool saveAs)
{
    auto result = FileBasedDocument::userCancelledSave;

    if (saveAs) {
        result = document.saveAs (File(), true, true, true);
    } else {
        result = document.save (true, true);
    }

    if (result == FileBasedDocument::userCancelledSave)
        return;
    
    if (result == FileBasedDocument::savedOk)
    {
        // ensure change messages are flushed so the changed flag doesn't reset
        document.setChangedFlag (false);
        jassert (! hasGraphChanged());
    }
}

void GraphController::refreshOtherControllers()
{
    findSibling<EngineController>()->sessionReloaded();
    findSibling<DevicesController>()->refresh();
    findSibling<MappingController>()->learn (false);
    findSibling<PresetsController>()->refresh();
}

}
