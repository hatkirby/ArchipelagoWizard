#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wizard_frame.h"

class WizardApp : public wxApp {
 public:
  virtual bool OnInit() {
    WizardFrame *frame = new WizardFrame();
    frame->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(WizardApp);
