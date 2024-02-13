#include "wizard_frame.h"

#include <wx/listctrl.h>
#include <wx/splitter.h>

#include "world_window.h"

WizardFrame::WizardFrame()
    : wxFrame(nullptr, wxID_ANY, "Archipelago Generation Wizard") {
  SetSize(728, 728);

  game_definitions_ = std::make_unique<GameDefinitions>();

  wxMenu* menuFile = new wxMenu();
  menuFile->Append(wxID_EXIT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");

  SetMenuBar(menuBar);

  Bind(wxEVT_MENU, &WizardFrame::OnExit, this, wxID_EXIT);

  splitter_window_ = new wxSplitterWindow(this, wxID_ANY);

  worlds_.push_back(std::make_unique<World>());
  worlds_.push_back(std::make_unique<World>());

  world_list_ = new wxListView(splitter_window_, wxID_ANY);
  world_list_->AppendColumn("World Name");
  world_list_->InsertItem(0, "Hello");
  world_list_->InsertItem(1, "Bye");

  world_list_->Bind(wxEVT_LIST_ITEM_SELECTED, &WizardFrame::OnWorldSelected,
                    this);

  world_window_ = new WorldWindow(splitter_window_, game_definitions_.get());

  splitter_window_->SplitVertically(world_list_, world_window_, 250);

  world_window_->Hide();
}

void WizardFrame::OnExit(wxCommandEvent& event) { Close(true); }

void WizardFrame::OnWorldSelected(wxListEvent& event) {
  world_window_->LoadWorld(worlds_.at(event.GetItem().GetId()).get());
  world_window_->Show();
}
