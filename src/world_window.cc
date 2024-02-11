#include "world_window.h"

#include "wizard_editor.h"

WorldWindow::WorldWindow(wxWindow* parent,
                         const GameDefinitions* game_definitions)
    : wxNotebook(parent, wxID_ANY), game_definitions_(game_definitions) {
  wizard_editor_ = new WizardEditor(this, game_definitions_);
  AddPage(wizard_editor_, "Wizard", true);
}
