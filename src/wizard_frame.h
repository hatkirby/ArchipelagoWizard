#ifndef WIZARD_FRAME_H_E923FBAE
#define WIZARD_FRAME_H_E923FBAE

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "game_definition.h"

class wxListView;
class WorldWindow;
class wxSplitterWindow;

class WizardFrame : public wxFrame {
 public:
  WizardFrame();

 private:
  void OnExit(wxCommandEvent& event);

  wxSplitterWindow* splitter_window_;
  wxListView* world_list_;
  WorldWindow* world_window_;

  std::unique_ptr<GameDefinitions> game_definitions_;
};

#endif /* end of include guard: WIZARD_FRAME_H_E923FBAE */
