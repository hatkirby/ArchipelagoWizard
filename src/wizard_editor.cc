#include "wizard_editor.h"

#include <wx/statline.h>
#include <wx/tglbtn.h>

#include "item_dict_dialog.h"
#include "option_set_dialog.h"
#include "random_choice_dialog.h"
#include "random_range_dialog.h"
#include "util.h"
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

  description_box_ = new wxTextCtrl(this, wxID_ANY);
  description_box_->Bind(wxEVT_TEXT, &WizardEditor::OnChangeDescription, this);

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

  top_sizer_ = new wxBoxSizer(wxVERTICAL);
  top_sizer_->Add(form_sizer, wxSizerFlags().DoubleBorder().Expand());
  top_sizer_->Add(new wxStaticLine(this));

  SetScrollRate(0, 20);

  Rebuild();
}

void WizardEditor::LoadWorld(World* world) {
  world_ = world;

  Rebuild();
}

void WizardEditor::Reload() { Rebuild(); }

void WizardEditor::Rebuild() {
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

  if (other_options_ != nullptr) {
    other_options_->Destroy();
    other_options_ = nullptr;
    if (common_options_pane_ != nullptr) {
      common_options_pane_->Destroy();
      common_options_pane_ = nullptr;
    }
    form_options_.clear();
  }

  if (world_ && world_->HasGame()) {
    other_options_ = new wxPanel(this, wxID_ANY);

    const Game& game = game_definitions_->GetGame(world_->GetGame());

    wxFlexGridSizer* options_form_sizer = new wxFlexGridSizer(3, 10, 10);
    options_form_sizer->AddGrowableCol(1);

    std::vector<const OptionDefinition*> common_options;
    for (const OptionDefinition& game_option : game.GetOptions()) {
      if (game_option.common) {
        common_options.push_back(&game_option);
        continue;
      }
      form_options_.emplace_back(this, other_options_, game_option.name,
                                 options_form_sizer);
    }

    other_options_->SetSizerAndFit(options_form_sizer);
    top_sizer_->Add(other_options_, wxSizerFlags().DoubleBorder().Expand());

    if (!common_options.empty()) {
      common_options_pane_ = new wxCollapsiblePane(
          this, wxID_ANY, "Advanced Options", wxDefaultPosition, wxDefaultSize,
          wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);

      common_options_pane_->Bind(
          wxEVT_COLLAPSIBLEPANE_CHANGED,
          [this](wxCollapsiblePaneEvent&) { FixSize(); });

      wxFlexGridSizer* common_options_sizer = new wxFlexGridSizer(3, 10, 10);
      common_options_sizer->AddGrowableCol(1);

      for (const OptionDefinition* game_option : common_options) {
        form_options_.emplace_back(this, common_options_pane_->GetPane(),
                                   game_option->name, common_options_sizer);
      }

      common_options_pane_->GetPane()->SetSizerAndFit(common_options_sizer);
      top_sizer_->Add(common_options_pane_,
                      wxSizerFlags().DoubleBorder().Proportion(0).Expand());
    }
  }

  Populate();
  FixSize();

  cur_game_ = next_game;
}

void WizardEditor::Populate() {
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

void WizardEditor::FixSize() {
  SetSizer(top_sizer_);
  Layout();
  FitInside();

  wxWindow* frame = wxGetTopLevelParent(this);
  frame->SetMinSize(frame->GetSize());
  frame->Fit();
  frame->SetMinSize(wxSize(728, 728 / 2));
}

void WizardEditor::OnChangeName(wxCommandEvent& event) {
  world_->SetName(name_box_->GetValue().ToStdString());
}

void WizardEditor::OnChangeDescription(wxCommandEvent& event) {
  world_->SetDescription(description_box_->GetValue().ToStdString());
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

FormOption::FormOption(WizardEditor* parent, wxWindow* container,
                       const std::string& option_name, wxSizer* sizer)
    : parent_(parent), option_name_(option_name) {
  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  option_label_ = new wxStaticText(container, wxID_ANY, "");
  option_label_->SetLabelText(game_option.display_name + ":");
  option_label_->Bind(
      wxEVT_ENTER_WINDOW, [parent, &game_option](wxMouseEvent&) {
        if (parent->message_callback_) {
          if (parent->world_->HasOption(game_option.name) &&
              parent->world_->GetOption(game_option.name).error) {
            parent->message_callback_(
                "Error", *parent->world_->GetOption(game_option.name).error);
          } else {
            parent->message_callback_(game_option.display_name,
                                      game_option.description);
          }
        }
      });
  sizer->Add(option_label_, wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));

  bool randomizable = false;

  if (game_option.type == kSelectOption) {
    combo_box_ = new wxChoice(container, wxID_ANY);

    for (const auto& [value_id, value_display] :
         game_option.choices.GetItems()) {
      combo_box_->Append(value_display);
    }

    combo_box_->Bind(wxEVT_CHOICE, &FormOption::OnSelectChanged, this);
    sizer->Add(combo_box_, wxSizerFlags().Expand());

    randomizable = true;
  } else if (game_option.type == kRangeOption) {
    slider_ =
        new wxSlider(container, wxID_ANY, game_option.default_value.int_value,
                     game_option.min_value, game_option.max_value);
    slider_->Bind(wxEVT_SLIDER, &FormOption::OnRangeSliderChanged, this);

    label_ = new wxStaticText(container, wxID_ANY, "");

    wxFlexGridSizer* range_sizer = new wxFlexGridSizer(2, 10, 10);
    range_sizer->AddGrowableCol(0);

    range_sizer->Add(slider_, wxSizerFlags().Expand());
    range_sizer->Add(label_, wxSizerFlags().Align(wxALIGN_RIGHT));

    wxSizer* final_sizer = range_sizer;

    if (game_option.named_range) {
      combo_box_ = new wxChoice(container, wxID_ANY);

      for (const auto& [value_value, value_name] :
           game_option.value_names.GetItems()) {
        combo_box_->Append(ConvertToTitleCase(value_name));
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

    randomizable = true;
  } else if (game_option.type == kSetOption ||
             game_option.type == kDictOption) {
    if (game_option.type == kSetOption && game_option.set_type == kCustomSet &&
        game_option.custom_set.size() <= 15) {
      list_box_ = new wxCheckListBox(container, wxID_ANY);

      const DoubleMap<std::string>& option_set =
          GetOptionSetElements(game, option_name_);
      for (const std::string& name : option_set.GetList()) {
        list_box_->Append(name);
      }

      list_box_->Bind(wxEVT_CHECKLISTBOX, &FormOption::OnListItemChecked, this);

      sizer->Add(list_box_, wxSizerFlags().Expand());
    } else {
      open_choice_btn_ = new wxButton(container, wxID_ANY, "Edit option");
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
    sizer->Add(new wxStaticText(container, wxID_ANY, "YAML-only option."));
  }

  if (randomizable) {
    std::string dice = "\xf0\x9f\x8e\xb2";
    random_button_ =
        new wxToggleButton(container, wxID_ANY, wxString::FromUTF8(dice),
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

      int index = game_option.choices.GetKeyId(ov.string_value);
      combo_box_->SetSelection(index);
      combo_box_->Enable();
    }
  } else if (game_option.type == kRangeOption) {
    if (ov.error) {
      slider_->Disable();
      random_button_->Disable();
      random_button_->SetValue(false);

      if (game_option.named_range) {
        combo_box_->Disable();
      }
    } else if (ov.random) {
      slider_->Disable();
      random_button_->SetValue(true);
      random_button_->Enable();

      if (game_option.named_range) {
        combo_box_->Disable();
      }
    } else {
      slider_->Enable();
      slider_->SetValue(ov.int_value);
      label_->SetLabel(std::to_string(ov.int_value));
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

    int selection;
    if (game_option.value_names.HasKey(slider_->GetValue())) {
      selection = game_option.value_names.GetKeyId(slider_->GetValue());
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
  if (combo_box_ == nullptr || slider_ == nullptr) {
    return;
  }

  const Game& game =
      parent_->game_definitions_->GetGame(parent_->world_->GetGame());
  const OptionDefinition& game_option = game.GetOption(option_name_);

  if (game_option.value_names.HasId(combo_box_->GetSelection())) {
    int result = game_option.value_names.GetKeyById(combo_box_->GetSelection());

    if (result != slider_->GetValue()) {
      slider_->SetValue(result);

      if (label_ != nullptr) {
        label_->SetLabel(std::to_string(slider_->GetValue()));
        label_->GetContainingSizer()->Layout();
      }

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
              std::get<0>(game_option.choices.GetItems().at(0));
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
    new_value.string_value = std::get<0>(
        game_option.choices.GetItems().at(combo_box_->GetSelection()));
  } else if (game_option.type == kRangeOption) {
    new_value.int_value = slider_->GetValue();
  } else if (game_option.type == kSetOption) {
    if (list_box_ != nullptr) {
      for (size_t i = 0; i < list_box_->GetCount(); i++) {
        new_value.set_values.push_back(list_box_->IsChecked(i));
      }
    }
  }

  parent_->world_->SetOption(option_name_, std::move(new_value));
}
