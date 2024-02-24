#ifndef WIZARD_FRAME_H_E923FBAE
#define WIZARD_FRAME_H_E923FBAE

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/treectrl.h>

#include "game_definition.h"
#include "world.h"

class wxListView;
class WorldWindow;
class wxSplitterWindow;

class WizardFrame : public wxFrame {
 public:
  WizardFrame();

 private:
  void OnNewWorld(wxCommandEvent& event);
  void OnLoadWorld(wxCommandEvent& event);
  void OnSaveWorld(wxCommandEvent& event);
  void OnSaveAsWorld(wxCommandEvent& event);
  void OnCloseWorld(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnWorldSelecting(wxTreeEvent& event);
  void OnWorldSelected(wxTreeEvent& event);
  void OnWorldRightClick(wxTreeEvent& event);

  void InitializeWorld(std::unique_ptr<World> world);
  void SyncWorldIndices();
  void UpdateWorldDisplay(World* world, wxTreeItemId tree_item_id);
  void ShowMessage(const wxString& header, const wxString& msg);
  bool FlushSelectedWorld(bool ask_discard);
  bool FlushAllWorlds();
  bool AttemptSaveWorld(World& world, bool force_dialog = false);

  wxSplitterWindow* splitter_window_;
  wxTreeCtrl* world_list_;
  WorldWindow* world_window_;

  wxScrolledWindow* message_pane_;
  wxStaticText* message_header_;
  wxStaticText* message_window_;

  std::unique_ptr<GameDefinitions> game_definitions_;

  std::vector<std::unique_ptr<World>> worlds_;
};

#endif /* end of include guard: WIZARD_FRAME_H_E923FBAE */
