#ifndef RANDOM_RANGE_DIALOG_H_A3AEEBE5
#define RANDOM_RANGE_DIALOG_H_A3AEEBE5

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>
#include <optional>
#include <sstream>
#include <string>

#include "world.h"

class NumericPicker;
struct OptionDefinition;

struct RrdValue {
  std::optional<int> static_value;
  RandomValueType type = kUNKNOWN_RANDOM_VALUE_TYPE;
  int min = -1;
  int max = -1;

  RrdValue() = default;
  explicit RrdValue(const OptionValue& ov);
  OptionValue ToOptionValue() const;
  std::string ToString(const OptionDefinition& option_definition) const;
  bool operator<(const RrdValue& rhs) const;
};

class RandomRangeDialog : public wxDialog {
 public:
  RandomRangeDialog(const OptionDefinition* option_definition,
                    const OptionValue& option_value);

  OptionValue GetOptionValue() const;

 private:
  void OnOK(wxCommandEvent& event);
  void OnModeChanged(wxCommandEvent& event);
  void OnDeleteClicked(wxCommandEvent& event);

  void AddWeightRow(const RrdValue& value, wxWindow* parent, wxSizer* sizer,
                    int default_value = 0, int deleteable = true);

  struct WeightRow {
    wxStaticText* header_label;
    wxButton* delete_button;
    NumericPicker* row_slider;

    int weight = -1;
  };

  const OptionDefinition* option_definition_;

  wxRadioBox* modes_box_;

  wxPanel* regular_panel_;
  RandomValueType chosen_random_type_ = kUniformRandom;
  wxCheckBox* enable_range_subset_;
  NumericPicker* subset_min_;
  NumericPicker* subset_max_;
  wxButton* add_regular_button_;

  wxPanel* weighted_panel_;
  wxFlexGridSizer* weighted_sizer_;

  std::map<RrdValue, WeightRow> weights_;
  std::map<wxWindowID, RrdValue> value_by_delete_button_id_;
};

#endif /* end of include guard: RANDOM_RANGE_DIALOG_H_A3AEEBE5 */