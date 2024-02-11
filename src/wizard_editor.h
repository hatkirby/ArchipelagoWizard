#ifndef WIZARD_EDITOR_H_AB195E2D
#define WIZARD_EDITOR_H_AB195E2D

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/scrolwin.h>

#include "game_definition.h"
#include "world.h"

class wxChoice;
class wxSlider;
class wxTextCtrl;
class wxBoxSizer;

struct FormOption {
  std::string option_name;
  wxChoice* combo_box;
  wxSlider* slider;
};

class WizardEditor : public wxScrolledWindow {
 public:
  WizardEditor(wxWindow* parent, const GameDefinitions* game_definitions);

 private:
  void Rebuild();

  void OnChangeGame(wxCommandEvent& event);

  const GameDefinitions* game_definitions_;

  World world_;

  wxTextCtrl* name_box_;
  wxChoice* game_box_;
  wxPanel* other_options_ = nullptr;
  wxBoxSizer* top_sizer_;

  std::vector<FormOption> form_options_;
};

#endif /* end of include guard: WIZARD_EDITOR_H_AB195E2D */
