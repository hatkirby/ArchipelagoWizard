#include "wizard_editor.h"

#include <wx/statline.h>
#include <wx/tglbtn.h>

#include "random_choice_dialog.h"
#include "world.h"

enum WizardEditorIds { ID_CONNECT = 1, ID_CHECK_FOR_UPDATES = 2 };

WizardEditor::WizardEditor(wxWindow* parent,
                           const GameDefinitions* game_definitions)
    : wxScrolledWindow(parent), game_definitions_(game_definitions) {
  name_box_ = new wxTextCtrl(this, wxID_ANY);
  name_box_->Bind(wxEVT_TEXT, &WizardEditor::OnChangeName, this);

  game_box_ = new wxChoice(this, wxID_ANY);

  game_box_->Append("");
  for (const std::string& game_name : game_definitions->GetAllGames()) {
    game_box_->Append(game_name);
  }

  game_box_->Bind(wxEVT_CHOICE, &WizardEditor::OnChangeGame, this);

  wxFlexGridSizer* form_sizer = new wxFlexGridSizer(2, 10, 10);
  form_sizer->AddGrowableCol(1);

  form_sizer->Add(new wxStaticText(this, -1, "Name:"),
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(name_box_, wxSizerFlags().Expand());
  form_sizer->Add(new wxStaticText(this, -1, "Game:"),
                  wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));
  form_sizer->Add(game_box_, wxSizerFlags().Expand());

  top_sizer_ = new wxBoxSizer(wxVERTICAL);
  top_sizer_->Add(form_sizer, wxSizerFlags().DoubleBorder().Expand());
  top_sizer_->Add(new wxStaticLine(this));

  SetScrollRate(0, 20);

  Rebuild();
}

void WizardEditor::LoadWorld(World* world) {
  world_ = world;

  name_box_->ChangeValue(world_->GetName());

  if (world_->HasGame()) {
    game_box_->SetSelection(game_box_->FindString(world_->GetGame()));
  } else {
    game_box_->SetSelection(0);
  }

  Rebuild();
}

void WizardEditor::Reload() {
  Populate();
  Layout();
}

void WizardEditor::Rebuild() {
  if (other_options_ != nullptr) {
    other_options_->Destroy();
    other_options_ = nullptr;
    form_options_.clear();
  }

  if (world_ && world_->HasGame()) {
    other_options_ = new wxPanel(this, wxID_ANY);

    const Game& game = game_definitions_->GetGame(world_->GetGame());

    wxFlexGridSizer* options_form_sizer = new wxFlexGridSizer(3, 10, 10);
    options_form_sizer->AddGrowableCol(1);

    for (const OptionDefinition& game_option : game.GetOptions()) {
      wxStaticText* option_label =
          new wxStaticText(other_options_, wxID_ANY, "");
      option_label->SetLabelText(game_option.display_name + ":");
      option_label->SetToolTip(game_option.description);
      options_form_sizer->Add(option_label,
                              wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));

      form_options_.emplace_back(this, game_option.name, options_form_sizer);
    }

    other_options_->SetSizerAndFit(options_form_sizer);
    top_sizer_->Add(other_options_, wxSizerFlags().DoubleBorder().Expand());

    Populate();
  }

  SetSizer(top_sizer_);
  Layout();
  FitInside();

  wxWindow* frame = wxGetTopLevelParent(this);
  frame->SetMinSize(frame->GetSize());
  frame->Fit();
  frame->SetMinSize(wxSize(728, 728 / 2));
}

void WizardEditor::Populate() {
  if (world_ && !world_->HasGame()) return;

  for (FormOption& form_option : form_options_) {
    form_option.PopulateFromWorld();
  }
}

void WizardEditor::OnChangeName(wxCommandEvent& event) {
  world_->SetName(name_box_->GetValue().ToStdString());
}

void WizardEditor::OnChangeGame(wxCommandEvent& event) {
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

FormOption::FormOption(WizardEditor* parent, const std::string& option_name,
                       wxSizer* sizer)
    : parent_(parent), option_name_(option_name) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  bool randomizable = false;

  if (game_option.type == kSelectOption) {
    combo_box_ = new wxChoice(parent_->other_options_, wxID_ANY);

    for (const auto& [value_id, value_display] :
         game_option.choices.GetItems()) {
      combo_box_->Append(value_display);
    }

    combo_box_->Bind(wxEVT_CHOICE, &FormOption::OnSelectChanged, this);
    sizer->Add(combo_box_, wxSizerFlags().Expand());

    randomizable = true;
  } else if (game_option.type == kRangeOption) {
    slider_ = new wxSlider(parent_->other_options_, wxID_ANY,
                           game_option.default_range_value,
                           game_option.min_value, game_option.max_value);
    slider_->Bind(wxEVT_SLIDER, &FormOption::OnRangeSliderChanged, this);

    label_ = new wxStaticText(parent_->other_options_, wxID_ANY, "");

    wxFlexGridSizer* range_sizer = new wxFlexGridSizer(2, 10, 10);
    range_sizer->AddGrowableCol(0);

    range_sizer->Add(slider_, wxSizerFlags().Expand());
    range_sizer->Add(label_, wxSizerFlags().Align(wxALIGN_RIGHT));

    wxSizer* final_sizer = range_sizer;

    if (game_option.named_range) {
      combo_box_ = new wxChoice(parent_->other_options_, wxID_ANY);

      for (const auto& [value_value, value_name] :
           game_option.value_names.GetItems()) {
        combo_box_->Append(value_name);
      }
      combo_box_->Append("Custom");

      combo_box_->Bind(wxEVT_CHOICE, &FormOption::OnNamedRangeChanged, this);

      wxBoxSizer* named_sizer = new wxBoxSizer(wxVERTICAL);
      named_sizer->Add(combo_box_, wxSizerFlags().Expand());
      named_sizer->AddSpacer(5);
      named_sizer->Add(range_sizer, wxSizerFlags().Expand());

      final_sizer = named_sizer;
    }

    sizer->Add(final_sizer, wxSizerFlags().Expand());

    // randomizable = true;
  } else if (game_option.type == kSetOption) {
    list_box_ = new wxCheckListBox(parent_->other_options_, wxID_ANY);

    for (const auto& [value_id, value_display] :
         game_option.choices.GetItems()) {
      list_box_->Append(value_display);
    }

    list_box_->Bind(wxEVT_CHECKLISTBOX, &FormOption::OnListItemChecked, this);

    sizer->Add(list_box_, wxSizerFlags().Expand());
  } else {
    sizer->Add(0, 0);
  }

  if (randomizable) {
    random_button_ =
        new wxToggleButton(parent_->other_options_, wxID_ANY, "🎲",
                           wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    random_button_->Bind(wxEVT_TOGGLEBUTTON, &FormOption::OnRandomClicked,
                         this);
    sizer->Add(random_button_);
  } else {
    sizer->Add(0, 0);
  }
}

void FormOption::PopulateFromWorld() {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  if (game_option.type == kSelectOption) {
    bool is_random = false;
    std::string str_sel;

    if (parent_->world_->HasOption(option_name_)) {
      const OptionValue& ov = parent_->world_->GetOption(option_name_);
      is_random = ov.random;
      str_sel = ov.string_value;
    } else {
      is_random = game_option.default_random;
      str_sel = game_option.default_choice;
    }

    if (is_random) {
      combo_box_->Disable();
      random_button_->SetValue(true);
    } else {
      random_button_->SetValue(false);

      int index = game_option.choices.GetKeyId(str_sel);
      combo_box_->SetSelection(index);
      combo_box_->Enable();
    }
  } else if (game_option.type == kRangeOption) {
    int int_sel = parent_->world_->HasOption(option_name_)
                      ? parent_->world_->GetOption(option_name_).int_value
                      : game_option.default_range_value;
    slider_->SetValue(int_sel);
    label_->SetLabel(std::to_string(int_sel));

    if (game_option.named_range) {
      std::optional<std::string> findstr =
          game_option.value_names.GetByKeyOptional(int_sel);
      if (!findstr) {
        findstr = "Custom";
      }
      combo_box_->SetSelection(combo_box_->FindString(*findstr));
    }
  } else if (game_option.type == kSetOption) {
    const std::vector<bool>& option_values =
        parent_->world_->HasOption(option_name_)
            ? parent_->world_->GetOption(option_name_).set_values
            : game_option.default_set_choices;
    for (int i = 0; i < option_values.size(); i++) {
      list_box_->Check(i, option_values.at(i));
    }
  }
}

void FormOption::OnRangeSliderChanged(wxCommandEvent& event) {
  if (slider_ == nullptr || label_ == nullptr) {
    return;
  }

  label_->SetLabel(std::to_string(slider_->GetValue()));
  label_->GetContainingSizer()->Layout();

  if (combo_box_ != nullptr) {
    const Game& game =
        parent_->game_definitions_->GetGame(parent_->world_->GetGame());
    const OptionDefinition& game_option = game.GetOption(option_name_);

    std::optional<std::string> findstr =
        game_option.value_names.GetByKeyOptional(slider_->GetValue());
    if (!findstr) {
      findstr = "Custom";
    }

    int selection = combo_box_->FindString(*findstr);
    if (combo_box_->GetSelection() != selection) {
      combo_box_->SetSelection(selection);
    }
  }

  SaveToWorld();
}

void FormOption::OnNamedRangeChanged(wxCommandEvent& event) {
  if (combo_box_ == nullptr || slider_ == nullptr) {
    return;
  }

  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  std::string str_sel =
      combo_box_->GetString(combo_box_->GetSelection()).ToStdString();
  std::optional<int> result =
      game_option.value_names.GetByValueOptional(str_sel);
  if (!result) {
    result = slider_->GetValue();
  }

  if (result != slider_->GetValue()) {
    slider_->SetValue(*result);

    if (label_ != nullptr) {
      label_->SetLabel(std::to_string(slider_->GetValue()));
      label_->GetContainingSizer()->Layout();
    }

    SaveToWorld();
  }
}

void FormOption::OnSelectChanged(wxCommandEvent& event) { SaveToWorld(); }

void FormOption::OnListItemChecked(wxCommandEvent& event) { SaveToWorld(); }

void FormOption::OnRandomClicked(wxCommandEvent& event) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  OptionValue option_value;
  option_value.random = game_option.default_random;
  if (parent_->world_->HasOption(option_name_)) {
    option_value = parent_->world_->GetOption(option_name_);
  }

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
        if (game_option.default_random) {
          dlg_value.string_value =
              std::get<0>(game_option.choices.GetItems().at(0));
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

void FormOption::SaveToWorld() {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  OptionValue new_value;
  if (game_option.type == kSelectOption) {
    new_value.string_value = std::get<0>(
        game_option.choices.GetItems().at(combo_box_->GetSelection()));
  } else if (game_option.type == kRangeOption) {
    new_value.int_value = slider_->GetValue();
  } else if (game_option.type == kSetOption) {
    for (int i = 0; i < game_option.choices.GetItems().size(); i++) {
      new_value.set_values.push_back(list_box_->IsChecked(i));
    }
  }

  parent_->world_->SetOption(option_name_, std::move(new_value));
}
