#include "numeric_picker.h"

wxDEFINE_EVENT(EVT_PICK_NUMBER, wxCommandEvent);

NumericPicker::NumericPicker(wxWindow* parent, wxWindowID id, int min, int max,
                             int default_value)
    : wxPanel(parent, id), min_(min), max_(max), value_(default_value) {
  slider_ = new wxSlider(this, wxID_ANY, value_, min_, max_);
  slider_->Bind(wxEVT_SLIDER, &NumericPicker::OnSliderChanged, this);

  spin_ctrl_ =
      new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                     wxSP_ARROW_KEYS, min_, max_, value_);
  spin_ctrl_->Bind(wxEVT_SPINCTRL, &NumericPicker::OnSpinChanged, this);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(slider_, wxSizerFlags().Proportion(1).Expand());
  sizer->AddSpacer(10);
  sizer->Add(spin_ctrl_);

  SetSizer(sizer);
}

int NumericPicker::GetValue() const { return value_; }

void NumericPicker::SetValue(int v) {
  if (value_ != v) {
    value_ = v;

    if (slider_->GetValue() != value_) {
      slider_->SetValue(value_);
    }
    if (spin_ctrl_->GetValue() != value_) {
      spin_ctrl_->SetValue(std::to_string(value_));
    }

    wxCommandEvent picked_event(EVT_PICK_NUMBER, GetId());
    picked_event.SetInt(value_);

    ProcessWindowEvent(picked_event);
  }
}

void NumericPicker::OnSliderChanged(wxCommandEvent& event) {
  SetValue(slider_->GetValue());
}

void NumericPicker::OnSpinChanged(wxSpinEvent& event) {
  SetValue(spin_ctrl_->GetValue());
}
