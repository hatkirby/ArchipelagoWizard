#ifndef WIZARD_EDITOR_H_AB195E2D
#define WIZARD_EDITOR_H_AB195E2D

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/checklst.h>
#include <wx/collpane.h>
#include <wx/scrolwin.h>

#include <functional>
#include <list>
#include <map>
#include <optional>

#include "game_definition.h"
#include "world.h"

class wxChoice;
class wxSlider;
class wxTextCtrl;
class wxBoxSizer;
class wxToggleButton;
class WizardEditor;
class World;

class FormOption {
 public:
  FormOption(WizardEditor* parent, wxWindow* container,
             const std::string& option_name, wxSizer* sizer);

  void PopulateFromWorld();

 private:
  friend class WizardEditor;

  void OnRangeSliderChanged(wxCommandEvent& event);
  void OnNamedRangeChanged(wxCommandEvent& event);
  void OnSelectChanged(wxCommandEvent& event);
  void OnListItemChecked(wxCommandEvent& event);
  void OnRandomClicked(wxCommandEvent& event);
  void OnOptionSetClicked(wxCommandEvent& event);
  void OnItemDictClicked(wxCommandEvent& event);

  void SaveToWorld();

  WizardEditor* parent_;

  std::string option_name_;
  wxStaticText* option_label_ = nullptr;
  wxChoice* combo_box_ = nullptr;
  wxSlider* slider_ = nullptr;
  wxStaticText* label_ = nullptr;
  wxCheckListBox* list_box_ = nullptr;
  wxToggleButton* random_button_ = nullptr;
  wxButton* open_choice_btn_ = nullptr;
};

class WizardEditor : public wxScrolledWindow {
 public:
  WizardEditor(wxWindow* parent, const GameDefinitions* game_definitions);

  void LoadWorld(World* world);

  void Reload();

  void SetMessageCallback(
      std::function<void(const wxString&, const wxString&)> callback) {
    message_callback_ = std::move(callback);
  }

 private:
  friend class FormOption;

  void Populate();

  void Rebuild();

  void FixSize();

  void OnChangeName(wxCommandEvent& event);
  void OnChangeDescription(wxCommandEvent& event);
  void OnChangeGame(wxCommandEvent& event);

  const GameDefinitions* game_definitions_;

  World* world_ = nullptr;
  std::optional<std::string> cur_game_;
  bool first_time_ = true;

  wxTextCtrl* name_box_;
  wxTextCtrl* description_box_;
  wxChoice* game_box_;
  wxPanel* other_options_ = nullptr;
  wxCollapsiblePane* common_options_pane_ = nullptr;
  wxBoxSizer* top_sizer_;

  std::list<FormOption> form_options_;

  std::function<void(const wxString&, const wxString&)> message_callback_;
};

#endif /* end of include guard: WIZARD_EDITOR_H_AB195E2D */
