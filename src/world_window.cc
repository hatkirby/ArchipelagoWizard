#include "world_window.h"

#include "wizard_editor.h"
#include "yaml_editor.h"

WorldWindow::WorldWindow(wxWindow* parent,
                         const GameDefinitions* game_definitions)
    : wxNotebook(parent, wxID_ANY), game_definitions_(game_definitions) {
  Bind(wxEVT_NOTEBOOK_PAGE_CHANGING, &WorldWindow::OnPageChanging, this);
  Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &WorldWindow::OnPageChanged, this);

  wizard_editor_ = new WizardEditor(this, game_definitions_);
  yaml_editor_ = new YamlEditor(this);
}

void WorldWindow::LoadWorld(World* world) {
  if (!world_) {
    AddPage(wizard_editor_, "Wizard", true);
    AddPage(yaml_editor_, "YAML", false);
  }

  world_ = world;

  wizard_editor_->LoadWorld(world_);

  if (GetSelection() == 1) {
    yaml_editor_->LoadWorld(world_);
  }
}

void WorldWindow::SaveWorld() {
  if (GetSelection() == 1) {
    yaml_editor_->SaveWorld();
  }
}

void WorldWindow::UnloadWorld() {
  if (world_) {
    world_ = nullptr;

    RemovePage(1);
    RemovePage(0);
  }
}

void WorldWindow::OnPageChanging(wxBookCtrlEvent& event) {
  if (!world_) {
    return;
  }

  if (event.GetOldSelection() == 1) {
    try {
      yaml_editor_->SaveWorld();
    } catch (const std::exception& ex) {
      wxString msg;
      msg << "Could not process YAML.\n\n";
      msg << ex.what();
      msg << "\n\nWould you like to discard your changes?";

      if (wxMessageBox(msg, "Invalid YAML", wxYES_NO) == wxNO) {
        event.Veto();
      }
    }
  }
}

void WorldWindow::OnPageChanged(wxBookCtrlEvent& event) {
  if (!world_) {
    return;
  }

  if (GetSelection() == 0) {
    wizard_editor_->Reload();
  } else if (GetSelection() == 1) {
    yaml_editor_->LoadWorld(world_);
  }
}

void WorldWindow::SetMessageCallback(
    std::function<void(const wxString&, const wxString&)> callback) {
  wizard_editor_->SetMessageCallback(std::move(callback));
}
