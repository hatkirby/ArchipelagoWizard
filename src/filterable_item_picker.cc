#include "filterable_item_picker.h"

wxDEFINE_EVENT(EVT_PICK_ITEM, wxCommandEvent);

FilterableItemPicker::FilterableItemPicker(wxWindow* parent, wxWindowID id,
                                           const DoubleMap<std::string>* items)
    : wxPanel(parent, id), items_(items) {
  source_filter_ = new wxTextCtrl(this, wxID_ANY);
  source_filter_->Bind(wxEVT_TEXT, &FilterableItemPicker::OnFilterEdited, this);

  source_list_ =
      new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
  source_list_->Bind(wxEVT_LEFT_DCLICK, &FilterableItemPicker::OnDoubleClick,
                     this);
  UpdateSourceList();

  wxBoxSizer* filter_sizer = new wxBoxSizer(wxHORIZONTAL);
  filter_sizer->Add(new wxStaticText(this, wxID_ANY, "Filter:"),
                    wxSizerFlags().Center());
  filter_sizer->AddSpacer(10);
  filter_sizer->Add(source_filter_, wxSizerFlags().Proportion(1).Expand());

  wxButton* add_btn = new wxButton(this, wxID_ANY, "Add");
  add_btn->Bind(wxEVT_BUTTON, &FilterableItemPicker::OnAddClicked, this);

  wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
  left_sizer->Add(filter_sizer, wxSizerFlags().Expand());
  left_sizer->AddSpacer(10);
  left_sizer->Add(source_list_, wxSizerFlags().Proportion(1).Expand());
  left_sizer->AddSpacer(10);
  left_sizer->Add(add_btn, wxSizerFlags().Center());

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

  source_list_->SetColumnWidth(0, wxLIST_AUTOSIZE);
}

void FilterableItemPicker::OnFilterEdited(wxCommandEvent&) {
  UpdateSourceList();
}

void FilterableItemPicker::OnAddClicked(wxCommandEvent& event) {
  std::optional<std::string> selected_text = GetSelected();
  if (!selected_text) {
    return;
  }

  wxCommandEvent picked_event(EVT_PICK_ITEM, GetId());
  picked_event.SetString(*selected_text);

  ProcessWindowEvent(picked_event);
}

void FilterableItemPicker::OnDoubleClick(wxMouseEvent& event) {
  std::optional<std::string> selected_text = GetSelected();
  if (!selected_text) {
    return;
  }

  wxCommandEvent picked_event(EVT_PICK_ITEM, GetId());
  picked_event.SetString(*selected_text);

  ProcessWindowEvent(picked_event);
}
