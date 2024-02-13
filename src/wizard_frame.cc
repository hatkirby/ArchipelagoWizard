#include "wizard_frame.h"

#include <wx/listctrl.h>
#include <wx/splitter.h>

#include <sstream>

#include "world_window.h"

enum WizardFrameIds {
  ID_NEW_WORLD = 1,
  ID_LOAD_WORLD = 2,
  ID_SAVE_WORLD = 3,
};

class WorldEntryData : public wxTreeItemData {
 public:
  explicit WorldEntryData(int v) : index(v) {}

  int index = -1;
};

WizardFrame::WizardFrame()
    : wxFrame(nullptr, wxID_ANY, "Archipelago Generation Wizard") {
  SetSize(728, 728);

  game_definitions_ = std::make_unique<GameDefinitions>();

  wxMenu* menuFile = new wxMenu();
  menuFile->Append(ID_NEW_WORLD, "&New World\tCtrl-N");
  menuFile->Append(ID_LOAD_WORLD, "&Load World from File\tCtrl-O");
  menuFile->Append(ID_SAVE_WORLD, "&Save World\tCtrl-S");
  menuFile->Append(wxID_EXIT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");

  SetMenuBar(menuBar);

  Bind(wxEVT_MENU, &WizardFrame::OnNewWorld, this, ID_NEW_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnLoadWorld, this, ID_LOAD_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnSaveWorld, this, ID_SAVE_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnExit, this, wxID_EXIT);

  splitter_window_ = new wxSplitterWindow(this, wxID_ANY);
  splitter_window_->SetMinimumPaneSize(250);

  world_list_ = new wxTreeCtrl(splitter_window_, wxID_ANY, wxDefaultPosition,
                               wxDefaultSize, wxTR_HIDE_ROOT);
  wxTreeItemId root_id = world_list_->AddRoot("Multiworld");

  world_list_->Bind(wxEVT_TREE_SEL_CHANGED, &WizardFrame::OnWorldSelected,
                    this);

  world_window_ = new WorldWindow(splitter_window_, game_definitions_.get());

  splitter_window_->SplitVertically(world_list_, world_window_, 250);

  world_window_->Hide();
}

void WizardFrame::OnNewWorld(wxCommandEvent& event) {
  InitializeWorld(std::make_unique<World>());
}

void WizardFrame::OnLoadWorld(wxCommandEvent& event) {
  wxFileDialog openFileDialog(this, "Open World YAML", "", "",
                              "YAML files (*.yaml;*.yml)|*.yaml;*.yml",
                              wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (openFileDialog.ShowModal() == wxID_CANCEL) {
    return;
  }

  try {
    std::unique_ptr<World> load_world = std::make_unique<World>();
    load_world->Load(openFileDialog.GetPath().ToStdString(),
                     game_definitions_.get());

    InitializeWorld(std::move(load_world));
  } catch (const std::exception& ex) {
    wxMessageBox(ex.what(), "Error loading World", wxOK, this);
  }
}

void WizardFrame::OnSaveWorld(wxCommandEvent& event) {}

void WizardFrame::OnExit(wxCommandEvent& event) { Close(true); }

void WizardFrame::OnWorldSelected(wxTreeEvent& event) {
  const WorldEntryData* data =
      dynamic_cast<WorldEntryData*>(world_list_->GetItemData(event.GetItem()));
  world_window_->LoadWorld(worlds_.at(data->index).get());
  world_window_->Show();
}

void WizardFrame::InitializeWorld(std::unique_ptr<World> world) {
  int index = worlds_.size();
  worlds_.push_back(std::move(world));

  World* new_world = worlds_.back().get();

  wxTreeItemId root_id = world_list_->GetRootItem();
  wxTreeItemId new_id = world_list_->AppendItem(root_id, "New World", -1, -1,
                                                new WorldEntryData(index));

  new_world->SetMetaUpdateCallback(
      [this, new_world, new_id] { UpdateWorldDisplay(new_world, new_id); });

  UpdateWorldDisplay(new_world, new_id);
  world_list_->SelectItem(new_id);
}

void WizardFrame::SyncWorldIndices() {
  wxTreeItemId root_id = world_list_->GetRootItem();
  wxTreeItemIdValue cookie;
  int index = 0;
  for (wxTreeItemId tree_item_id = world_list_->GetFirstChild(root_id, cookie);
       tree_item_id.IsOk();
       tree_item_id = world_list_->GetNextChild(root_id, cookie)) {
    dynamic_cast<WorldEntryData*>(world_list_->GetItemData(tree_item_id))
        ->index = index;
    index++;
  }
}

void WizardFrame::UpdateWorldDisplay(World* world, wxTreeItemId tree_item_id) {
  std::ostringstream world_display;
  if (world->GetName() == "") {
    world_display << "Untitled World";
  } else {
    world_display << world->GetName();
  }

  if (world->HasGame()) {
    world_display << "[" << world->GetGame() << "]";
  }

  world_list_->SetItemText(tree_item_id, world_display.str());
}
