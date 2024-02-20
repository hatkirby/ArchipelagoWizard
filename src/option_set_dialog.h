#ifndef OPTION_SET_DIALOG_H_9F0D48DA
#define OPTION_SET_DIALOG_H_9F0D48DA

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dataview.h>
#include <wx/listctrl.h>

#include <set>
#include <string>

#include "game_definition.h"

class FilterableItemPicker;

class OptionSetDialog : public wxDialog {
 public:
  OptionSetDialog(const Game* game, const std::string& option_name,
                  const OptionValue& option_value);

  OptionValue GetOptionValue() const;

 private:
  void OnAddClicked(wxCommandEvent& event);
  void OnRemoveClicked(wxCommandEvent& event);

  const Game* game_;
  const OptionDefinition* option_definition_;
  FilterableItemPicker* item_picker_;
  wxDataViewListCtrl* chosen_list_;

  std::set<std::string> picked_;
};

#endif /* end of include guard: OPTION_SET_DIALOG_H_9F0D48DA */
