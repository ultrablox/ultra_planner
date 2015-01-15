//
//  USSingleton.h
//  UltraSimpleEngine
//
//  Created by Yuri Blokhin on 05.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef UltraSimpleEngine_USSingleton_h
#define UltraSimpleEngine_USSingleton_h

/*
Singletones can be called from any point of application code.
*/

template <class T> class USAbstractSingleton
{
	/*
	Base for singleton with static variable instance.
	*/
public:
    static T * setSingleton(T* instance)
	{
		m_pInstance = instance;
		return m_pInstance;
	}

protected:
    USAbstractSingleton();
    virtual ~USAbstractSingleton();
protected:
    static T * m_pInstance;
};

template <class T> T* USAbstractSingleton<T>::m_pInstance=0;


template <class T> class USSingleton : public USAbstractSingleton<T>
{
	/*
	Singleton with manual creation. First time you must
	create the instance and call setSingleton().
	*/
public:
    static T* singleton()
    {
        return USAbstractSingleton<T>::m_pInstance;
    }
};

template <class T> class USAutocreateSingleton : public USAbstractSingleton<T>
{
public:
    /*
	Singleton with automatic creation on first call.
	*/
    static T* singleton()
    {
		if(!USAbstractSingleton<T>::m_pInstance)
			USAbstractSingleton<T>::m_pInstance = new T;
        return USAbstractSingleton<T>::m_pInstance;
    }
};

#endif
