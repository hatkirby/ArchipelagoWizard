#include "item_dict_dialog.h"

#include "double_map.h"
#include "filterable_item_picker.h"
#include "util.h"

ItemDictDialog::ItemDictDialog(const Game* game, const std::string& option_name,
                               const OptionValue& option_value)
    : wxDialog(nullptr, wxID_ANY, "Item Configuration", wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      game_(game),
      option_definition_(&game->GetOption(option_name)) {
  // Initialize the form.
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

  // Set up the source list
  wxPanel* lists_panel = new wxPanel(this, wxID_ANY);
  wxBoxSizer* lists_sizer = new wxBoxSizer(wxHORIZONTAL);

  item_picker_ = new FilterableItemPicker(
      lists_panel, wxID_ANY,
      &GetOptionSetElements(*game_, option_definition_->name));

  item_picker_->Bind(EVT_PICK_ITEM, &ItemDictDialog::OnItemPicked, this);

  lists_sizer->Add(item_picker_,
                   wxSizerFlags().DoubleBorder().Proportion(1).Expand());

  // Set up the chosen list
  value_panel_ = new wxScrolledWindow(lists_panel, wxID_ANY);
  value_panel_->SetScrollRate(5, 5);

  value_sizer_ = new wxFlexGridSizer(3, 5, 5);
  value_sizer_->AddGrowableCol(0);
  value_panel_->SetSizer(value_sizer_);

  lists_sizer->Add(value_panel_,
                   wxSizerFlags().DoubleBorder().Proportion(1).Expand());

  lists_panel->SetSizerAndFit(lists_sizer);
  top_sizer->Add(lists_panel, wxSizerFlags().Expand());
  top_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand());

  // Finish up the form.
  SetSizer(top_sizer);
  Layout();
  Fit();

  int width = option_header->GetClientSize().GetWidth();
  option_header->SetLabel(option_definition_->display_name);
  option_header->Wrap(width);

  option_description->SetLabel(option_definition_->description);
  option_description->Wrap(width);

  Fit();
  CentreOnParent();

  // Load in existing values.
  const DoubleMap<std::string>& option_set =
      GetOptionSetElements(*game_, option_name);
  for (const auto& [id, amount] : option_value.dict_values) {
    AddRow(option_set.GetValue(id), value_panel_, value_sizer_, amount);
  }
}

OptionValue ItemDictDialog::GetOptionValue() const {
  const DoubleMap<std::string>& option_set =
      GetOptionSetElements(*game_, option_definition_->name);

  OptionValue option_value;

  for (const auto& [value, row] : values_) {
    option_value.dict_values[option_set.GetId(value)] = row.amount;
  }

  return option_value;
}

void ItemDictDialog::OnItemPicked(wxCommandEvent& event) {
  if (values_.count(event.GetString().ToStdString())) {
    return;
  }

  AddRow(event.GetString().ToStdString(), value_panel_, value_sizer_, 1);

  value_panel_->Layout();
  value_panel_->FitInside();
}

void ItemDictDialog::AddRow(const std::string& value, wxWindow* parent,
                            wxSizer* sizer, int default_value) {
  Row wr;
  wr.amount = default_value;
  wr.spin_ctrl =
      new wxSpinCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                     wxSP_ARROW_KEYS, 1, 1000, default_value);

  wr.spin_ctrl->Bind(wxEVT_SPINCTRL, [this, value](wxSpinEvent&) {
    Row& wr = values_[value];
    wr.amount = wr.spin_ctrl->GetValue();
  });

  wr.display_label = new wxStaticText(parent, wxID_ANY, value);
  sizer->Add(wr.display_label,
             wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL).Expand());
  sizer->Add(wr.spin_ctrl, wxSizerFlags().Expand());

  wr.delete_button = new wxButton(parent, wxID_ANY, "X", wxDefaultPosition,
                                  wxDefaultSize, wxBU_EXACTFIT);
  wr.delete_button->Bind(wxEVT_BUTTON, &ItemDictDialog::OnDeleteClicked, this);

  sizer->Add(wr.delete_button);

  value_by_delete_button_id_[wr.delete_button->GetId()] = value;

  values_[value] = std::move(wr);
}

void ItemDictDialog::OnDeleteClicked(wxCommandEvent& event) {
  std::string rrd_value = value_by_delete_button_id_[event.GetId()];
  value_by_delete_button_id_.erase(event.GetId());

  const Row& wr = values_[rrd_value];
  wr.display_label->Destroy();
  wr.spin_ctrl->Destroy();
  wr.delete_button->Destroy();
  Layout();
  Fit();

  values_.erase(rrd_value);
}