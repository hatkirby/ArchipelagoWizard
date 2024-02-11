#include "wizard_editor.h"

#include <wx/statline.h>

enum WizardEditorIds { ID_CONNECT = 1, ID_CHECK_FOR_UPDATES = 2 };

WizardEditor::WizardEditor(wxWindow* parent,
                           const GameDefinitions* game_definitions)
    : wxScrolledWindow(parent), game_definitions_(game_definitions) {
  name_box_ = new wxTextCtrl(this, wxID_ANY);
  game_box_ = new wxChoice(this, wxID_ANY);

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

  SetScrollRate(5, 5);

  Rebuild();
}

void WizardEditor::Rebuild() {
  if (other_options_ != nullptr) {
    other_options_->Destroy();
    other_options_ = nullptr;
    form_options_.clear();
  }

  if (world_.HasGame()) {
    other_options_ = new wxPanel(this, wxID_ANY);

    const std::vector<OptionDefinition>& game_options =
        game_definitions_->GetGameOptions(world_.GetGame());

    wxFlexGridSizer* options_form_sizer = new wxFlexGridSizer(2, 10, 10);
    options_form_sizer->AddGrowableCol(1);

    for (const OptionDefinition& game_option : game_options) {
      form_options_.emplace_back();
      FormOption& form_option = form_options_.back();

      options_form_sizer->Add(new wxStaticText(other_options_, wxID_ANY,
                                               game_option.display_name + ":"),
                              wxSizerFlags().Align(wxALIGN_TOP | wxALIGN_LEFT));

      if (game_option.type == kSelectOption) {
        form_option.combo_box = new wxChoice(other_options_, wxID_ANY);

        int default_selection = 0;
        for (const auto& [value_id, value_display] : game_option.choices) {
          if (value_id == game_option.default_choice) {
            default_selection = form_option.combo_box->GetCount();
          }
          form_option.combo_box->Append(value_display);
        }

        form_option.combo_box->SetSelection(default_selection);

        options_form_sizer->Add(form_option.combo_box, wxSizerFlags().Expand());
      } else if (game_option.type == kRangeOption) {
        form_option.slider = new wxSlider(
            other_options_, wxID_ANY, game_option.default_range_value,
            game_option.min_value, game_option.max_value);
        form_option.slider->Bind(
            wxEVT_SLIDER, &FormOption::OnRangeSliderChanged, &form_option);

        form_option.label =
            new wxStaticText(other_options_, wxID_ANY,
                             std::to_string(game_option.default_range_value));

        wxFlexGridSizer* range_sizer = new wxFlexGridSizer(2, 10, 10);
        range_sizer->AddGrowableCol(0);

        range_sizer->Add(form_option.slider, wxSizerFlags().Expand());
        range_sizer->Add(form_option.label,
                         wxSizerFlags().Align(wxALIGN_RIGHT));

        options_form_sizer->Add(range_sizer, wxSizerFlags().Expand());
      }
    }

    other_options_->SetSizerAndFit(options_form_sizer);
    top_sizer_->Add(other_options_, wxSizerFlags().DoubleBorder().Expand());
  }

  SetSizer(top_sizer_);
  Layout();
  FitInside();
}

void WizardEditor::OnChangeGame(wxCommandEvent& event) {
  world_.SetGame(game_box_->GetString(game_box_->GetSelection()).ToStdString());

  Rebuild();
}

void FormOption::OnRangeSliderChanged(wxCommandEvent& event) {
  if (slider == nullptr || label == nullptr) {
    return;
  }

  label->SetLabel(std::to_string(slider->GetValue()));
  label->GetContainingSizer()->Layout();
}
