#include "filterable_item_picker.h"

FilterableItemPicker::FilterableItemPicker(wxWindow* parent, wxWindowID id,
                                           const DoubleMap<std::string>* items)
    : wxPanel(parent, id), items_(items) {
  source_filter_ = new wxTextCtrl(this, wxID_ANY);
  source_filter_->Bind(wxEVT_TEXT, &FilterableItemPicker::OnFilterEdited, this);

  source_list_ =
      new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
  UpdateSourceList();

  wxBoxSizer* filter_sizer = new wxBoxSizer(wxHORIZONTAL);
  filter_sizer->Add(new wxStaticText(this, wxID_ANY, "Filter:"),
                    wxSizerFlags().Center());
  filter_sizer->AddSpacer(10);
  filter_sizer->Add(source_filter_, wxSizerFlags().Proportion(1).Expand());

  wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
  left_sizer->Add(filter_sizer, wxSizerFlags().Expand());
  left_sizer->AddSpacer(10);
  left_sizer->Add(source_list_, wxSizerFlags().Proportion(1).Expand());

  SetSizerAndFit(left_sizer);

  source_list_->SetColumnWidth(0, wxLIST_AUTOSIZE);
}

std::optional<std::string> FilterableItemPicker::GetSelected() const {
  long selection = source_list_->GetFirstSelected();
  if (selection == -1) {
    return std::nullopt;
  }

  return source_list_->GetItemText(selection, 0).ToStdString();
}

void FilterableItemPicker::UpdateSourceList() {
  source_list_->ClearAll();
  source_list_->AppendColumn("Value");

  int i = 0;
  for (const std::string& list_item : items_->GetList()) {
    if (!source_filter_->GetValue().IsEmpty()) {
      wxString wx_list = wxString(list_item).Lower();
      if (wx_list.Find(source_filter_->GetValue().Lower()) == wxNOT_FOUND) {
        continue;
      }
    }

    source_list_->InsertItem(i, list_item);
    i++;
  }
}

void FilterableItemPicker::OnFilterEdited(wxCommandEvent&) {
  UpdateSourceList();
}
