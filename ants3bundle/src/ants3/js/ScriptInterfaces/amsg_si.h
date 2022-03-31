#ifndef AINTERFACETOMESSAGEWINDOW_H
#define AINTERFACETOMESSAGEWINDOW_H

#include "awindowinterfacebase.h"

#include <QObject>
#include <QVector>
#include <QString>

class AScriptManager;
class AScriptMsgWindow;

class AMsg_SI : public AWindowInterfaceBase
{
  Q_OBJECT

public:
  AMsg_SI(AScriptMsgWindow * msgWin);

//  bool IsMultithreadCapable() const override {return true;}

public slots:
  void append(const QString & text);
  void clear();

  void setFontSize(int size);

public:
//  void RestorelWidget();  //on script window restore
//  void HideWidget();     //on script window hide

private:
  AScriptMsgWindow * MsgWin = nullptr;

};

#endif // AINTERFACETOMESSAGEWINDOW_H
