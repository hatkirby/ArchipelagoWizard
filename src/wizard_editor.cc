#include "wizard_editor.h"

#include <list>
#include <map>
#include <optional>

#include <wx/checklst.h>
#include <wx/collpane.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>

#include "item_dict_dialog.h"
#include "numeric_picker.h"
#include "option_set_dialog.h"
#include "random_choice_dialog.h"
#include "random_range_dialog.h"
#include "util.h"
#include "window_pool.h"
#include "world.h"

namespace {

enum WizardEditorIds { ID_CONNECT = 1, ID_CHECK_FOR_UPDATES = 2 };

constexpr int kMaxChoicesInChecklist = 15;

class WizardEditorImpl;

struct EditorPoolManager {
 public:
  explicit EditorPoolManager(wxWindow* parent)
      : parent_(parent),
        labels_(parent),
        choices_(parent),
        pickers_(parent),
        check_lists_(parent),
        toggle_buttons_(parent),
        buttons_(parent) {}

  wxWindow* parent_;
  WindowPool<wxStaticText> labels_;
  WindowPool<wxChoice> choices_;
  WindowPool<NumericPicker> pickers_;
  WindowPool<wxCheckListBox> check_lists_;
  WindowPool<wxToggleButton> toggle_buttons_;
  WindowPool<wxButton> buttons_;

  void Reset() {
    labels_.Reset();
    choices_.Reset();
    pickers_.Reset();
    check_lists_.Reset();
    toggle_buttons_.Reset();
    buttons_.Reset();
  }
};

class FormOption {
 public:
  FormOption(WizardEditorImpl* parent, EditorPoolManager& container,
             const std::string& option_name, wxSizer* sizer);

  ~FormOption();

  void PopulateFromWorld();

 private:
  friend class WizardEditor;

  void OnHoverLabel(wxMouseEvent& event);
  void OnRangePickerChanged(wxCommandEvent& event);
  void OnNamedRangeChanged(wxCommandEvent& event);
  void OnSelectChanged(wxCommandEvent& event);
  void OnListItemChecked(wxCommandEvent& event);
  void OnRandomClicked(wxCommandEvent& event);
  void OnOptionSetClicked(wxCommandEvent& event);
  void OnItemDictClicked(wxCommandEvent& event);

  void SaveToWorld();

  WizardEditorImpl* parent_;

  std::string option_name_;
  wxStaticText* option_label_ = nullptr;
  wxChoice* combo_box_ = nullptr;
  NumericPicker* numeric_picker_ = nullptr;
  wxCheckListBox* list_box_ = nullptr;
  wxToggleButton* random_button_ = nullptr;
  wxButton* open_choice_btn_ = nullptr;
};

class WizardEditorImpl : public WizardEditor {
 public:
  WizardEditorImpl(wxWindow* parent, const GameDefinitions* game_definitions);

  void LoadWorld(World* world) override;

  void Reload() override;

  void SetMessageCallback(
      std::function<void(const wxString&, const wxString&)> callback) override {
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
  void OnChangePreset(wxCommandEvent& event);

  const GameDefinitions* game_definitions_;

  World* world_ = nullptr;
  std::optional<std::string> cur_game_;
  bool first_time_ = true;

  wxTextCtrl* name_box_;
  wxTextCtrl* description_box_;
  wxChoice* game_box_;
  wxStaticText* preset_label_;
  wxChoice* preset_box_;
  wxPanel* other_options_ = nullptr;
  wxCollapsiblePane* common_options_pane_ = nullptr;
  wxCollapsiblePane* hidden_options_pane_ = nullptr;
  wxBoxSizer* top_sizer_;

  std::unique_ptr<EditorPoolManager> other_options_manager_;
  std::unique_ptr<EditorPoolManager> common_options_manager_;
  std::unique_ptr<EditorPoolManager> hidden_options_manager_;

  std::list<FormOption> form_options_;

  std::function<void(const wxString&, const wxString&)> message_callback_;
};

WizardEditorImpl::WizardEditorImpl(wxWindow* parent,
                                   const GameDefinitions* game_definitions)
    : WizardEditor(parent), game_definitions_(game_definitions) {
  name_box_ = new wxTextCtrl(this, wxID_ANY);
  name_box_->Bind(wxEVT_TEXT, &WizardEditorImpl::OnChangeName, this);

  description_box_ = new wxTextCtrl(this, wxID_ANY);
  description_box_->Bind(wxEVT_TEXT, &WizardEditorImpl::OnChangeDescription,
                         this);

  game_box_ = new wxChoice(this, wxID_ANY);

  game_box_->Append("");
  for (const std::string& game_name : game_definitions->GetAllGames()) {
    game_box_->Append(game_name);
  }

  game_box_->Bind(wxEVT_CHOICE, &WizardEditorImpl::OnChangeGame, this);

  preset_label_ = new wxStaticText(this, wxID_ANY, "Preset:");
  preset_label_->Hide();

  preset_box_ = new wxChoice(this, wxID_ANY);
  preset_box_->Bind(wxEVT_CHOICE, &WizardEditorImpl::OnChangePreset, this);
  preset_box_->Hide();

  wxFlexGridSizer* form_sizer = new wxFlexGridSizer(2, 10, 10);
  form_sizer->AddGrowableCol(1);

  form_sizer->Add(new wxStaticText(this, -1, "Name:"),
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(name_box_, wxSizerFlags().Expand());
  form_sizer->Add(new wxStaticText(this, -1, "Description:"),
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(description_box_, wxSizerFlags().Expand());
  form_sizer->Add(new wxStaticText(this, -1, "Game:"),
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(game_box_, wxSizerFlags().Expand());
  form_sizer->Add(preset_label_,
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(preset_box_, wxSizerFlags().Expand());

  top_sizer_ = new wxBoxSizer(wxVERTICAL);
  top_sizer_->Add(form_sizer, wxSizerFlags().DoubleBorder().Expand());
  top_sizer_->Add(new wxStaticLine(this));

  SetScrollRate(0, 20);

  other_options_ = new wxPanel(this, wxID_ANY);
  common_options_pane_ = new wxCollapsiblePane(
      this, wxID_ANY, "Advanced Options", wxDefaultPosition, wxDefaultSize,
      wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
  hidden_options_pane_ = new wxCollapsiblePane(
      this, wxID_ANY, "Hidden Options", wxDefaultPosition, wxDefaultSize,
      wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);

  common_options_pane_->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED,
                             [this](wxCollapsiblePaneEvent&) { FixSize(); });
  hidden_options_pane_->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED,
                             [this](wxCollapsiblePaneEvent&) { FixSize(); });

  other_options_manager_ = std::make_unique<EditorPoolManager>(other_options_);
  common_options_manager_ =
      std::make_unique<EditorPoolManager>(common_options_pane_->GetPane());
  hidden_options_manager_ =
      std::make_unique<EditorPoolManager>(hidden_options_pane_->GetPane());

  top_sizer_->Add(other_options_, wxSizerFlags().DoubleBorder().Expand());
  top_sizer_->Add(common_options_pane_,
                  wxSizerFlags().DoubleBorder().Proportion(0).Expand());
  top_sizer_->Add(hidden_options_pane_,
                  wxSizerFlags().DoubleBorder().Proportion(0).Expand());

  Rebuild();
}

void WizardEditorImpl::LoadWorld(World* world) {
  world_ = world;

  Rebuild();
}

void WizardEditorImpl::Reload() { Rebuild(); }

void WizardEditorImpl::Rebuild() {
  std::optional<std::string> next_game;
  if (world_ && world_->HasGame()) {
    next_game = world_->GetGame();
  }

  if (!first_time_ && cur_game_ == next_game) {
    Populate();
    Layout();

    return;
  }

  first_time_ = false;

  other_options_->Hide();
  common_options_pane_->Hide();
  hidden_options_pane_->Hide();

  form_options_.clear();

  other_options_manager_->Reset();
  common_options_manager_->Reset();
  hidden_options_manager_->Reset();

  if (world_ && world_->HasGame()) {
    const Game& game = game_definitions_->GetGame(world_->GetGame());

    wxFlexGridSizer* options_form_sizer = new wxFlexGridSizer(3, 10, 10);
    options_form_sizer->AddGrowableCol(1);

    std::vector<const OptionDefinition*> common_options;
    std::vector<const OptionDefinition*> hidden_options;
    for (const OptionDefinition& game_option : game.GetOptions()) {
      if (game_option.common) {
        common_options.push_back(&game_option);
      } else if (game_option.hidden) {
        hidden_options.push_back(&game_option);
      } else {
        form_options_.emplace_back(this, *other_options_manager_,
                                   game_option.name, options_form_sizer);
      }
    }

    other_options_->Show();
    other_options_->SetSizerAndFit(options_form_sizer);

    if (!common_options.empty()) {
      wxFlexGridSizer* common_options_sizer = new wxFlexGridSizer(3, 10, 10);
      common_options_sizer->AddGrowableCol(1);

      for (const OptionDefinition* game_option : common_options) {
        form_options_.emplace_back(this, *common_options_manager_,
                                   game_option->name, common_options_sizer);
      }

      common_options_pane_->GetPane()->SetSizerAndFit(common_options_sizer);
      common_options_pane_->Show();
    }

    if (!hidden_options.empty()) {
      wxFlexGridSizer* hidden_options_sizer = new wxFlexGridSizer(3, 10, 10);
      hidden_options_sizer->AddGrowableCol(1);

      for (const OptionDefinition* game_option : hidden_options) {
        form_options_.emplace_back(this, *hidden_options_manager_,
                                   game_option->name, hidden_options_sizer);
      }

      hidden_options_pane_->GetPane()->SetSizerAndFit(hidden_options_sizer);
      hidden_options_pane_->Show();
    }

    if (!game.GetPresets().empty()) {
      preset_box_->Clear();
      preset_box_->Append("");

      for (const auto& [preset_name, preset_options] : game.GetPresets()) {
        preset_box_->Append(preset_name);
      }

      preset_label_->Show();
      preset_box_->Show();
    } else {
      preset_label_->Hide();
      preset_box_->Hide();
    }
  } else {
    preset_label_->Hide();
    preset_box_->Hide();
  }

  Populate();
  FixSize();

  cur_game_ = next_game;
}

void WizardEditorImpl::Populate() {
  if (world_) {
    name_box_->ChangeValue(world_->GetName());
    description_box_->ChangeValue(world_->GetDescription());
  } else {
    name_box_->ChangeValue("");
    description_box_->ChangeValue("");
  }

  if (world_ && world_->HasGame()) {
    game_box_->SetSelection(game_box_->FindString(world_->GetGame()));

    for (FormOption& form_option : form_options_) {
      form_option.PopulateFromWorld();
    }
  } else {
    game_box_->SetSelection(0);
  }
}

void WizardEditorImpl::FixSize() {
  SetSizer(top_sizer_);
  Layout();
  FitInside();

  wxWindow* frame = wxGetTopLevelParent(this);
  frame->SetMinSize(frame->GetSize());
  frame->Fit();
  frame->SetMinSize(wxSize(728, 728 / 2));
}

void WizardEditorImpl::OnChangeName(wxCommandEvent& event) {
  world_->SetName(name_box_->GetValue().ToStdString());
}

void WizardEditorImpl::OnChangeDescription(wxCommandEvent& event) {
  world_->SetDescription(description_box_->GetValue().ToStdString());
}

void WizardEditorImpl::OnChangeGame(wxCommandEvent& event) {
  if (world_->HasSetOptions()) {
    if (wxMessageBox("This World has options set on it. Changing the game will "
                     "clear these options. Are you sure you want to proceed?",
                     "Confirm", wxYES_NO, this) == wxNO) {
      game_box_->SetSelection(game_box_->FindString(world_->GetGame()));
      return;
    }
  }

  if (game_box_->GetSelection() == 0) {
    world_->UnsetGame();
  } else {
    world_->SetGame(
        game_box_->GetString(game_box_->GetSelection()).ToStdString());
  }

  Rebuild();
}

void WizardEditorImpl::OnChangePreset(wxCommandEvent& event) {
  if (preset_box_->GetSelection() == 0) {
    return;
  }

  if (world_->HasSetOptions()) {
    if (wxMessageBox("This World has options set on it. Using a preset will "
                     "clear these options. Are you sure you want to proceed?",
                     "Confirm", wxYES_NO, this) == wxNO) {
      preset_box_->SetSelection(0);
      return;
    }
  }

  world_->ClearOptions();

  const Game& game = game_definitions_->GetGame(world_->GetGame());
  const std::map<std::string, OptionValue>& preset = game.GetPresets().at(
      preset_box_->GetString(preset_box_->GetSelection()).ToStdString());

  for (const auto& [option_name, option_value] : preset) {
    world_->SetOption(option_name, option_value);
  }

  Populate();
  Layout();
}

FormOption::FormOption(WizardEditorImpl* parent, EditorPoolManager& container,
                       const std::string& option_name, wxSizer* sizer)
    : parent_(parent), option_name_(option_name) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  option_label_ = container.labels_.Allocate("");
  option_label_->SetLabelText(game_option.display_name + ":");
  option_label_->Bind(wxEVT_ENTER_WINDOW, &FormOption::OnHoverLabel, this);
  sizer->Add(option_label_, wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));

  bool randomizable = false;

  if (game_option.type == kSelectOption) {
    combo_box_ = container.choices_.Allocate();
    combo_box_->Clear();

    for (const std::string& value_display : game_option.choice_names) {
      combo_box_->Append(value_display);
    }

    combo_box_->Bind(wxEVT_CHOICE, &FormOption::OnSelectChanged, this);
    sizer->Add(combo_box_, wxSizerFlags().Expand());

    randomizable = true;
  } else if (game_option.type == kRangeOption) {
    numeric_picker_ = container.pickers_.Allocate();
    numeric_picker_->SetMin(game_option.min_value);
    numeric_picker_->SetMax(game_option.max_value);
    numeric_picker_->SetValue(game_option.default_value.int_value);
    numeric_picker_->Bind(EVT_PICK_NUMBER, &FormOption::OnRangePickerChanged,
                          this);

    if (game_option.named_range) {
      combo_box_ = container.choices_.Allocate();
      combo_box_->Clear();

      for (const auto& [value_value, value_name] :
           game_option.value_names.GetItems()) {
        combo_box_->Append(ConvertToTitleCase(value_name));
      }
      combo_box_->Append("Custom");

      combo_box_->Bind(wxEVT_CHOICE, &FormOption::OnNamedRangeChanged, this);

      wxBoxSizer* named_sizer = new wxBoxSizer(wxVERTICAL);
      named_sizer->Add(combo_box_, wxSizerFlags().Expand());
      named_sizer->AddSpacer(5);
      named_sizer->Add(numeric_picker_, wxSizerFlags().Expand());

      sizer->Add(named_sizer, wxSizerFlags().Expand());
    } else {
      sizer->Add(numeric_picker_, wxSizerFlags().Expand());
    }

    randomizable = true;
  } else if (game_option.type == kSetOption ||
             game_option.type == kDictOption) {
    if (game_option.type == kSetOption && game_option.set_type == kCustomSet &&
        game_option.custom_set.size() <= kMaxChoicesInChecklist) {
      list_box_ = container.check_lists_.Allocate();
      list_box_->Clear();

      const DoubleMap<std::string>& option_set =
          GetOptionSetElements(game, option_name_);
      for (const std::string& name : option_set.GetList()) {
        list_box_->Append(name);
      }

      list_box_->Bind(wxEVT_CHECKLISTBOX, &FormOption::OnListItemChecked, this);

      sizer->Add(list_box_, wxSizerFlags().Expand());
    } else {
      open_choice_btn_ = container.buttons_.Allocate();
      open_choice_btn_->SetLabelText("Edit option");
      if (game_option.type == kDictOption) {
        open_choice_btn_->Bind(wxEVT_BUTTON, &FormOption::OnItemDictClicked,
                               this);
      } else {
        open_choice_btn_->Bind(wxEVT_BUTTON, &FormOption::OnOptionSetClicked,
                               this);
      }

      sizer->Add(open_choice_btn_, wxSizerFlags().Expand());
    }
  } else {
    wxStaticText* help_text = container.labels_.Allocate("");
    help_text->SetLabelText("YAML-only option.");
    sizer->Add(help_text);
  }

  if (randomizable) {
    std::string dice = "\xf0\x9f\x8e\xb2";
    random_button_ = container.toggle_buttons_.Allocate("");
    random_button_->SetLabelText(wxString::FromUTF8(dice));
    random_button_->SetWindowStyle(wxBU_EXACTFIT);
    random_button_->Bind(wxEVT_TOGGLEBUTTON, &FormOption::OnRandomClicked,
                         this);
    sizer->Add(random_button_);
  } else {
    sizer->Add(0, 0);
  }
}

FormOption::~FormOption() {
  const Game& game = parent_->game_definitions_->GetGame(*parent_->cur_game_);
  const OptionDefinition& game_option = game.GetOption(option_name_);

  option_label_->Unbind(wxEVT_ENTER_WINDOW, &FormOption::OnHoverLabel, this);

  if (game_option.type == kSelectOption) {
    combo_box_->Unbind(wxEVT_CHOICE, &FormOption::OnSelectChanged, this);
  } else if (game_option.type == kRangeOption) {
    numeric_picker_->Unbind(EVT_PICK_NUMBER, &FormOption::OnRangePickerChanged,
                            this);

    if (game_option.named_range) {
      combo_box_->Unbind(wxEVT_CHOICE, &FormOption::OnNamedRangeChanged, this);
    }
  } else if (game_option.type == kSetOption ||
             game_option.type == kDictOption) {
    if (game_option.type == kSetOption && game_option.set_type == kCustomSet &&
        game_option.custom_set.size() <= kMaxChoicesInChecklist) {
      list_box_->Unbind(wxEVT_CHECKLISTBOX, &FormOption::OnListItemChecked,
                        this);
    } else {
      if (game_option.type == kDictOption) {
        open_choice_btn_->Unbind(wxEVT_BUTTON, &FormOption::OnItemDictClicked,
                                 this);
      } else {
        open_choice_btn_->Unbind(wxEVT_BUTTON, &FormOption::OnOptionSetClicked,
                                 this);
      }
    }
  }

  if (random_button_ != nullptr) {
    random_button_->Unbind(wxEVT_TOGGLEBUTTON, &FormOption::OnRandomClicked,
                           this);
  }
}

void FormOption::PopulateFromWorld() {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  const OptionValue& ov = parent_->world_->HasOption(option_name_)
                              ? parent_->world_->GetOption(option_name_)
                              : game_option.default_value;

  if (ov.error) {
    option_label_->SetForegroundColour(*wxRED);
  } else {
    option_label_->SetForegroundColour(
        option_label_->GetParent()->GetForegroundColour());
  }

  if (game_option.type == kSelectOption) {
    if (ov.error) {
      combo_box_->Disable();
      random_button_->Disable();
      random_button_->SetValue(false);
    } else if (ov.random) {
      combo_box_->Disable();
      random_button_->SetValue(true);
      random_button_->Enable();
    } else {
      random_button_->SetValue(false);
      random_button_->Enable();

      int index = game_option.choices.GetValueId(ov.string_value);
      combo_box_->SetSelection(index);
      combo_box_->Enable();
    }
  } else if (game_option.type == kRangeOption) {
    if (ov.error) {
      numeric_picker_->Disable();
      random_button_->Disable();
      random_button_->SetValue(false);

      if (game_option.named_range) {
        combo_box_->Disable();
      }
    } else if (ov.random) {
      numeric_picker_->Disable();
      random_button_->SetValue(true);
      random_button_->Enable();

      if (game_option.named_range) {
        combo_box_->Disable();
      }
    } else {
      numeric_picker_->Enable();
      numeric_picker_->SetValue(ov.int_value);
      random_button_->Enable();
      random_button_->SetValue(false);

      if (game_option.named_range) {
        if (game_option.value_names.HasKey(ov.int_value)) {
          combo_box_->SetSelection(
              game_option.value_names.GetKeyId(ov.int_value));
        } else {
          combo_box_->SetSelection(combo_box_->GetCount() - 1);
        }
        combo_box_->Enable();
      }
    }
  } else if (game_option.type == kSetOption ||
             game_option.type == kDictOption) {
    if (list_box_ != nullptr) {
      if (ov.error) {
        list_box_->Disable();
      } else {
        list_box_->Enable();

        for (int i = 0; i < ov.set_values.size(); i++) {
          list_box_->Check(i, ov.set_values.at(i));
        }
      }
    } else if (open_choice_btn_ != nullptr) {
      if (ov.error) {
        open_choice_btn_->Disable();
      } else {
        open_choice_btn_->Enable();
      }
    }
  }
}

void FormOption::OnHoverLabel(wxMouseEvent& event) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  if (parent_->message_callback_) {
    if (parent_->world_->HasOption(game_option.name) &&
        parent_->world_->GetOption(game_option.name).error) {
      parent_->message_callback_(
          "Error", *parent_->world_->GetOption(game_option.name).error);
    } else {
      parent_->message_callback_(game_option.display_name,
                                 game_option.description);
    }
  }
}

void FormOption::OnRangePickerChanged(wxCommandEvent& event) {
  if (numeric_picker_ == nullptr) {
    return;
  }

  if (combo_box_ != nullptr) {
    const Game& game =
        parent_->game_definitions_->GetGame(parent_->world_->GetGame());
    const OptionDefinition& game_option = game.GetOption(option_name_);

    int selection;
    if (game_option.value_names.HasKey(numeric_picker_->GetValue())) {
      selection = game_option.value_names.GetKeyId(numeric_picker_->GetValue());
    } else {
      selection = combo_box_->GetCount() - 1;
    }

    if (combo_box_->GetSelection() != selection) {
      combo_box_->SetSelection(selection);
    }
  }

  SaveToWorld();
}

void FormOption::OnNamedRangeChanged(wxCommandEvent& event) {
  if (combo_box_ == nullptr || numeric_picker_ == nullptr) {
    return;
  }

  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  if (game_option.value_names.HasId(combo_box_->GetSelection())) {
    int result = game_option.value_names.GetKeyById(combo_box_->GetSelection());

    if (result != numeric_picker_->GetValue()) {
      numeric_picker_->SetValue(result);

      SaveToWorld();
    }
  }
}

void FormOption::OnSelectChanged(wxCommandEvent& event) { SaveToWorld(); }

void FormOption::OnListItemChecked(wxCommandEvent& event) { SaveToWorld(); }

void FormOption::OnRandomClicked(wxCommandEvent& event) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  const OptionValue& option_value =
      parent_->world_->HasOption(option_name_)
          ? parent_->world_->GetOption(option_name_)
          : game_option.default_value;
  random_button_->SetValue(option_value.random);

  if (game_option.type == kSelectOption) {
    RandomChoiceDialog rcd(&game_option, option_value);
    if (rcd.ShowModal() != wxID_OK) {
      return;
    }

    OptionValue dlg_value = rcd.GetOptionValue();

    // If randomization was just turned off, we need to choose a value to
    // fall back to. If the default is non-random, then we can just unset the
    // option, because that's basically the same as setting the default. If the
    // default is random, arbitrarily select the first choice.
    //
    // If randomization is still on, just copy the option value from the dialog
    // into the world.
    if (!dlg_value.random) {
      if (option_value.random) {
        if (game_option.default_value.random) {
          dlg_value.string_value =
              std::get<1>(game_option.choices.GetItems().at(0));
          parent_->world_->SetOption(option_name_, dlg_value);
        } else {
          parent_->world_->UnsetOption(option_name_);
        }
      }
    } else {
      parent_->world_->SetOption(option_name_, dlg_value);
    }
  } else if (game_option.type == kRangeOption) {
    RandomRangeDialog rrd(&game_option, option_value);
    if (rrd.ShowModal() != wxID_OK) {
      return;
    }

    OptionValue dlg_value = rrd.GetOptionValue();

    // If randomization was just turned off, we need to choose a value to
    // fall back to. If the default is non-random, then we can just unset the
    // option, because that's basically the same as setting the default. If the
    // default is random, arbitrarily select the minimum value.
    //
    // If randomization is still on, just copy the option value from the dialog
    // into the world.
    if (!dlg_value.random) {
      if (option_value.random) {
        if (game_option.default_value.random) {
          dlg_value.int_value = game_option.min_value;
          parent_->world_->SetOption(option_name_, dlg_value);
        } else {
          parent_->world_->UnsetOption(option_name_);
        }
      }
    } else {
      parent_->world_->SetOption(option_name_, dlg_value);
    }
  }

  PopulateFromWorld();
}

void FormOption::OnOptionSetClicked(wxCommandEvent& event) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  const OptionValue& ov = parent_->world_->HasOption(option_name_)
                              ? parent_->world_->GetOption(option_name_)
                              : game_option.default_value;

  OptionSetDialog osd(&game, option_name_, ov);
  if (osd.ShowModal() != wxID_OK) {
    return;
  }

  parent_->world_->SetOption(option_name_, osd.GetOptionValue());
}

void FormOption::OnItemDictClicked(wxCommandEvent& event) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  const OptionValue& ov = parent_->world_->HasOption(option_name_)
                              ? parent_->world_->GetOption(option_name_)
                              : game_option.default_value;

  ItemDictDialog idd(&game, option_name_, ov);
  if (idd.ShowModal() != wxID_OK) {
    return;
  }

  parent_->world_->SetOption(option_name_, idd.GetOptionValue());
}

void FormOption::SaveToWorld() {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  OptionValue new_value;
  if (game_option.type == kSelectOption) {
    new_value.string_value = std::get<1>(
        game_option.choices.GetItems().at(combo_box_->GetSelection()));
  } else if (game_option.type == kRangeOption) {
    new_value.int_value = numeric_picker_->GetValue();
  } else if (game_option.type == kSetOption) {
    if (list_box_ != nullptr) {
      for (size_t i = 0; i < list_box_->GetCount(); i++) {
        new_value.set_values.push_back(list_box_->IsChecked(i));
      }
    }
  }

  parent_->world_->SetOption(option_name_, std::move(new_value));
}

}  // namespace

WizardEditor* CreateWizardEditor(wxWindow* parent,
                                 const GameDefinitions* game_definitions) {
  return new WizardEditorImpl(parent, game_definitions);
}
