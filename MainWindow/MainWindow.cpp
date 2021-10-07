// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "MainWindow.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/FileUtils.h"
#include "TimeStamp.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QDate>
#include <QProgressDialog>
#include <map>
#include <set>

CMainWindow::CMainWindow(QWidget* parent)
    : QMainWindow(parent),
    fImpl(new Ui::CMainWindow)
{
    fImpl->setupUi(this);
    connect(fImpl->srcDir, &QLineEdit::textChanged, this, &CMainWindow::slotDirectoryChanged);
    connect(fImpl->btnSelectSrcDir, &QPushButton::clicked, this, &CMainWindow::slotSelectSrcDirectory);
    connect(fImpl->btnLoad, &QPushButton::clicked, this, &CMainWindow::slotLoad);
    connect(fImpl->files, &QTreeWidget::itemDoubleClicked, this, &CMainWindow::slotEdit);

    auto completer = new QCompleter(this);
    auto fsModel = new QFileSystemModel(completer);
    fsModel->setRootPath("");
    completer->setModel(fsModel);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    fImpl->srcDir->setCompleter(completer);

    loadSettings();
}

CMainWindow::~CMainWindow()
{
    saveSettings();
}

void CMainWindow::loadSettings()
{
    QSettings settings;

    fImpl->srcDir->setText(settings.value("SrcDir", QString()).toString());

    slotDirectoryChanged();
}

void CMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue("SrcDir", fImpl->srcDir->text());
}

void CMainWindow::slotDirectoryChanged()
{
    QFileInfo dir1(fImpl->srcDir->text());
    fImpl->btnLoad->setEnabled(!fImpl->srcDir->text().isEmpty() && dir1.exists() && dir1.isDir());
}

void CMainWindow::slotSelectSrcDirectory()
{
    auto dir = QFileDialog::getExistingDirectory(this, tr("Select Source Directory:"), fImpl->srcDir->text());
    if (!dir.isEmpty())
        fImpl->srcDir->setText(dir);
}

void CMainWindow::slotEdit(QTreeWidgetItem* item, int /*column*/)
{
    if (item->type() != 2)
        return;

    CTimeStamp dlg( getFullPath( item ), this);
    dlg.exec();
    loadItem(item);
}
void CMainWindow::slotLoad()
{
    loadDirectory();
}

void CMainWindow::loadDirectory()
{
    fDirMap.clear();
    fFileMap.clear();
    fImpl->files->clear();
    fImpl->files->setHeaderLabels(QStringList() << "Name" << "Created" << "Accessed" << "MetaData Modified" << "Modified" );

    std::map< QString, QString > knownExtensions;
    std::set< QString > ignoredExtensions;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QProgressDialog dlg(tr("Finding Files..."), "Cancel", 0, 0, this);
    dlg.setMinimumDuration(0);
    dlg.setValue(1);

    fRelToDir = QDir(fImpl->srcDir->text());

    auto rootDir = new QTreeWidgetItem(fImpl->files, QStringList() << ".", 1);
    fDirMap["."] = rootDir;

    loadFilesForDir( QFileInfo( fImpl->srcDir->text() ), rootDir, &dlg);
    rootDir->setExpanded( true );

    loadDir(fImpl->srcDir->text(), rootDir, &dlg);

    deleteEmptyFolders();

    QApplication::restoreOverrideCursor();
}

void CMainWindow::deleteEmptyFolders()
{
    for (int ii = fImpl->files->topLevelItemCount()-1; ii >= 0; --ii)
    {
        auto item = fImpl->files->topLevelItem(ii);
        deleteEmptyFolders(item);
    }
}

void CMainWindow::deleteEmptyFolders( QTreeWidgetItem * item )
{
    qApp->processEvents();
    if (!item)
        return;

    for( int ii = item->childCount() - 1; ii >= 0; --ii )
    {
        auto child = item->child(ii);
        deleteEmptyFolders(child);
    }

    if ((item->type() == 1) && item->childCount() == 0)
    {
        delete item;
        return;
    }
}

void CMainWindow::loadDir(const QString& dir, QTreeWidgetItem* parent, QProgressDialog* dlg )
{
    //auto extFilter = fImpl->sourceExts->text().split(";");
    QDirIterator ii(dir, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::NoSymLinks );

    int cnt = 0;
    while (ii.hasNext())
    {
        if ((cnt % 50) == 0)
            qApp->processEvents();

        cnt++;
        ii.next();

        if (dlg->wasCanceled())
            break;

        auto srcFI = ii.fileInfo();
        auto srcPath = srcFI.absoluteFilePath();

        auto addOnPath = fRelToDir.relativeFilePath(srcPath);

        QStringList strings;
        strings << addOnPath << QString() << QString() << QString();

        QTreeWidgetItem* dirItem = nullptr;
        auto pos = fDirMap.find(addOnPath);
        if (pos == fDirMap.end())
        {
            if (parent)
                dirItem = new QTreeWidgetItem(parent, strings, 1);
            else
                dirItem = new QTreeWidgetItem(fImpl->files, strings, 1);
        }
        else
            dirItem = (*pos).second;
        fDirMap[addOnPath] = dirItem;

        loadFilesForDir(srcFI, dirItem, dlg );
        dirItem->setExpanded(true);

        loadDir(srcPath, dirItem, dlg );
    }
}

void CMainWindow::loadFilesForDir(const QFileInfo& dir, QTreeWidgetItem * dirItem, QProgressDialog* dlg)
{
    auto files = getFiles(dir, dlg);


    int cnt = 0;
    for( auto && fileName : files )
    {
        if ((cnt % 10) == 0)
            qApp->processEvents();

        cnt++;
        if (dlg->wasCanceled())
            break;

        auto lhsFullPath = QDir( dir.absoluteFilePath() ).absoluteFilePath(fileName);
        auto key = fRelToDir.relativeFilePath(lhsFullPath);
        if (fFileMap.find(key) != fFileMap.end())
            continue;

        auto curr = new QTreeWidgetItem(dirItem, QStringList() << fileName << QString() << QString() << QString(), 2);
        loadItem( curr );

        curr->setExpanded(true);
        fFileMap[key] = curr;
    }
}

void CMainWindow::loadItem(QTreeWidgetItem* item) const
{
    auto fileName = getFullPath(item);
    auto fileInfo = QFileInfo(fileName);
    auto createdTime = fileInfo.fileTime(QFile::FileBirthTime);
    auto accessTime = fileInfo.fileTime(QFile::FileAccessTime);
    auto metaTime = fileInfo.fileTime(QFile::FileMetadataChangeTime);
    auto modTime = fileInfo.fileTime(QFile::FileModificationTime);

    item->setText(1, createdTime.toString());
    item->setText(2, accessTime.toString());
    item->setText(3, metaTime.toString());
    item->setText(4, modTime.toString());
}

std::set< QString > CMainWindow::getFiles( const QFileInfo & dir, QProgressDialog* dlg ) const
{
    std::set< QString > retVal;
    QDirIterator ii(dir.absoluteFilePath(), QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::NoSymLinks );

    int cnt = 0;
    while (ii.hasNext())
    {
        ii.next();

        if ((cnt % 10) == 0)
            qApp->processEvents();

        cnt++;
        if (dlg->wasCanceled())
            break;

        auto fileName = ii.fileInfo().fileName();
        retVal.insert(fileName);
    }
    return retVal;
}

QString CMainWindow::getFullPath( QTreeWidgetItem * item ) const
{
    if (!item)
        return QString();
    if (item->type() == 2)
    {
        auto parent = item->parent();
        auto dir = getFullPath(parent);
        if (dir.isEmpty())
            return QString();
        return QDir(dir).absoluteFilePath(item->text(0));
    }
    else if (item->type() == 1)
    {
        return  QDir(fImpl->srcDir->text()).absoluteFilePath(item->text(0));
    }
    else
        return QString();
}
//void CMainWindow::transformItem(QTreeWidgetItem* item, QProgressDialog * dlg)
//{
//    if (!item)
//        return;
//
//    if (item->type() == 2)
//    {
//        auto parent = item->parent();
//        if (!parent)
//            return;
//        if (parent->type() != 1)
//            return;
//        auto dir = parent->text(0);
//        auto relToPath = dir + "/" + item->text(0);
//
//        if (relToPath.endsWith("."))
//            relToPath = relToPath.left(relToPath.length() - 1);
//
//        auto lhsPath = QDir(fImpl->srcDir->text()).absoluteFilePath(relToPath);
//        auto rhsPath = QDir(fImpl->srcDir->text()).absoluteFilePath(relToPath);
//
//        auto lhsTS = NFileUtils::oldestTimeStamp(lhsPath);
//        auto rhsTS = NFileUtils::oldestTimeStamp(rhsPath);
//        auto tsToUse = QDateTime();
//        if (lhsTS.isValid() && rhsTS.isValid())
//            tsToUse = std::min(lhsTS, rhsTS);
//        else if (lhsTS.isValid())
//            tsToUse = lhsTS;
//        else if (rhsTS.isValid())
//            tsToUse = rhsTS;
//
//        if (tsToUse.isValid())
//        {
//            if (QFileInfo(lhsPath).exists())
//            {
//                bool aOK = NFileUtils::setTimeStamp(lhsPath, tsToUse, true);
//                if (!aOK)
//                    int xyz = 0;
//                item->setText(1, tsToUse.toString());
//            }
//            if (QFileInfo(rhsPath).exists())
//            {
//                bool aOK = NFileUtils::setTimeStamp(rhsPath, tsToUse, true);
//                if (!aOK)
//                    int xyz = 0;
//                item->setText(2, tsToUse.toString());
//            }
//        }
//        dlg->setValue(dlg->value() + 1);
//        if ((dlg->value() % 10) == 0)
//            qApp->processEvents();
//    }
//
//    for( int ii = 0; ii < item->childCount(); ++ii )
//    {
//        auto curr = item->child(ii);
//        transformItem(curr, dlg);
//        if (dlg->wasCanceled())
//            break;
//        qApp->processEvents();
//    }
//}
