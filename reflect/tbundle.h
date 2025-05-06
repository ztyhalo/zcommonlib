#ifndef TBUNDLE_H
#define TBUNDLE_H
#include <QtCore>

//
class TBundle
{
  public:
    TBundle();
    ~TBundle();
    bool     put(QString key, QVariant value);
    QVariant get(QString key);
    bool     putString(QString key, QString value);
    QString  getString(QString key);
    bool     putInt(QString key, int value);
    int      getInt(QString key);
    bool     putEnum(QString key, int value);
    int      getEnum(QString key);
    bool     contains(QString key);

  private:
    QMap< QString, QVariant > m_data;
};

#endif // TBUNDLE_H
