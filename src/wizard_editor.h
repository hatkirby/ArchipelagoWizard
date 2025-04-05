#ifndef WIZARD_EDITOR_H_AB195E2D
#define WIZARD_EDITOR_H_AB195E2D

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/scrolwin.h>

#include <functional>

#include "game_definition.h"

class World;

class WizardEditor : public wxScrolledWindow {
 public:
  explicit WizardEditor(wxWindow* parent) : wxScrolledWindow(parent) {}

  virtual void LoadWorld(World* world) = 0;

  virtual void Reload() = 0;

  virtual void SetMessageCallback(
      std::function<void(const wxString&, const wxString&)> callback) = 0;
};

WizardEditor* CreateWizardEditor(wxWindow* parent,
                                 const GameDefinitions* game_definitions);

#endif /* end of include guard: WIZARD_EDITOR_H_AB195E2D */
