#ifndef RANDOM_CHOICE_DIALOG_H_9C5EB2F4
#define RANDOM_CHOICE_DIALOG_H_9C5EB2F4

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>
#include <string>

#include "world.h"

class OptionDefinition;

class RandomChoiceDialog : public wxDialog {
 public:
  RandomChoiceDialog(const OptionDefinition* option_definition,
                     const OptionValue& option_value);

  OptionValue GetOptionValue() const;

 private:
  void OnModeChanged(wxCommandEvent& event);

  wxRadioBox* modes_box_;
  wxPanel* weighted_panel_;

  std::map<std::string, int> weights_;
};

#endif /* end of include guard: RANDOM_CHOICE_DIALOG_H_9C5EB2F4 */
