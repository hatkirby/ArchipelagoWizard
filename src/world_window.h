#ifndef WORLD_WINDOW_H_5F182828
#define WORLD_WINDOW_H_5F182828

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/notebook.h>

#include <functional>

class GameDefinitions;
class WizardEditor;
class World;
class YamlEditor;

class WorldWindow : public wxNotebook {
 public:
  WorldWindow(wxWindow* parent, const GameDefinitions* game_definitions);

  void LoadWorld(World* world);

  void SaveWorld();

  void SetMessageCallback(
      std::function<void(const wxString&, const wxString&)> callback);

 private:
  void OnPageChanged(wxBookCtrlEvent& event);

  const GameDefinitions* game_definitions_;

  WizardEditor* wizard_editor_;
  YamlEditor* yaml_editor_;

  World* world_;
};

#endif /* end of include guard: WORLD_WINDOW_H_5F182828 */
