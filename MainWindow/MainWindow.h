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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
class QTreeWidgetItemModel;
class QFileInfo;
class QDir;
class QFile;
class QProgressDialog;
class QTreeWidgetItem;
namespace Ui {class CMainWindow;};
#include <unordered_map>
#include <set>

class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    CMainWindow(QWidget* parent = 0);
    ~CMainWindow();
public Q_SLOTS:
    void slotSelectSrcDirectory();
    void slotDirectoryChanged();
    void slotLoad();
    void slotEdit(QTreeWidgetItem* item, int column);

private:
    void loadSettings();
    void saveSettings();
    void loadDirectory();

    void deleteEmptyFolders();
    void deleteEmptyFolders( QTreeWidgetItem * item );
    QString getFullPath(QTreeWidgetItem* item) const;

    void loadDir(const QString & dir, QTreeWidgetItem * parent, QProgressDialog* dlg);
    void loadFilesForDir(const QFileInfo & dir, QTreeWidgetItem* parent, QProgressDialog * dlg );

    void loadItem( QTreeWidgetItem* item ) const;

    std::set< QString > getFiles(const QFileInfo& dir, QProgressDialog* dlg) const;

    std::unordered_map< QString, QTreeWidgetItem* > fDirMap;
    std::unordered_map< QString, QTreeWidgetItem* > fFileMap;

    QDir fRelToDir;
    std::unique_ptr< Ui::CMainWindow > fImpl;
};

#endif 
