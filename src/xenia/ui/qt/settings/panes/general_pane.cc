#include "general_pane.h"

#include <QLabel>
#include <QVBoxLayout>

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

  auto settings_factory = SettingsWidgetFactory();
  auto settings_widget = settings_factory.BuildSettingsWidget("General");

  layout->addWidget(settings_widget);

  layout->addStretch();

  set_widget(scroll_area);
}


}  // namespace qt
}  // namespace ui
}  // namespace xe