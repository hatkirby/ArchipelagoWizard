#ifndef YAML_EDITOR_H_BB0F5830
#define YAML_EDITOR_H_BB0F5830

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class wxStyledTextCtrl;
class wxStyledTextEvent;
class World;

class YamlEditor : public wxPanel {
 public:
  explicit YamlEditor(wxWindow* parent);

  void LoadWorld(World* world);

  void SaveWorld();

 private:
  void OnTextEdited(wxStyledTextEvent& event);

  wxStyledTextCtrl* editor_;
  World* world_;

  bool dirty_ = false;
  bool ignore_edit_ = false;
};

#endif /* end of include guard: YAML_EDITOR_H_BB0F5830 */
