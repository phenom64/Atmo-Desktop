#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include "../config/settings.h"
//#include <QSharedMemory>
#include <QDebug>
#include <QPointer>

#if HASDBUS
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <QSystemSemaphore>

//class QImage;

namespace DSP
{

class SharedWindowData;

//QSharedMemory doesnt work between qt4 and qt5 apps for some reason,
//below a minimal shm implementation
class Q_DECL_EXPORT SharedMemory
{
public:
    SharedMemory(const QString &key)
        : m_keyString(key)
        , m_key(key.toUInt())
        , m_sem(key, 1, QSystemSemaphore::Create)
        , m_isLocked(false)
        , m_isAttached(false)
        , m_data(0)
    {}
    virtual ~SharedMemory() { detach(); }
    const inline bool isLocked() const { return m_isLocked; }
    const inline bool isAttached() const { return m_isAttached; }
    inline void *data() { return m_data; }
    const inline int size() const { return m_size; }
    const inline void *constData() const { return m_data; }
    bool lock()
    {
        if (m_isLocked)
            return false;
        if (m_sem.acquire())
        {
            m_isLocked = true;
            return true;
        }
        m_isLocked = false;
        return false;
    }
    bool unlock()
    {
        if (m_isLocked && m_sem.release())
        {
            m_isLocked = false;
            return true;
        }
        m_isLocked = false;
        return false;
    }
    bool create(const quint32 size)
    {
        m_size = size;
        const int shmid = shmget(m_key, m_size, IPC_CREAT | IPC_EXCL /*| IPC_NOWAIT*/ | 0600);
        if (shmid == -1)
        {
            qDebug() << "SharedMemory::create(): unable to create shared memory segment for key" << m_key;
            return false;
        }
        if ((m_data = shmat(shmid, NULL, 0)) == (char *) -1)
        {
            qDebug() << "SharedMemory::create(): unable to attach shared memory segment for key" << m_key;
            return false;
        }
        if (lock())
        {
            memset(data(), 0, size);
            unlock();
        }
        return true;
    }
    bool attach()
    {
        int shmid = shmget(m_key, 0, 0600);
        if ((m_data = shmat(shmid, NULL, 0)) == (char *) -1)
        {
            qDebug() << "SharedMemory::attach(): unable to attach shared memory segment for key" << m_key;
            return false;
        }

        struct shmid_ds shmid_ds;
        if (!shmctl(shmid, IPC_STAT, &shmid_ds))
            m_size = (int)shmid_ds.shm_segsz;
        else
            return false;
        m_isAttached = true;
        return true;
    }
    bool detach()
    {
        if (!m_isAttached)
            return false;
        if (-1 == shmdt(m_data))
            return false;
        m_data = 0;
        m_size = 0;

        struct shmid_ds shmid_ds;
        int id = shmget(m_key, 0, 0600);
        shmctl(id, IPC_STAT, &shmid_ds);
        if (shmid_ds.shm_nattch == 0)
        if (-1 == shmctl(id, IPC_RMID, &shmid_ds))
        switch (errno)
        {
        case EINVAL:
            break;
        default:
            return false;
        }
        m_key = 0;
        m_isAttached = false;
        return true;
    }
private:
    void *m_data;
    bool m_isLocked, m_isAttached;
    QString m_keyString;
    key_t m_key;
    int m_size;
    QSystemSemaphore m_sem;
};

template <typename T>
class Q_DECL_EXPORT TypedSharedMemory : public SharedMemory
{
public:
    TypedSharedMemory(const QString &key) : SharedMemory(key), m_winId(key.toUInt()) {}
    inline bool create(){ return SharedMemory::create(sizeof(T)); }
    inline T *data() { return static_cast<T *>(SharedMemory::data()); }
    const inline T *constData() const { return static_cast<const T *>(SharedMemory::constData()); }
    quint32 m_winId;
};

typedef struct Q_DECL_EXPORT _DataStruct
{
    bool
    separator,
    contAware,
    uno,
    hor,
    winIcon,
    embedButtons,
    isActiveWin;

    quint8
    opacity,
    unoHeight,
    frame,
    titleHeight,
    shadowOpacity,
    leftEmbedSize,
    rightEmbedSize,
    followDecoShadow,
    illumination,
    textBevelOpacity,
    toolBtnGradientStopCount,
    windowGradientStopCount;

    qint8 buttons;

    quint32
    bgColor,
    fgColor,
    closeColor,
    minColor,
    maxColor,
    decoId,
    imageWidth,
    imageHeight,
    toolBtnGradientStops[32],
    windowGradientStops[32];

    uchar imageData[2048*256*4];
} DataStruct;

template <typename T>
class Pointer
{
public:
    Pointer(T *data = 0) : m_data(data) {}
    T *data() const { return m_data; }
    operator bool () const { return m_data; }

protected:
    T *m_data;
};

class Q_DECL_EXPORT DataStructSharedMemory : public QObject, public SharedMemory
{
    Q_OBJECT
public:
    DataStructSharedMemory(const QString &key, QObject *parent = 0) : QObject(parent), SharedMemory(key), m_winId(key.toUInt()) {}
    inline bool create(){ return SharedMemory::create(sizeof(DataStruct)); }
    inline DataStruct *data() { return static_cast<DataStruct *>(SharedMemory::data()); }
    const inline DataStruct *constData() const { return static_cast<const DataStruct *>(SharedMemory::constData()); }
    quint32 m_winId;
};

class Q_DECL_EXPORT WindowData : public Pointer<DataStructSharedMemory>
{
public:
    static WindowData memory(const quint32 wid, QObject *parent, const bool create = false);
    inline DataStruct *operator -> () { return dataStruct(); }
    inline WindowData &operator = (const WindowData &other) { m_data = other.m_data; return *this; }
    inline bool lock() { return data()&&data()->lock(); }
    inline bool unlock() { return data()&&data()->unlock(); }
    inline bool attach() { return data()&&data()->attach(); }
    inline bool detach() { return data()&&data()->detach(); }
    inline bool create() { return data()&&data()->create(); }
    const inline bool isAttached() const { return data()&&data()->isAttached(); }
    DataStruct *dataStruct() const { return data() ? data()->data() : 0; }

    static Gradient dataToGradient(const quint32 *data, const int stops);
    const Gradient buttonGradient()
    {
        const DataStruct *d = dataStruct();
        return dataToGradient(d->toolBtnGradientStops, d->toolBtnGradientStopCount);
    }
    const Gradient windowGradient()
    {
        const DataStruct *d = dataStruct();
        return dataToGradient(d->windowGradientStops, d->windowGradientStopCount);
    }
    void setButtonGradient(const Gradient g);
    void setWindowGradient(const Gradient g);

    const QImage image() const;

    inline const quint32 winId() const { return data() ? data()->m_winId : 0; }

    void sync();

protected:
//    explicit WindowData(const QString &key) : Pointer(new DataStructSharedMemory(key)) {}
    explicit WindowData(DataStructSharedMemory *data) : Pointer<DataStructSharedMemory>(data) {}
    WindowData() : Pointer<DataStructSharedMemory>() {}
};

class WindowDataLocker
{
public:
    WindowDataLocker(WindowData data) : data(data) { lockObtained = data.lock(); }
    ~WindowDataLocker() { data.unlock(); }
    bool lockObtained;
    WindowData data;
};

} //namespace

#endif //WINDOWDATA_H
