#ifndef AGEOOBJECT_H
#define AGEOOBJECT_H

#include "amonitorconfig.h"

#include <QString>
#include <QStringList>

#include <vector>

class QJsonObject;
class QJsonArray;
class AGeoType;
class TGeoShape;
class AGeoShape;
class AGeoSpecial;
class AGridElementRecord;
class TGeoRotation;
class QRegularExpression;

class AGeoObject
{
public:
  AGeoObject(QString name = "", QString ShapeGenerationString = ""); //name must be unique!
  AGeoObject(const AGeoObject * objToCopy);  //normal object is created
  AGeoObject(const QString & name, const QString & container, int iMat, AGeoShape * shape,
             double x, double y, double z,   double phi, double theta, double psi); // for tmp only, needs positioning in the container and name uniqueness check!
  AGeoObject(AGeoType * objType, AGeoShape * shape = nullptr);  // pointers are assigned to the object properties! if no shape, default cube is formed
  ~AGeoObject();

  AGeoType    * Type  = nullptr;  // always created in the constructor!
  AGeoShape   * Shape = nullptr;  // allowed to remain nullptr after construction!
  AGeoSpecial * Role  = nullptr;   // mainly remains nullptr after construction!

  QString Name;
  int Material = 0;
  double Position[3];
  double Orientation[3];
  bool fLocked = false;
  bool fActive = true;

  QString PositionStr[3];
  QString OrientationStr[3];

  AGeoObject * Container = nullptr;
  std::vector<AGeoObject*> HostedObjects;

  //visualization properties
  int color = 1;
  int style = 1;
  int width = 1;

  bool readShapeFromString(const QString & GenerationString, bool OnlyCheck = false); // using parameter values taken from gui generation string
  void onMaterialRemoved(int imat); // assumes isMaterialInUse was already called (and returned false)!
  bool isWorld() const;

  int  getMaterial() const;

  const AGeoObject * isGeoConstInUse(const QRegularExpression & nameRegExp) const;
  void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);
  const AGeoObject * isGeoConstInUseRecursive(const QRegularExpression & nameRegExp) const;
  void replaceGeoConstNameRecursive(const QRegularExpression & nameRegExp, const QString & newName);

  //json for a single object
  void writeToJson(QJsonObject & json) const;
  void readFromJson(const QJsonObject & json);

  //recursive json, using single object json
  void writeAllToJarr(QJsonArray & jarr);
  QString readAllFromJarr(AGeoObject * World, const QJsonArray & jarr);  // returns "" if no errors

  //for composite
  AGeoObject* getContainerWithLogical();
  const AGeoObject* getContainerWithLogical() const;
  bool isCompositeMemeber() const;
  void refreshShapeCompositeMembers(AGeoShape * ExternalShape = nullptr); //safe to use on any AGeoObject; if ExternalShape is provided , it is updated; otherwise, Objects's shape is updated
  bool isInUseByComposite(); //safe to use on any AGeoObject
  void clearCompositeMembers();
  void removeCompositeStructure();
  void updateNameOfLogicalMember(const QString & oldName, const QString & newName);

/*
  //for grid
  AGeoObject* getGridElement();    // used in cpp - temporary commented in addObjectFirst
  void  updateGridElementShape();
  AGridElementRecord* createGridRecord();
*/

  //for monitor -> TODO: remove from here
  void updateMonitorShape();
  const AMonitorConfig * getMonitorConfig() const; //returns nullptr if obj is not a monitor

  //for stacks
  bool isStackMember() const;
  bool isStackReference() const;
  AGeoObject * getOrMakeStackReferenceVolume();  // for stack container or members
  void updateStack();  //called on one object of the set - it is used to calculate positions of other members!
  void updateAllStacks();

  // the following checks are always done DOWN the chain
  // for global effect, the check has to be performed on World (Top) object
  void removeHostedObject(AGeoObject * obj); //does not delete the removed object!
  AGeoObject * findObjectByName(const QString & name);
  void findObjectsByWildcard(const QString & name, QVector<AGeoObject*> & foundObjs);
  void changeLineWidthRecursive(int delta);
  bool isNameExists(const QString & name);
  bool isContainsLocked();
  bool isDisabled() const;
  void enableUp();
  void addObjectFirst(AGeoObject* Object);
  void addObjectLast(AGeoObject* Object);
  bool migrateTo(AGeoObject* objTo, bool fAfter = false, AGeoObject *reorderObj = nullptr);
  bool repositionInHosted(AGeoObject* objTo, bool fAfter);
  bool suicide(); // not possible for locked and static objects
  void recursiveSuicide(); // does not remove locked and static objects, but removes all unlocked objects down the chain
  void lockUpTheChain();
  void lockBuddies();
  void lockRecursively();
  void unlockAllInside();
  void clearAll();    // bad name!
  void clearContent();
  void updateWorldSize(double& XYm, double& Zm);
  bool isMaterialInUse(int imat, QString & volName) const;  //including disabled objects
  bool isMaterialInActiveUse(int imat) const;  //excluding disabled objects  !!!**** need? if yes, synchronize with previous
  void collectContainingObjects(std::vector<AGeoObject*> & vec) const;
  double getMaxSize() const;

  bool getPositionInWorld(double * worldPos) const;

  bool isContainerValidForDrop(QString &errorStr) const;

  AGeoObject * makeClone(AGeoObject * World); // returns nullptr if failed; garantees unique names if World is not nullptr; Slabs are not properly cloned while there is a special container with them!
  AGeoObject * makeCloneForInstance(const QString & suffix);

  void findAllInstancesRecursive(std::vector<AGeoObject*> &Instances);
  bool isContainInstanceRecursive() const;
  bool isInstanceMember() const;

  bool    isPossiblePrototype(QString * sReason = nullptr) const;
  QString makeItPrototype(AGeoObject * Prototypes);
  bool    isPrototypeInUseRecursive(const QString & PrototypeName, QStringList * Users = nullptr) const;

  //service propertie
  QString tmpContName;   //used only during load
  bool fExpanded = true; //gui only: expand status in the tree view
  TGeoRotation * TrueRot = nullptr;  // used only during population of TGeoManager
  double TruePos[3];                 // used only during population of TGeoManager

private:
  AGeoObject * findContainerUp(const QString & name);

  void constructorInit();

  void enforceUniqueNameForCloneRecursive(AGeoObject * World, AGeoObject & tmpContainer);
  void addSuffixToNameRecursive(const QString & suffix);

public:
  static QString GenerateRandomName();
  static QString GenerateRandomObjectName();
  static QString GenerateRandomPrototypeName();
  static QString GenerateRandomCompositeName();
  static QString GenerateRandomArrayName();
  static QString GenerateRandomGridName();
  static QString GenerateRandomStackName();
  static QString GenerateRandomMonitorName();

  static QString generateCloneObjName(const QString & initialName);

};

#endif // AGEOOBJECT_H
