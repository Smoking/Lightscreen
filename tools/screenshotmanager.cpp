/*
 * Copyright (C) 2011  Christian Kaiser
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "screenshotmanager.h"
#include "screenshot.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QSettings>

ScreenshotManager::ScreenshotManager(QObject *parent = 0) : QObject(parent), mCount(0)
{
  if (QFile::exists(qApp->applicationDirPath() + "/config.ini")) {
    mSettings     = new QSettings(qApp->applicationDirPath() + QDir::separator() + "config.ini", QSettings::IniFormat);
    mHistoryPath  = qApp->applicationDirPath() + QDir::separator() + "history";
    mPortableMode = true;
  }
  else {
    mSettings     = new QSettings();
    mHistoryPath  = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "history";
    mPortableMode = false;
  }
}

ScreenshotManager::~ScreenshotManager()
{
  delete mSettings;
}

QString& ScreenshotManager::historyPath()
{
  return mHistoryPath;
}

bool ScreenshotManager::portableMode()
{
  return mPortableMode;
}

void ScreenshotManager::saveHistory(QString fileName, QString url)
{
  if (!mSettings->value("/options/history", true).toBool())
    return;

  QFile historyFile(mHistoryPath);
  QTextStream out(&historyFile);

  if (!historyFile.exists())
  {
    QString path = mHistoryPath;
    path.chop(7);

    if (!QDir().mkpath(path))
      return;
  }

  if (historyFile.open(QFile::WriteOnly | QFile::Append)) {
    out << QString("%1|%2|%3").arg(fileName).arg(url).arg(QDateTime::currentMSecsSinceEpoch()) << "\n";
  }

  historyFile.close();
}

//

void ScreenshotManager::askConfirmation()
{
  Screenshot* s = qobject_cast<Screenshot*>(sender());
  emit confirm(s);
}

void ScreenshotManager::cleanup()
{
  Screenshot* s = qobject_cast<Screenshot*>(sender());
  emit windowCleanup(s->options());
  s->deleteLater();
}

void ScreenshotManager::take(Screenshot::Options &options)
{
  Screenshot* newScreenshot = new Screenshot(this, options);

  connect(newScreenshot, SIGNAL(askConfirmation()), this, SLOT(askConfirmation()));
  connect(newScreenshot, SIGNAL(finished())       , this, SLOT(cleanup()));

  newScreenshot->take();
}

// Singleton
ScreenshotManager* ScreenshotManager::mInstance = 0;

ScreenshotManager *ScreenshotManager::instance()
{
  if (!mInstance)
    mInstance = new ScreenshotManager();

  return mInstance;
}
