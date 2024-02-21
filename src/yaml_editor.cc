#include "yaml_editor.h"

#include <wx/stc/stc.h>

#include "world.h"

YamlEditor::YamlEditor(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
  editor_ = new wxStyledTextCtrl(this, wxID_ANY);
  editor_->SetLexer(wxSTC_LEX_YAML);

  editor_->SetMarginWidth(0, 30);
  editor_->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(204, 204, 204));
  editor_->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(45, 45, 45));
  editor_->SetMarginType(0, wxSTC_MARGIN_NUMBER);

  editor_->SetWrapMode(wxSTC_WRAP_WORD);
  editor_->SetUseTabs(false);
  editor_->SetTabWidth(2);

  wxFont font(14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
  editor_->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
  editor_->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(204, 204, 204));
  editor_->StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(45, 45, 45));
  editor_->StyleClearAll();

  editor_->StyleSetForeground(wxSTC_YAML_DEFAULT, wxColour(204, 204, 204));
  editor_->StyleSetForeground(wxSTC_YAML_COMMENT, wxColour(153, 153, 153));
  editor_->StyleSetForeground(wxSTC_YAML_IDENTIFIER, wxColour(242, 119, 122));
  editor_->StyleSetForeground(wxSTC_YAML_KEYWORD, wxColour(249, 145, 87));
  editor_->StyleSetForeground(wxSTC_YAML_NUMBER, wxColour(249, 145, 87));
  editor_->StyleSetForeground(wxSTC_YAML_REFERENCE, wxColour(102, 204, 204));
  editor_->StyleSetForeground(wxSTC_YAML_DOCUMENT, wxColour(102, 204, 204));
  editor_->StyleSetForeground(wxSTC_YAML_TEXT, wxColour(153, 204, 153));
  editor_->StyleSetForeground(wxSTC_YAML_OPERATOR, wxColour(102, 204, 204));

  editor_->SetCaretForeground(wxColour(204, 204, 204));

  editor_->SetModEventMask(wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT |
                           wxSTC_PERFORMED_USER | wxSTC_PERFORMED_UNDO |
                           wxSTC_PERFORMED_REDO);
  editor_->Bind(wxEVT_STC_CHANGE, &YamlEditor::OnTextEdited, this);

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(editor_, wxSizerFlags().Proportion(1).Expand());
  SetSizer(sizer);
}

void YamlEditor::LoadWorld(World* world) {
  world_ = world;
  dirty_ = false;

  ignore_edit_ = true;
  editor_->SetText(world_->ToYaml());
  ignore_edit_ = false;
}

void YamlEditor::SaveWorld() {
  if (dirty_) {
    world_->FromYaml(editor_->GetText().ToStdString());
    dirty_ = false;
  }
}

void YamlEditor::OnTextEdited(wxStyledTextEvent& event) {
  if (ignore_edit_) {
    return;
  }

  dirty_ = true;
  if (!world_->IsDirty()) {
    world_->SetDirty(true);
  }
}
