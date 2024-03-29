#include "random_choice_dialog.h"

#include "game_definition.h"
#include "numeric_picker.h"
#include "world.h"

RandomChoiceDialog::RandomChoiceDialog(
    const OptionDefinition* option_definition, const OptionValue& option_value)
    : wxDialog(nullptr, wxID_ANY, "Randomization Settings") {
  // Load the weights from the option value.
  for (const OptionValue& weight_value : option_value.weighting) {
    weights_[weight_value.string_value] = weight_value.weight;
  }

  for (const auto& [option_id, option_name] :
       option_definition->choices.GetItems()) {
    if (weights_.count(option_name)) {
      continue;
    }

    if (option_value.weighting.empty() &&
        option_name == option_definition->default_value.string_value) {
      weights_[option_name] = 50;
    } else {
      weights_[option_name] = 0;
    }
  }

  if (weights_.count("random")) {
    weights_["random"] = 0;
  }

  // Initialise the form.
  wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);

  wxPanel* desc_panel = new wxPanel(this, wxID_ANY);
  wxStaticBoxSizer* desc_sizer =
      new wxStaticBoxSizer(wxVERTICAL, desc_panel, "Option Description");

  wxStaticText* option_header =
      new wxStaticText(desc_sizer->GetStaticBox(), wxID_ANY, "");
  option_header->SetFont(option_header->GetFont().Bold());

  wxStaticText* option_description =
      new wxStaticText(desc_sizer->GetStaticBox(), wxID_ANY, "");

  desc_sizer->Add(option_header, wxSizerFlags().Expand());
  desc_sizer->AddSpacer(10);
  desc_sizer->Add(option_description, wxSizerFlags().Expand());
  desc_panel->SetSizer(desc_sizer);

  top_sizer->Add(desc_panel, wxSizerFlags().DoubleBorder().Expand());

  // Mode selector.
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

  wxFlexGridSizer* rows_sizer = new wxFlexGridSizer(2, 10, 10);
  rows_sizer->AddGrowableCol(1);

  for (int i = 0; i < option_definition->choices.GetItems().size(); i++) {
    const auto& [option_id, option_name] =
        option_definition->choices.GetItems().at(i);

    NumericPicker* row_input = new NumericPicker(
        weighted_sizer->GetStaticBox(), wxID_ANY, 0, 50, weights_[option_name]);

    row_input->Bind(EVT_PICK_NUMBER,
                    [this, ov = option_name, row_input](wxCommandEvent&) {
                      weights_[ov] = row_input->GetValue();
                    });

    rows_sizer->Add(
        new wxStaticText(weighted_sizer->GetStaticBox(), wxID_ANY,
                         option_definition->choice_names.at(
                             option_definition->choices.GetKeyId(option_id))));
    rows_sizer->Add(row_input, wxSizerFlags().Expand());
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

  int width = option_header->GetClientSize().GetWidth();
  option_header->SetLabel(option_definition->display_name);
  option_header->Wrap(width);

  option_description->SetLabel(option_definition->description);
  option_description->Wrap(width);

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
