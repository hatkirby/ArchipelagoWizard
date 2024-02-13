#ifndef WIZARD_EDITOR_H_AB195E2D
#define WIZARD_EDITOR_H_AB195E2D

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/scrolwin.h>

#include <list>
#include <map>

#include "game_definition.h"
#include "world.h"

class wxChoice;
class wxSlider;
class wxTextCtrl;
class wxBoxSizer;
class WizardEditor;

class FormOption {
 public:
  FormOption(WizardEditor* parent, const std::string& option_name,
             wxSizer* sizer);

  void PopulateFromWorld();

 private:
  void OnRangeSliderChanged(wxCommandEvent& event);
  void OnNamedRangeChanged(wxCommandEvent& event);
  void OnSelectChanged(wxCommandEvent& event);

  void SaveToWorld();

  WizardEditor* parent_;

  std::string option_name_;
  wxChoice* combo_box_ = nullptr;
  wxSlider* slider_ = nullptr;
  wxStaticText* label_ = nullptr;
};

class WizardEditor : public wxScrolledWindow {
 public:
  WizardEditor(wxWindow* parent, const GameDefinitions* game_definitions);

 private:
  friend class FormOption;

  void Rebuild();

  void Populate();

  void OnChangeGame(wxCommandEvent& event);

  const GameDefinitions* game_definitions_;

  World world_;

  wxTextCtrl* name_box_;
  wxChoice* game_box_;
  wxPanel* other_options_ = nullptr;
  wxBoxSizer* top_sizer_;

  std::list<FormOption> form_options_;
};

#endif /* end of include guard: WIZARD_EDITOR_H_AB195E2D */
