#include "tbundle.h"

TBundle::TBundle()
{
}
TBundle::~TBundle()
{
}
bool TBundle::put(QString key, QVariant value)
{
    m_data.insert(key, value);
    return true;
}
QVariant TBundle::get(QString key)
{
    if (m_data.contains(key))
    {
        QVariant value = m_data.value(key);
        return value;
    }
    else
        return QVariant();
}
bool TBundle::putString(QString key, QString value)
{
    m_data.insert(key, QVariant(value));
    return true;
}
QString TBundle::getString(QString key)
{
    if (m_data.contains(key))
    {
        QVariant l_value = m_data.value(key);
        if (l_value.canConvert< QString >())
        {
            QString value = l_value.value< QString >();
            return value;
        }
        else
            return QString();
    }
    else
        return QString();
}
bool TBundle::putInt(QString key, int value)
{
    m_data.insert(key, QVariant(value));
    return true;
}
int TBundle::getInt(QString key)
{
    if (m_data.contains(key))
    {
        QVariant l_value = m_data.value(key);
        if (l_value.canConvert< int >())
        {
            int value = l_value.value< int >();
            return value;
        }
        else
            return 0;
    }
    else
        return false;
}
bool TBundle::putEnum(QString key, int value)
{
    m_data.insert(key, QVariant(value));
    return true;
}
int TBundle::getEnum(QString key)
{
    return getInt(key);
}
bool TBundle::contains(QString key)
{
    return m_data.contains(key);
}
