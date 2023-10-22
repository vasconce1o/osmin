/*
 * Copyright (C) 2020
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platformextras.h"

#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>

#if defined(SAILFISHOS)
#define AUTO_MOUNT        "/run/media/"

#elif defined(Q_OS_ANDROID)
#include <QCoreApplication>
#include <QJniObject>
#define AUTO_MOUNT        "/storage/"

#else
#define AUTO_MOUNT        "/media/"
#endif

PlatformExtras::PlatformExtras(QObject* parent)
: QObject(parent)
, m_preventBlanking(false)
{
#ifdef HAVE_DBUS
  // register dbus service for inhibitor
  RemoteService* inhibitor = new RemoteService("org.freedesktop.ScreenSaver",
                                               "/org/freedesktop/ScreenSaver",
                                               "org.freedesktop.ScreenSaver");
  if (inhibitor->interface.isValid())
    m_remoteServices.insert("inhibitor", inhibitor);
  else
  {
    qWarning("Failed to register DBus interface for inhibitor");
    delete inhibitor;
  }
#endif
}

PlatformExtras::~PlatformExtras()
{
  // clear inhibitor lock
  setPreventBlanking(false);
#ifdef HAVE_DBUS
  // free registered DBus services
  for (RemoteService* svc : qAsConst(m_remoteServices))
    delete svc;
  m_remoteServices.clear();
#endif
}

QString PlatformExtras::getHomeDir()
{
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject nullstr = QJniObject::fromString("");
  QJniObject file = activity.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  return path.toString();
#else
  return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
}

QString PlatformExtras::getDataDir(const char* appId)
{
#ifdef Q_OS_ANDROID
  return "assets:";
#else
  return QStandardPaths::locate(QStandardPaths::GenericDataLocation, appId, QStandardPaths::LocateDirectory);
#endif
}

QStringList PlatformExtras::getStorageDirs()
{
  QStringList dirs;
#ifdef Q_OS_ANDROID
  /*
   * WARNING: SDCARD storage cannot be properly managed since Android 10. Therefore I return only the internal storage.
   */
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject nullstr = QJniObject::fromString("");
  QJniObject file = activity.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  dirs.push_back(path.toString());
#else
  // search for a mounted sdcard
  for (const QStorageInfo& storage : QStorageInfo::mountedVolumes())
  {
    QString path = storage.rootPath();
    if (storage.isValid() && storage.isReady() && path.startsWith(AUTO_MOUNT))
    {
      qInfo("Found storage: %s", path.toUtf8().constData());
      dirs.push_back(path);
    }
  }
#endif
  return dirs;
}

void PlatformExtras::setPreventBlanking(bool on)
{
  if (m_preventBlanking == on)
    return;
  m_preventBlanking = on;
#if defined(Q_OS_ANDROID)
  {
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on]
    {
        static const int FLAG_KEEP_SCREEN_ON = QJniObject::getStaticField<jint>("android/view/WindowManager$LayoutParams", "FLAG_KEEP_SCREEN_ON");
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        auto window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        if (on)
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        else
            window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
    });
  }
#elif defined(HAVE_DBUS)
  {
    auto inhibitor = m_remoteServices.find("inhibitor");
    if (inhibitor == m_remoteServices.end())
    {
      // rollback
      m_preventBlanking = !on;
      return;
    }
    else if(on)
    {
      QDBusReply<uint> reply = inhibitor.value()->interface.call("Inhibit", "osmin", "navigation enabled");
      if (reply.isValid())
        inhibitor.value()->cookie = reply.value();
      else
      {
        qWarning("Inhibitor failed: %s", reply.error().message().toUtf8().constData());
        // rollback
        m_preventBlanking = !on;
        return;
      }
    }
    else
      inhibitor.value()->interface.call("UnInhibit", inhibitor.value()->cookie);
  }
#else
  qWarning("Inhibitor isn't implemented for this platform");
#endif
  emit preventBlanking();
}
