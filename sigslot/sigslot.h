#ifndef __SIGSLOT_H__
#define __SIGSLOT_H__

#include <algorithm>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <map>

using namespace std;

// PARA_TMP 数据参数模板 能够表示不同的回调参数类型

template < class PARA_TMP >
class ZSlotBase
{
  public:
    virtual void Exec(PARA_TMP param1) = 0;
    virtual ~ZSlotBase()               = 0;
};
// SLOT_OWN 操函数的拥有者

template < class SLOT_OWN, class PARA_TMP >
class ZSlotImpl : public ZSlotBase< PARA_TMP >
{
  public:
    ZSlotImpl(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pObj = pObj;
        m_Func = func;
    }

    void Exec(PARA_TMP param1)
    {
        (m_pObj->*m_Func)(param1);
    }

  private:
    SLOT_OWN* m_pObj;
    void (SLOT_OWN::*m_Func)(PARA_TMP);
};

template < class PARA_TMP >
class ZSlot
{
  public:
    template < class SLOT_OWN >
    ZSlot(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pSlotBase = new ZSlotImpl< SLOT_OWN, PARA_TMP >(pObj, func);
    }

    ~ZSlot()
    {
        delete m_pSlotBase;
    }

    void Exec(PARA_TMP param1)
    {
        m_pSlotBase->Exec(param1);
    }

  private:
    ZSlotBase< PARA_TMP >* m_pSlotBase;
};

template < class PARA_TMP >
class ZSignal
{
  public:
    template < class SLOT_OWN >
    void Bind(SLOT_OWN* pObj, void (SLOT_OWN::*func)(PARA_TMP))
    {
        m_pSlotSet.push_back(new ZSlot< PARA_TMP >(pObj, func));
    }

    ~ZSignal()
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            delete m_pSlotSet[i];
        }
    }

    void operator()(PARA_TMP param1)
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            m_pSlotSet[i]->Exec(param1);
        }
    }

  private:
    vector< ZSlot< PARA_TMP >* > m_pSlotSet;
};

#define P_Connect(sender, signal, receiver, method) ((sender)->signal.Bind(receiver, method))

// PARA_TMP 数据参数模板 能够表示不同的回调参数类型

class No_SlotBase
{
  public:
    virtual void Exec(void) = 0;
    virtual ~No_SlotBase()
    {
    }
};
// SLOT_OWN 操函数的拥有者

template < class SLOT_OWN >
class No_SlotImpl : public No_SlotBase
{
  public:
    No_SlotImpl(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pObj = pObj;
        m_Func = func;
    }

    void Exec(void)
    {
        (m_pObj->*m_Func)();
    }

  private:
    SLOT_OWN* m_pObj;
    void (SLOT_OWN::*m_Func)(void);
};

class No_Slot
{
  public:
    template < class SLOT_OWN >
    No_Slot(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pSlotBase = new No_SlotImpl< SLOT_OWN >(pObj, func);
    }

    virtual ~No_Slot()
    {
        if (m_pSlotBase != nullptr)
        {
            delete m_pSlotBase;
            m_pSlotBase = nullptr;
        }
    }

    void Exec(void)
    {
        m_pSlotBase->Exec();
    }

  private:
    No_SlotBase* m_pSlotBase;
};

class No_Signal
{
  public:
    template < class SLOT_OWN >
    void Bind(SLOT_OWN* pObj, void (SLOT_OWN::*func)(void))
    {
        m_pSlotSet.push_back(new No_Slot(pObj, func));
    }

    ~No_Signal()
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            delete m_pSlotSet[i];
        }
    }

    void operator()(void)
    {
        for (int i = 0; i < (int) m_pSlotSet.size(); i++)
        {
            m_pSlotSet[i]->Exec();
        }
    }

  private:
    vector< No_Slot* > m_pSlotSet;
};

#endif /*__SIGSLOT_H__*/
