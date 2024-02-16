#include "random_choice_dialog.h"

#include "game_definition.h"
#include "world.h"

RandomChoiceDialog::RandomChoiceDialog(
    const OptionDefinition* option_definition, const OptionValue& option_value)
    : wxDialog(nullptr, wxID_ANY, "Randomization Settings") {
  // Load the weights from the option value.
  for (const OptionValue& weight_value : option_value.weighting) {
    weights_[weight_value.string_value] = weight_value.weight;
  }

  for (const auto& [option_id, option_display] :
       option_definition->choices.GetItems()) {
    if (weights_.count(option_id)) {
      continue;
    }

    if (option_value.weighting.empty() &&
        option_id == option_definition->default_value.string_value) {
      weights_[option_id] = 50;
    } else {
      weights_[option_id] = 0;
    }
  }

  if (weights_.count("random")) {
    weights_["random"] = 0;
  }

  // Initialise the form.
  wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);

  const wxString mode_choices[] = {"Off", "On", "Weighted"};
  modes_box_ =
      new wxRadioBox(this, wxID_ANY, "Randomization Mode", wxDefaultPosition,
                     wxDefaultSize, 3, mode_choices);
  if (option_value.random) {
    if (option_value.weighting.empty()) {
      modes_box_->SetSelection(1);
    } else {
      modes_box_->SetSelection(2);
    }
  } else {
    modes_box_->SetSelection(0);
  }

  modes_box_->Bind(wxEVT_RADIOBOX, &RandomChoiceDialog::OnModeChanged, this);
  top_sizer->Add(modes_box_, wxSizerFlags().DoubleBorder().Expand());

  // Set up the weighting form.
  weighted_panel_ = new wxPanel(this, wxID_ANY);
  wxStaticBoxSizer* weighted_sizer = new wxStaticBoxSizer(
      wxVERTICAL, weighted_panel_, "Weighted Randomization Options");

  wxFlexGridSizer* rows_sizer = new wxFlexGridSizer(3, 10, 10);
  rows_sizer->AddGrowableCol(1);

  for (int i = 0; i < option_definition->choices.GetItems().size(); i++) {
    const auto& [option_value, option_display] =
        option_definition->choices.GetItems().at(i);

    wxSlider* row_slider =
        new wxSlider(weighted_sizer->GetStaticBox(), wxID_ANY,
                     weights_[option_value], 0, 50);
    wxStaticText* row_label =
        new wxStaticText(weighted_sizer->GetStaticBox(), wxID_ANY,
                         std::to_string(weights_[option_value]));

    row_slider->Bind(wxEVT_SLIDER, [this, ov = option_value, row_slider,
                                    row_label](wxCommandEvent&) {
      weights_[ov] = row_slider->GetValue();
      row_label->SetLabel(std::to_string(row_slider->GetValue()));
      row_label->GetContainingSizer()->Layout();
    });

    rows_sizer->Add(new wxStaticText(weighted_sizer->GetStaticBox(), wxID_ANY,
                                     option_display));
    rows_sizer->Add(row_slider, wxSizerFlags().Expand());
    rows_sizer->Add(row_label, wxSizerFlags().Align(wxALIGN_RIGHT));
  }

  weighted_sizer->Add(rows_sizer, wxSizerFlags().Proportion(1).Expand());
  weighted_panel_->SetSizer(weighted_sizer);

  top_sizer->Add(weighted_panel_, wxSizerFlags().DoubleBorder().Expand());
  top_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand());

  if (!(option_value.random && !option_value.weighting.empty())) {
    weighted_panel_->Disable();
  }

  SetSizer(top_sizer);
  Layout();
  SetMinSize(GetSize());
  Fit();
  CentreOnParent();
}

void RandomChoiceDialog::OnModeChanged(wxCommandEvent& event) {
  if (event.GetSelection() == 2) {
    weighted_panel_->Enable();
  } else {
    weighted_panel_->Disable();
  }
}

OptionValue RandomChoiceDialog::GetOptionValue() const {
  OptionValue result;

  if (modes_box_->GetSelection() == 1) {
    result.random = true;
  } else if (modes_box_->GetSelection() == 2) {
    result.random = true;

    for (const auto& [option_value, weight] : weights_) {
      if (weight == 0) {
        continue;
      }

      OptionValue sub_option;
      sub_option.weight = weight;

      if (option_value == "random") {
        sub_option.random = true;
      } else {
        sub_option.string_value = option_value;
      }

      result.weighting.push_back(sub_option);
    }
  }

  return result;
}
