#ifndef WORLD_WINDOW_H_5F182828
#define WORLD_WINDOW_H_5F182828

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/notebook.h>

class GameDefinitions;
class WizardEditor;

class WorldWindow : public wxNotebook {
 public:
  WorldWindow(wxWindow* parent, const GameDefinitions* game_definitions);

 private:
  const GameDefinitions* game_definitions_;

  WizardEditor* wizard_editor_;
};

#endif /* end of include guard: WORLD_WINDOW_H_5F182828 */
