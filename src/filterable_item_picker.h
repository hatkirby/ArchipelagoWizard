#ifndef FILTERABLE_ITEM_PICKER_H_6860BEAD
#define FILTERABLE_ITEM_PICKER_H_6860BEAD

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>

#include <optional>

#include "double_map.h"

class FilterableItemPicker : public wxPanel {
 public:
  FilterableItemPicker(wxWindow* parent, wxWindowID id,
                       const DoubleMap<std::string>* items);

  std::optional<std::string> GetSelected() const;

 private:
  void UpdateSourceList();

  void OnFilterEdited(wxCommandEvent& event);

  const DoubleMap<std::string>* items_;

  wxTextCtrl* source_filter_;
  wxListView* source_list_;
};

#endif /* end of include guard: FILTERABLE_ITEM_PICKER_H_6860BEAD */
