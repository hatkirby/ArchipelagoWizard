#include "random_range_dialog.h"

#include <wx/spinctrl.h>

#include "game_definition.h"
#include "world.h"

RrdValue::RrdValue(const OptionValue& ov) {
  if (ov.random) {
    type = ov.range_random_type;

    if (ov.range_subset) {
      std::tie(min, max) = *ov.range_subset;
    }
  } else {
    static_value = ov.int_value;
  }
}

OptionValue RrdValue::ToOptionValue() const {
  OptionValue result;

  if (static_value != -1) {
    result.int_value = static_value;
  } else {
    result.random = true;
    result.range_random_type = type;

    if (min != -1) {
      result.range_subset = std::tuple<int, int>(min, max);
    }
  }

  return result;
}

std::string RrdValue::ToString() const {
  std::ostringstream output;

  if (static_value != -1) {
    output << static_value;
  } else if (min != -1) {
    output << min << " to " << max;

    switch (type) {
      case kUNKNOWN_RANDOM_VALUE_TYPE:
      case kUniformRandom: {
        break;
      }
      case kLowRandom: {
        output << " Low";
        break;
      }
      case kMiddleRandom: {
        output << " Middle";
        break;
      }
      case kHighRandom: {
        output << " High";
        break;
      }
    }
  } else {
    switch (type) {
      case kUNKNOWN_RANDOM_VALUE_TYPE: {
        output << "Error";
        break;
      }
      case kUniformRandom: {
        output << "Random";
        break;
      }
      case kLowRandom: {
        output << "Random Low";
        break;
      }
      case kMiddleRandom: {
        output << "Random Middle";
        break;
      }
      case kHighRandom: {
        output << "Random High";
        break;
      }
    }
  }

  return output.str();
}

bool RrdValue::operator<(const RrdValue& rhs) const {
  return std::tie(static_value, type, min, max) <
         std::tie(rhs.static_value, rhs.type, rhs.min, rhs.max);
}

RandomRangeDialog::RandomRangeDialog(const OptionDefinition* option_definition,
                                     const OptionValue& option_value)
    : wxDialog(nullptr, wxID_ANY, "Randomization Settings") {
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

  // Mode picker.
  const wxString mode_choices[] = {"Off", "On", "Weighted"};
  modes_box_ =
      new wxRadioBox(this, wxID_ANY, "Randomization Mode", wxDefaultPosition,
                     wxDefaultSize, 3, mode_choices);

  modes_box_->Bind(wxEVT_RADIOBOX, &RandomRangeDialog::OnModeChanged, this);
  top_sizer->Add(modes_box_, wxSizerFlags().DoubleBorder().Expand());

  // Regular randomization settings.
  regular_panel_ = new wxPanel(this, wxID_ANY);
  wxStaticBoxSizer* regular_box_sizer = new wxStaticBoxSizer(
      wxVERTICAL, regular_panel_, "Regular Randomization Options");

  wxRadioButton* uniform_button =
      new wxRadioButton(regular_box_sizer->GetStaticBox(), wxID_ANY, "Uniform",
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  wxRadioButton* low_button =
      new wxRadioButton(regular_box_sizer->GetStaticBox(), wxID_ANY, "Low");
  wxRadioButton* middle_button =
      new wxRadioButton(regular_box_sizer->GetStaticBox(), wxID_ANY, "Middle");
  wxRadioButton* high_button =
      new wxRadioButton(regular_box_sizer->GetStaticBox(), wxID_ANY, "High");

  uniform_button->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent&) {
    chosen_random_type_ = kUniformRandom;
  });
  low_button->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent&) {
    chosen_random_type_ = kLowRandom;
  });
  middle_button->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent&) {
    chosen_random_type_ = kMiddleRandom;
  });
  high_button->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent&) {
    chosen_random_type_ = kHighRandom;
  });

  wxBoxSizer* regular_affinity_sizer = new wxBoxSizer(wxVERTICAL);
  regular_affinity_sizer->Add(uniform_button);
  regular_affinity_sizer->AddSpacer(5);
  regular_affinity_sizer->Add(low_button);
  regular_affinity_sizer->AddSpacer(5);
  regular_affinity_sizer->Add(middle_button);
  regular_affinity_sizer->AddSpacer(5);
  regular_affinity_sizer->Add(high_button);

  if (option_value.random && option_value.weighting.empty()) {
    chosen_random_type_ = option_value.range_random_type;

    switch (option_value.range_random_type) {
      case kUNKNOWN_RANDOM_VALUE_TYPE: {
        // Invalid.
        break;
      }
      case kUniformRandom: {
        uniform_button->SetValue(true);
        break;
      }
      case kLowRandom: {
        low_button->SetValue(true);
        break;
      }
      case kMiddleRandom: {
        middle_button->SetValue(true);
        break;
      }
      case kHighRandom: {
        high_button->SetValue(true);
        break;
      }
    }
  } else {
    uniform_button->SetValue(true);
  }

  // Range subset form for regular randomization.
  wxFlexGridSizer* subset_sizer = new wxFlexGridSizer(3, 10, 10);
  subset_sizer->AddGrowableCol(1);

  int regular_min_value = option_value.range_subset
                              ? std::get<0>(*option_value.range_subset)
                              : option_definition->min_value;
  wxStaticText* subset_min_label =
      new wxStaticText(regular_box_sizer->GetStaticBox(), wxID_ANY,
                       std::to_string(regular_min_value));
  subset_min_ = new wxSlider(regular_box_sizer->GetStaticBox(), wxID_ANY,
                             regular_min_value, option_definition->min_value,
                             option_definition->max_value);
  subset_min_->Bind(
      wxEVT_SLIDER, [this, subset_min_label, subset_sizer](wxCommandEvent&) {
        subset_min_label->SetLabel(std::to_string(subset_min_->GetValue()));
        subset_sizer->Layout();
      });

  subset_sizer->Add(new wxStaticText(regular_box_sizer->GetStaticBox(),
                                     wxID_ANY, "Minimum:"));
  subset_sizer->Add(subset_min_, wxSizerFlags().Expand());
  subset_sizer->Add(subset_min_label);

  int regular_max_value = option_value.range_subset
                              ? std::get<1>(*option_value.range_subset)
                              : option_definition->max_value;
  wxStaticText* subset_max_label =
      new wxStaticText(regular_box_sizer->GetStaticBox(), wxID_ANY,
                       std::to_string(regular_max_value));
  subset_max_ = new wxSlider(regular_box_sizer->GetStaticBox(), wxID_ANY,
                             regular_max_value, option_definition->min_value,
                             option_definition->max_value);
  subset_max_->Bind(
      wxEVT_SLIDER, [this, subset_max_label, subset_sizer](wxCommandEvent&) {
        subset_max_label->SetLabel(std::to_string(subset_max_->GetValue()));
        subset_sizer->Layout();
      });

  subset_sizer->Add(new wxStaticText(regular_box_sizer->GetStaticBox(),
                                     wxID_ANY, "Maximum:"));
  subset_sizer->Add(subset_max_, wxSizerFlags().Expand());
  subset_sizer->Add(subset_max_label);

  enable_range_subset_ = new wxCheckBox(regular_box_sizer->GetStaticBox(),
                                        wxID_ANY, "Restrict to sub-range");
  enable_range_subset_->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
    subset_min_->Enable(enable_range_subset_->GetValue());
    subset_max_->Enable(enable_range_subset_->GetValue());
  });

  if (option_value.random && option_value.weighting.empty() &&
      option_value.range_subset) {
    enable_range_subset_->SetValue(true);
  } else {
    subset_min_->Disable();
    subset_max_->Disable();
  }

  wxBoxSizer* second_column = new wxBoxSizer(wxVERTICAL);
  second_column->Add(enable_range_subset_);
  second_column->AddSpacer(10);
  second_column->Add(subset_sizer, wxSizerFlags().Proportion(1).Expand());

  wxBoxSizer* regular_box_columns = new wxBoxSizer(wxHORIZONTAL);
  regular_box_columns->Add(regular_affinity_sizer,
                           wxSizerFlags().Proportion(1).Expand());
  regular_box_columns->Add(second_column,
                           wxSizerFlags().Proportion(1).Expand());

  add_regular_button_ = new wxButton(regular_box_sizer->GetStaticBox(),
                                     wxID_ANY, "Add to weights");

  regular_box_sizer->Add(regular_box_columns,
                         wxSizerFlags().Proportion(1).Expand());
  regular_box_sizer->AddSpacer(10);
  regular_box_sizer->Add(add_regular_button_, wxSizerFlags().Right());

  regular_panel_->SetSizer(regular_box_sizer);
  top_sizer->Add(regular_panel_, wxSizerFlags().DoubleBorder().Expand());

  // Set up the weighting form.
  weighted_panel_ = new wxPanel(this, wxID_ANY);
  wxStaticBoxSizer* weighted_box_sizer = new wxStaticBoxSizer(
      wxVERTICAL, weighted_panel_, "Weighted Randomization Options");

  weighted_sizer_ = new wxFlexGridSizer(4, 10, 10);
  weighted_sizer_->AddGrowableCol(1);

  // Add a control for adding static value rows.
  wxSpinCtrl* add_static_spin = new wxSpinCtrl(
      weighted_box_sizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition,
      wxDefaultSize, wxSP_ARROW_KEYS, option_definition->min_value,
      option_definition->max_value, option_definition->min_value);
  wxButton* add_static_button =
      new wxButton(weighted_box_sizer->GetStaticBox(), wxID_ANY, "Add");
  add_static_button->Bind(wxEVT_BUTTON, [this, add_static_spin,
                                         weighted_box_sizer](wxCommandEvent&) {
    RrdValue rrd_value;
    rrd_value.static_value = add_static_spin->GetValue();

    if (weights_.count(rrd_value)) {
      wxMessageBox("This option is already in the form.");
      return;
    }

    AddWeightRow(rrd_value, weighted_box_sizer->GetStaticBox(),
                 weighted_sizer_);
    Layout();
    Fit();
  });

  weighted_sizer_->Add(add_static_spin, wxSizerFlags().Expand());
  weighted_sizer_->Add(add_static_button);
  weighted_sizer_->Add(0, 0);
  weighted_sizer_->Add(0, 0);

  weighted_box_sizer->Add(weighted_sizer_,
                          wxSizerFlags().Proportion(1).Expand());
  weighted_panel_->SetSizer(weighted_box_sizer);

  top_sizer->Add(weighted_panel_, wxSizerFlags().DoubleBorder().Expand());
  top_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand());

  // Set up the buttons to add rows to the weighted form.
  add_regular_button_->Bind(
      wxEVT_BUTTON, [this, weighted_box_sizer](wxCommandEvent&) {
        RrdValue rrd_value;
        rrd_value.type = chosen_random_type_;

        if (enable_range_subset_->GetValue()) {
          rrd_value.min = subset_min_->GetValue();
          rrd_value.max = subset_max_->GetValue();

          if (rrd_value.min >= rrd_value.max) {
            wxMessageBox(
                "The range maximum must be greater than the range minimum.");
            return;
          }
        }

        if (weights_.count(rrd_value)) {
          wxMessageBox("This option is already in the form.");
          return;
        }

        AddWeightRow(rrd_value, weighted_box_sizer->GetStaticBox(),
                     weighted_sizer_);
        Layout();
        Fit();
      });

  // Load the weights from the option value.
  if (option_value.random && !option_value.weighting.empty()) {
    for (const OptionValue& weight_value : option_value.weighting) {
      AddWeightRow(RrdValue(weight_value), weighted_box_sizer->GetStaticBox(),
                   weighted_sizer_, weight_value.weight);
    }
  }

  // Add a default row if there isn't already one.
  if (!option_definition->default_value.random) {
    RrdValue default_rrd_value;
    default_rrd_value.static_value = option_definition->default_value.int_value;

    if (!weights_.count(default_rrd_value)) {
      int default_value = option_value.weighting.empty() ? 50 : 0;
      AddWeightRow(default_rrd_value, weighted_box_sizer->GetStaticBox(),
                   weighted_sizer_, default_value);
    }
  }

  // Bring in default values for weighting table.
  auto add_static_random_row = [this, weighted_box_sizer,
                                option_definition](RandomValueType rvt) {
    RrdValue rrd_value;
    rrd_value.type = rvt;

    if (weights_.count(rrd_value)) {
      return;
    }

    int default_value =
        option_definition->default_value.random &&
                option_definition->default_value.range_random_type == rvt
            ? 50
            : 0;
    AddWeightRow(rrd_value, weighted_box_sizer->GetStaticBox(), weighted_sizer_,
                 default_value, /*deleteable=*/false);
  };

  add_static_random_row(kUniformRandom);
  add_static_random_row(kLowRandom);
  add_static_random_row(kMiddleRandom);
  add_static_random_row(kHighRandom);

  // Enable the form based on the option value.
  if (option_value.random) {
    if (option_value.weighting.empty()) {
      modes_box_->SetSelection(1);

      regular_panel_->Enable();
      weighted_panel_->Disable();
      add_regular_button_->Hide();
    } else {
      modes_box_->SetSelection(2);

      regular_panel_->Enable();
      weighted_panel_->Enable();
      add_regular_button_->Show();
    }
  } else {
    modes_box_->SetSelection(0);

    regular_panel_->Disable();
    weighted_panel_->Disable();
    add_regular_button_->Hide();
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

void RandomRangeDialog::OnModeChanged(wxCommandEvent& event) {
  if (event.GetSelection() == 0) {
    regular_panel_->Disable();
    weighted_panel_->Disable();
    add_regular_button_->Hide();
  } else if (event.GetSelection() == 1) {
    regular_panel_->Enable();
    weighted_panel_->Disable();
    add_regular_button_->Hide();
  } else if (event.GetSelection() == 2) {
    regular_panel_->Enable();
    weighted_panel_->Enable();
    add_regular_button_->Show();
  }

  Layout();
  Fit();
}

OptionValue RandomRangeDialog::GetOptionValue() const {
  OptionValue result;

  if (modes_box_->GetSelection() == 1) {
    result.random = true;
    result.range_random_type = chosen_random_type_;

    if (enable_range_subset_->GetValue()) {
      result.range_subset = std::make_tuple<int, int>(subset_min_->GetValue(),
                                                      subset_max_->GetValue());
    }
  } else if (modes_box_->GetSelection() == 2) {
    result.random = true;

    for (const auto& [rrd_value, weight] : weights_) {
      if (weight.weight == 0) {
        continue;
      }

      OptionValue sub_option = rrd_value.ToOptionValue();
      sub_option.weight = weight.weight;

      result.weighting.push_back(sub_option);
    }
  }

  return result;
}

void RandomRangeDialog::AddWeightRow(const RrdValue& value, wxWindow* parent,
                                     wxSizer* sizer, int default_value,
                                     int deleteable) {
  WeightRow wr;
  wr.weight = default_value;
  wr.row_slider = new wxSlider(parent, wxID_ANY, default_value, 0, 50);
  wr.row_label =
      new wxStaticText(parent, wxID_ANY, std::to_string(default_value));

  wr.row_slider->Bind(wxEVT_SLIDER, [this, value](wxCommandEvent&) {
    WeightRow& wr = weights_[value];
    wr.weight = wr.row_slider->GetValue();
    wr.row_label->SetLabel(std::to_string(wr.row_slider->GetValue()));
    wr.row_label->GetContainingSizer()->Layout();
  });

  wr.header_label = new wxStaticText(parent, wxID_ANY, value.ToString());
  sizer->Add(wr.header_label);
  sizer->Add(wr.row_slider, wxSizerFlags().Expand());
  sizer->Add(wr.row_label, wxSizerFlags().Align(wxALIGN_RIGHT));

  if (deleteable) {
    wr.delete_button = new wxButton(parent, wxID_ANY, "X", wxDefaultPosition,
                                    wxDefaultSize, wxBU_EXACTFIT);
    wr.delete_button->Bind(wxEVT_BUTTON, &RandomRangeDialog::OnDeleteClicked,
                           this);

    sizer->Add(wr.delete_button);

    value_by_delete_button_id_[wr.delete_button->GetId()] = value;
  } else {
    sizer->Add(0, 0);
  }

  weights_[value] = std::move(wr);
}

void RandomRangeDialog::OnDeleteClicked(wxCommandEvent& event) {
  RrdValue rrd_value = value_by_delete_button_id_[event.GetId()];
  value_by_delete_button_id_.erase(event.GetId());

  const WeightRow& wr = weights_[rrd_value];
  wr.header_label->Destroy();
  wr.row_label->Destroy();
  wr.delete_button->Destroy();
  wr.row_slider->Destroy();
  Layout();
  Fit();

  weights_.erase(rrd_value);
}
