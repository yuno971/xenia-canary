#include "general_pane.h"

#include <QLabel>
#include <QVBoxLayout>

#include "xenia/ui/qt/settings/widgets/settings_checkbox.h"
#include "xenia/ui/qt/settings/widgets/settings_groupbox.h"
#include "xenia/ui/qt/widgets/combobox.h"
#include "xenia/ui/qt/widgets/groupbox.h"
#include "xenia/ui/qt/widgets/scroll_area.h"

DECLARE_bool(show_debug_tab);
DECLARE_bool(discord);
DECLARE_bool(use_game_icon);

namespace xe {
namespace ui {
namespace qt {

const QStringList game_languages = {
    "English", "Japanese", "German",  "French",     "Spanish",
    "Italian", "Korean",   "Chinese", "Portuguese", "Polish",
    "Russian", "Swedish",  "Turkish", "Norwegian",  "Dutch"};

void GeneralPane::Build() {
  QWidget* base_widget = new QWidget();
  base_widget->setSizePolicy(QSizePolicy::MinimumExpanding,
                             QSizePolicy::MinimumExpanding);

  // Setup scroll area for settings pane
  XScrollArea* scroll_area = new XScrollArea(this);
  scroll_area->setWidget(base_widget);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setWidgetResizable(true);

  QVBoxLayout* layout = new QVBoxLayout();
  base_widget->setLayout(layout);

  layout->setSpacing(16);
  layout->setContentsMargins(32, 16, 32, 16);

  // Add settings groupboxes to layout
  layout->addWidget(CreateGeneralGroupBox());
  layout->addWidget(CreateUpdateGroupBox());
  layout->addWidget(CreateWindowGroupBox());
  layout->addWidget(CreateLogGroupBox());
  layout->addStretch();

  set_widget(scroll_area);
}

XGroupBox* GeneralPane::CreateGeneralGroupBox() {
  auto groupbox = new SettingsGroupBox("General Settings");

  auto& config = Config::Instance();
  auto discord_cvar = config.FindConfigVar(cvars::discord);
  auto discord_checkbox = groupbox->CreateCheckBox("Discord Rich Presence", discord_cvar);

  discord_checkbox->set_update_config_fn(
      [discord_checkbox](bool value, cvar::ConfigVar<bool>& cvar) {
        cvar.set_config_value(value);
        discord_checkbox->UpdateLabel(
            tr("Please restart xenia for this change to take effect."));
      });

  return groupbox;
}

XGroupBox* GeneralPane::CreateUpdateGroupBox() {
  return new XGroupBox("Update Settings");
}

XGroupBox* GeneralPane::CreateWindowGroupBox() {
  return new XGroupBox("Window Settings");
}

XGroupBox* GeneralPane::CreateLogGroupBox() {
  return new XGroupBox("Log Settings");
}

}  // namespace qt
}  // namespace ui
}  // namespace xe