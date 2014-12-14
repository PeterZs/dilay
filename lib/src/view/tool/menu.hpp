#ifndef DILAY_VIEW_TOOL_MENU
#define DILAY_VIEW_TOOL_MENU

#include <QMenu>
#include <QString>
#include <glm/fwd.hpp>
#include "state.hpp"
#include "view/tool/menu-parameters.hpp"

class ViewToolMenu : public QMenu {
  public:
    DECLARE_BIG3 (ViewToolMenu, const glm::ivec2&)

    ViewToolMenuParameters menuParameters (const QString&) const;
    void                   addSeparator   ();

    template <typename T>
    void addAction (const QString& label) {
      QAction* a = this->QMenu::addAction (label);
      QObject::connect (a, &QAction::triggered, [this, label] () { 
          State::setTool (new T (this->menuParameters (label)));
      });
    }

  private:
    IMPLEMENTATION
};

#endif