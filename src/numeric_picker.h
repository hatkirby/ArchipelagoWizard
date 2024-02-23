#ifndef NUMERIC_PICKER_H_A80423E2
#define NUMERIC_PICKER_H_A80423E2

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/slider.h>
#include <wx/spinctrl.h>

wxDECLARE_EVENT(EVT_PICK_NUMBER, wxCommandEvent);

class NumericPicker : public wxPanel {
 public:
  NumericPicker(wxWindow* parent, wxWindowID id, int min, int max,
                int default_value);

  int GetValue() const;

  void SetValue(int v);

 private:
  void OnSliderChanged(wxCommandEvent& event);
  void OnSpinChanged(wxSpinEvent& event);

  int min_;
  int max_;
  int value_;

  wxSlider* slider_;
  wxSpinCtrl* spin_ctrl_;
};

#endif /* end of include guard: NUMERIC_PICKER_H_A80423E2 */
