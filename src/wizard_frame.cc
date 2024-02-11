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

  world_list_ = new wxListView(splitter_window_, wxID_ANY);
  world_list_->AppendColumn("World Name");
  world_list_->InsertItem(0, "Hello");

  world_window_ = new WorldWindow(splitter_window_, game_definitions_.get());

  splitter_window_->SplitVertically(world_list_, world_window_, 250);
}

void WizardFrame::OnExit(wxCommandEvent& event) { Close(true); }
