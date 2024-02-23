#ifndef ITEM_DICT_DIALOG_H_AFF769E3
#define ITEM_DICT_DIALOG_H_AFF769E3

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/spinctrl.h>

#include <map>
#include <string>

#include "game_definition.h"

class FilterableItemPicker;

class ItemDictDialog : public wxDialog {
 public:
  ItemDictDialog(const Game* game, const std::string& option_name,
                 const OptionValue& option_value);

  OptionValue GetOptionValue() const;

 private:
  void OnItemPicked(wxCommandEvent& event);
  void OnDeleteClicked(wxCommandEvent& event);

  void AddRow(const std::string& value, wxWindow* parent, wxSizer* sizer,
              int default_value = 1);

  struct Row {
    wxStaticText* display_label;
    wxSpinCtrl* spin_ctrl;
    wxButton* delete_button;

    int amount = 1;
  };

  const Game* game_;
  const OptionDefinition* option_definition_;
  FilterableItemPicker* item_picker_;
  wxScrolledWindow* value_panel_;
  wxFlexGridSizer* value_sizer_;

  std::map<std::string, Row> values_;
  std::map<wxWindowID, std::string> value_by_delete_button_id_;
};

#endif /* end of include guard: ITEM_DICT_DIALOG_H_AFF769E3 */
