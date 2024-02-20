#include "option_set_dialog.h"

#include "double_map.h"
#include "filterable_item_picker.h"
#include "util.h"

OptionSetDialog::OptionSetDialog(const Game* game,
                                 const std::string& option_name,
                                 const OptionValue& option_value)
    : wxDialog(nullptr, wxID_ANY, "Value Picker"),
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
  wxStaticBoxSizer* lists_sizer =
      new wxStaticBoxSizer(wxHORIZONTAL, lists_panel, "Option Values");

  item_picker_ = new FilterableItemPicker(
      lists_sizer->GetStaticBox(), wxID_ANY,
      &GetOptionSetElements(*game_, option_definition_->name));

  wxButton* add_btn =
      new wxButton(lists_sizer->GetStaticBox(), wxID_ANY, "Add");
  add_btn->Bind(wxEVT_BUTTON, &OptionSetDialog::OnAddClicked, this);

  wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
  left_sizer->Add(item_picker_, wxSizerFlags().Proportion(1).Expand());
  left_sizer->AddSpacer(10);
  left_sizer->Add(add_btn, wxSizerFlags().Center());

  lists_sizer->Add(left_sizer,
                   wxSizerFlags().DoubleBorder().Proportion(1).Expand());

  // Set up the chosen list
  chosen_list_ = new wxDataViewListCtrl(lists_sizer->GetStaticBox(), wxID_ANY);

  if (option_definition_->type == kDictOption) {
    chosen_list_->AppendTextColumn("Value", wxDATAVIEW_CELL_INERT,
                                   wxCOL_WIDTH_AUTOSIZE);

#ifdef __WXMAC__
    chosen_list_->AppendTextColumn("Amount", wxDATAVIEW_CELL_INERT,
                                   wxCOL_WIDTH_AUTOSIZE);
#else
    wxDataViewSpinRenderer* amount_renderer =
        new wxDataViewSpinRenderer(1, 1000);
    chosen_list_->AppendColumn(new wxDataViewColumn("Amount", amount_renderer,
                                                    1, wxCOL_WIDTH_AUTOSIZE),
                               "long");
#endif
  } else {
    chosen_list_->AppendTextColumn("Value");
  }

  const DoubleMap<std::string>& option_set =
      GetOptionSetElements(*game_, option_name);
  if (option_definition_->type == kDictOption) {
    for (const auto& [id, amount] : option_value.dict_values) {
      std::string str_val = option_set.GetValue(id);

      wxVector<wxVariant> data;
      data.push_back(wxVariant(str_val));
#ifdef __WXMAC__
      data.push_back(wxVariant(std::to_string(amount)));
#else
      data.push_back(wxVariant(amount));
#endif
      chosen_list_->AppendItem(data);

      picked_.insert(str_val);
    }
  } else if (option_definition_->type == kSetOption) {
    for (int i = 0; i < option_value.set_values.size(); i++) {
      if (option_value.set_values.at(i)) {
        std::string str_val = option_set.GetValue(i);

        wxVector<wxVariant> data;
        data.push_back(wxVariant(str_val));
        chosen_list_->AppendItem(data);

        picked_.insert(str_val);
      }
    }
  }

  wxButton* remove_btn =
      new wxButton(lists_sizer->GetStaticBox(), wxID_ANY, "Remove");
  remove_btn->Bind(wxEVT_BUTTON, &OptionSetDialog::OnRemoveClicked, this);

  wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);
  right_sizer->Add(chosen_list_, wxSizerFlags().Proportion(1).Expand());
  right_sizer->AddSpacer(10);
  right_sizer->Add(remove_btn, wxSizerFlags().Center());

  lists_sizer->Add(right_sizer,
                   wxSizerFlags().DoubleBorder().Proportion(1).Expand());

  lists_panel->SetSizerAndFit(lists_sizer);
  top_sizer->Add(lists_panel, wxSizerFlags().DoubleBorder().Expand());
  top_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand());

  // Finish up the form.
  SetSizer(top_sizer);
  Layout();
  SetMinSize(GetSize());
  Fit();

  int width = option_header->GetClientSize().GetWidth();
  option_header->SetLabel(option_definition_->display_name);
  option_header->Wrap(width);

  option_description->SetLabel(option_definition_->description);
  option_description->Wrap(width);

  Fit();
  CentreOnParent();

  // UpdateSourceList();
}

OptionValue OptionSetDialog::GetOptionValue() const {
  const DoubleMap<std::string>& option_set =
      GetOptionSetElements(*game_, option_definition_->name);

  OptionValue option_value;

  if (option_definition_->type == kDictOption) {
    for (int i = 0; i < chosen_list_->GetItemCount(); i++) {
      wxString str_sel = chosen_list_->GetTextValue(i, 0);

#ifdef __WXMAC__
      wxString str_amount = chosen_list_->GetTextValue(i, 1);

      long amount_value;
      if (str_amount.ToLong(&amount_value)) {
#else
      wxVariant amount_variant;
      chosen_list_->GetValue(amount_variant, i, 1);
      long amount_value = amount_variant.GetLong();
      {
#endif
        option_value.dict_values[option_set.GetId(str_sel.ToStdString())] =
            amount_value;
      }
    }
  } else if (option_definition_->type == kSetOption) {
    option_value.set_values.resize(option_set.size());

    for (const std::string& name : picked_) {
      option_value.set_values[option_set.GetId(name)] = true;
    }
  }

  return option_value;
}

void OptionSetDialog::OnAddClicked(wxCommandEvent& event) {
  std::optional<std::string> selected_text = item_picker_->GetSelected();
  if (!selected_text) {
    return;
  }

  if (picked_.count(*selected_text)) {
    return;
  }

  wxVector<wxVariant> data;
  data.push_back(wxVariant(*selected_text));
  if (option_definition_->type == kDictOption) {
#ifdef __WXMAC__
    data.push_back(wxVariant("1"));
#else
    data.push_back(wxVariant(1));
#endif
  }
  chosen_list_->AppendItem(data);

  picked_.insert(*selected_text);
}

void OptionSetDialog::OnRemoveClicked(wxCommandEvent& event) {
  int selection = chosen_list_->GetSelectedRow();
  if (selection == wxNOT_FOUND) {
    return;
  }

  picked_.erase(chosen_list_->GetTextValue(selection, 0).ToStdString());
  chosen_list_->DeleteItem(selection);
}
