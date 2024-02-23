#include "wizard_frame.h"

#include <wx/aboutdlg.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>

#include <filesystem>
#include <sstream>

#include "version.h"
#include "world_window.h"

enum WizardFrameIds {
  ID_NEW_WORLD = 1,
  ID_LOAD_WORLD = 2,
  ID_SAVE_WORLD = 3,
  ID_CLOSE_WORLD = 4,
  ID_SAVE_AS_WORLD = 5,
};

class WorldEntryData : public wxTreeItemData {
 public:
  explicit WorldEntryData(World* w) : world(w) {}

  World* world;
};

WizardFrame::WizardFrame()
    : wxFrame(nullptr, wxID_ANY, "Archipelago Generation Wizard") {
  SetSize(728, 728);

  game_definitions_ = std::make_unique<GameDefinitions>();

  wxMenu* menuFile = new wxMenu();
  menuFile->Append(ID_NEW_WORLD, "&New World\tCtrl-N");
  menuFile->Append(ID_LOAD_WORLD, "&Load World from File\tCtrl-O");
  menuFile->Append(ID_SAVE_WORLD, "&Save World\tCtrl-S");
  menuFile->Append(ID_SAVE_AS_WORLD, "Save World As...\tCtrl-Shift-S");
  menuFile->Append(ID_CLOSE_WORLD, "&Close World\tCtrl-W");
  menuFile->Append(wxID_EXIT);

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

  Bind(wxEVT_MENU, &WizardFrame::OnNewWorld, this, ID_NEW_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnLoadWorld, this, ID_LOAD_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnSaveWorld, this, ID_SAVE_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnSaveAsWorld, this, ID_SAVE_AS_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnCloseWorld, this, ID_CLOSE_WORLD);
  Bind(wxEVT_MENU, &WizardFrame::OnExit, this, wxID_EXIT);
  Bind(wxEVT_MENU, &WizardFrame::OnAbout, this, wxID_ABOUT);

  Bind(wxEVT_CLOSE_WINDOW, &WizardFrame::OnClose, this);

  splitter_window_ = new wxSplitterWindow(this, wxID_ANY);
  splitter_window_->SetMinimumPaneSize(250);

  wxPanel* left_pane = new wxPanel(splitter_window_, wxID_ANY);

  world_list_ = new wxTreeCtrl(left_pane, wxID_ANY, wxDefaultPosition,
                               wxDefaultSize, wxTR_HIDE_ROOT);
  wxTreeItemId root_id = world_list_->AddRoot("Multiworld");

  world_list_->Bind(wxEVT_TREE_SEL_CHANGING, &WizardFrame::OnWorldSelecting,
                    this);
  world_list_->Bind(wxEVT_TREE_SEL_CHANGED, &WizardFrame::OnWorldSelected,
                    this);

  message_pane_ = new wxScrolledWindow(left_pane, wxID_ANY);

  message_header_ = new wxStaticText(message_pane_, wxID_ANY, "");
  message_header_->SetFont(message_header_->GetFont().Bold());

  message_window_ = new wxStaticText(message_pane_, wxID_ANY, "");

  wxBoxSizer* msg_sizer = new wxBoxSizer(wxVERTICAL);
  msg_sizer->Add(message_header_, wxSizerFlags().Expand());
  msg_sizer->AddSpacer(10);
  msg_sizer->Add(message_window_, wxSizerFlags().Expand());
  message_pane_->SetSizer(msg_sizer);
  message_pane_->Layout();

  wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
  left_sizer->Add(world_list_, wxSizerFlags().Proportion(4).Expand());
  left_sizer->Add(message_pane_,
                  wxSizerFlags().DoubleBorder().Proportion(1).Expand());
  left_pane->SetSizer(left_sizer);
  left_pane->Layout();

  message_pane_->SetScrollRate(0, 5);

  world_window_ = new WorldWindow(splitter_window_, game_definitions_.get());

  splitter_window_->SplitVertically(left_pane, world_window_, 250);

  world_window_->SetMessageCallback(
      [this](const wxString& header, const wxString& msg) {
        ShowMessage(header, msg);
      });

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(splitter_window_, wxSizerFlags().Proportion(1).Expand());
  SetSizer(sizer);
}

void WizardFrame::OnNewWorld(wxCommandEvent& event) {
  if (!FlushSelectedWorld(/*ask_discard=*/true)) {
    return;
  }

  InitializeWorld(std::make_unique<World>(game_definitions_.get()));
}

void WizardFrame::OnLoadWorld(wxCommandEvent& event) {
  if (!FlushSelectedWorld(/*ask_discard=*/true)) {
    return;
  }

  wxFileDialog openFileDialog(this, "Open World YAML", "", "",
                              "YAML files (*.yaml;*.yml)|*.yaml;*.yml",
                              wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (openFileDialog.ShowModal() == wxID_CANCEL) {
    return;
  }

  try {
    std::unique_ptr<World> load_world =
        std::make_unique<World>(game_definitions_.get());
    load_world->Load(openFileDialog.GetPath().ToStdString());

    InitializeWorld(std::move(load_world));
  } catch (const std::exception& ex) {
    wxMessageBox(ex.what(), "Error loading World", wxOK, this);
  }
}

void WizardFrame::OnSaveWorld(wxCommandEvent& event) {
  if (!world_list_->GetSelection().IsOk() ||
      world_list_->GetRootItem() == world_list_->GetSelection()) {
    return;
  }

  if (!FlushSelectedWorld(/*ask_discard=*/false)) {
    return;
  }

  const WorldEntryData* data = dynamic_cast<WorldEntryData*>(
      world_list_->GetItemData(world_list_->GetSelection()));
  AttemptSaveWorld(*data->world);
}

void WizardFrame::OnSaveAsWorld(wxCommandEvent& event) {
  if (!world_list_->GetSelection().IsOk() ||
      world_list_->GetRootItem() == world_list_->GetSelection()) {
    return;
  }

  if (!FlushSelectedWorld(/*ask_discard=*/false)) {
    return;
  }

  const WorldEntryData* data = dynamic_cast<WorldEntryData*>(
      world_list_->GetItemData(world_list_->GetSelection()));
  AttemptSaveWorld(*data->world, /*force_dialog=*/true);
}

void WizardFrame::OnCloseWorld(wxCommandEvent& event) {
  if (!world_list_->GetSelection().IsOk() ||
      world_list_->GetRootItem() == world_list_->GetSelection()) {
    return;
  }

  if (!FlushSelectedWorld(/*ask_discard=*/true)) {
    return;
  }

  const WorldEntryData* data = dynamic_cast<WorldEntryData*>(
      world_list_->GetItemData(world_list_->GetSelection()));
  World& world = *data->world;
  if (world.IsDirty()) {
    wxString message_text;
    message_text << "\"";

    if (world.GetName() == "") {
      message_text << "Untitled World";
    } else {
      message_text << world.GetName();
    }

    if (world.HasGame()) {
      message_text << " [" << world.GetGame() << "]";
    }

    message_text << "\" has unsaved changes. Would you like to save them?";

    int result =
        wxMessageBox(message_text, "Confirm", wxYES_NO | wxCANCEL, this);
    if (result == wxCANCEL) {
      return;
    } else if (result == wxYES) {
      if (!AttemptSaveWorld(world)) {
        return;
      }
    }
  }

  // Deleting the entry from the tree automatically changes the selection, which
  // runs the handlers. Thus we should not actually delete the old world until
  // afterwards.
  world_list_->Delete(world_list_->GetSelection());

  auto it = std::find_if(
      worlds_.begin(), worlds_.end(),
      [&world](const std::unique_ptr<World>& w) { return w.get() == &world; });
  if (it != worlds_.end()) {
    worlds_.erase(it);
  }
}

void WizardFrame::OnExit(wxCommandEvent& event) {
  if (!FlushAllWorlds()) {
    return;
  }

  Close(true);
}

void WizardFrame::OnClose(wxCloseEvent& event) {
  if (event.CanVeto() && !FlushAllWorlds()) {
    event.Veto();
    return;
  }

  Destroy();
}

void WizardFrame::OnAbout(wxCommandEvent& event) {
  std::ostringstream version_string;
  version_string << kWizardVersion;

  wxAboutDialogInfo about_info;
  about_info.SetName("ArchipelagoWizard");
  about_info.SetVersion(version_string.str());
  about_info.SetDescription("GUI tool for generating Archipelago worlds.");
  about_info.AddDeveloper("hatkirby");

  wxAboutBox(about_info);
}

void WizardFrame::OnWorldSelecting(wxTreeEvent& event) {
  if (!FlushSelectedWorld(/*ask_discard=*/true)) {
    event.Veto();
  }
}

void WizardFrame::OnWorldSelected(wxTreeEvent& event) {
  if (!event.GetItem().IsOk() ||
      event.GetItem() == world_list_->GetRootItem()) {
    world_window_->UnloadWorld();
  } else {
    const WorldEntryData* data = dynamic_cast<WorldEntryData*>(
        world_list_->GetItemData(event.GetItem()));
    world_window_->LoadWorld(data->world);
  }
}

void WizardFrame::InitializeWorld(std::unique_ptr<World> world) {
  int index = worlds_.size();
  worlds_.push_back(std::move(world));

  World* new_world = worlds_.back().get();

  wxTreeItemId root_id = world_list_->GetRootItem();
  wxTreeItemId new_id = world_list_->AppendItem(root_id, "New World", -1, -1,
                                                new WorldEntryData(new_world));

  new_world->SetMetaUpdateCallback(
      [this, new_world, new_id] { UpdateWorldDisplay(new_world, new_id); });

  UpdateWorldDisplay(new_world, new_id);
  world_list_->SelectItem(new_id);
}

void WizardFrame::UpdateWorldDisplay(World* world, wxTreeItemId tree_item_id) {
  wxString world_display;
  if (world->IsDirty()) {
    world_display << "*";
  }

  if (world->GetName() == "") {
    world_display << "Untitled World";
  } else {
    world_display << world->GetName();
  }

  if (world->HasGame()) {
    world_display << " [";
    world_display << world->GetGame();
    world_display << "]";
  }

  if (world->HasFilename()) {
    std::filesystem::path filepath(world->GetFilename());
    world_display << " (";
    world_display << filepath.filename().c_str();
    world_display << ")";
  }

  world_list_->SetItemText(tree_item_id, world_display);
}

void WizardFrame::ShowMessage(const wxString& header, const wxString& msg) {
  for (int i = 0; i < 2; i++) {
    int width = message_pane_->GetClientSize().GetWidth();
    message_header_->SetLabel(header);
    message_header_->Wrap(width);

    message_window_->SetLabel(msg);
    message_window_->Wrap(width);

    message_pane_->SetSizer(message_pane_->GetSizer());
    message_pane_->Layout();
    message_pane_->FitInside();
    message_pane_->Scroll(0, 0);
  }
}

bool WizardFrame::FlushSelectedWorld(bool ask_discard) {
  if (!world_list_->GetSelection().IsOk() ||
      world_list_->GetRootItem() == world_list_->GetSelection()) {
    return true;
  }

  try {
    world_window_->SaveWorld();
  } catch (const std::exception& ex) {
    wxString msg;
    msg << "Could not save world.\n\n";
    msg << ex.what();

    if (ask_discard) {
      msg << "\n\nWould you like to discard your changes?";

      if (wxMessageBox(msg, "Failure to save world", wxYES_NO) == wxNO) {
        return false;
      }
    } else {
      wxMessageBox(msg, "Failure to save world");

      return false;
    }
  }

  return true;
}

bool WizardFrame::FlushAllWorlds() {
  if (!FlushSelectedWorld(/*ask_discard=*/true)) {
    return false;
  }

  for (std::unique_ptr<World>& world : worlds_) {
    if (world->IsDirty()) {
      wxString message_text;
      message_text << "\"";

      if (world->GetName() == "") {
        message_text << "Untitled World";
      } else {
        message_text << world->GetName();
      }

      if (world->HasGame()) {
        message_text << " [" << world->GetGame() << "]";
      }

      message_text << "\" has unsaved changes. Would you like to save them "
                      "before quitting?";

      int result =
          wxMessageBox(message_text, "Confirm", wxYES_NO | wxCANCEL, this);
      if (result == wxCANCEL) {
        return false;
      } else if (result == wxYES) {
        if (!AttemptSaveWorld(*world)) {
          return false;
        }
      }
    }
  }

  return true;
}

bool WizardFrame::AttemptSaveWorld(World& world, bool force_dialog) {
  if (!world.HasFilename() || force_dialog) {
    wxFileDialog saveFileDialog(this, "Save World YAML", "", "",
                                "YAML files (*.yaml;*.yml)|*.yaml;*.yml",
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
      return false;
    }

    world.SetFilename(saveFileDialog.GetPath().ToStdString());
  }

  try {
    world.Save(world.GetFilename());
  } catch (const std::exception& ex) {
    wxMessageBox(ex.what(), "Error saving World", wxOK, this);

    return false;
  }

  return true;
}
