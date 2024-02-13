#ifndef WIZARD_FRAME_H_E923FBAE
#define WIZARD_FRAME_H_E923FBAE

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>

#include "game_definition.h"
#include "world.h"

class wxListView;
class WorldWindow;
class wxSplitterWindow;

class WizardFrame : public wxFrame {
 public:
  WizardFrame();

 private:
  void OnExit(wxCommandEvent& event);
  void OnWorldSelected(wxListEvent& event);

  wxSplitterWindow* splitter_window_;
  wxListView* world_list_;
  WorldWindow* world_window_;

  std::unique_ptr<GameDefinitions> game_definitions_;

  std::vector<std::unique_ptr<World>> worlds_;
};

#endif /* end of include guard: WIZARD_FRAME_H_E923FBAE */
