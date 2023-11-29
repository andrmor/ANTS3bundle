#ifndef AINTERFACETOMESSAGEWINDOW_H
#define AINTERFACETOMESSAGEWINDOW_H

#include "awindowinterfacebase.h"

#include <QObject>
#include <QVector>
#include <QString>

class AScriptManager;
class ATextOutputWindow;

class AMsg_SI : public AWindowInterfaceBase
{
  Q_OBJECT

public:
  AMsg_SI(ATextOutputWindow * msgWin);

  AScriptInterface * cloneBase() const {return new AMsg_SI(MsgWin);}

//  bool IsMultithreadCapable() const override {return true;}

public slots:
  void appendText(QString text);
  void appendHtml(QString text);
  void clear();

  void setFontSize(int size);

public:
//  void RestorelWidget();  //on script window restore
//  void HideWidget();     //on script window hide

private:
  ATextOutputWindow * MsgWin = nullptr;

};

#endif // AINTERFACETOMESSAGEWINDOW_H
