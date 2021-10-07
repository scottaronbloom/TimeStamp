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

#include "TimeStamp.h"
#include "SABUtils/FileUtils.h"
#include "ui_TimeStamp.h"

#include <QPushButton>
#include <QMessageBox>

CTimeStamp::CTimeStamp( const QString & fileName, QWidget* parent)
    : QDialog(parent),
    fImpl(new Ui::CTimeStamp)
{
    fImpl->setupUi(this);
    setFileName(fileName);

    connect(fImpl->creationTime, &QDateTimeEdit::dateTimeChanged, this, &CTimeStamp::slotChanged);
    connect(fImpl->accessTime, &QDateTimeEdit::dateTimeChanged, this, &CTimeStamp::slotChanged);
    connect(fImpl->metaModTime, &QDateTimeEdit::dateTimeChanged, this, &CTimeStamp::slotChanged);
    connect(fImpl->modTime, &QDateTimeEdit::dateTimeChanged, this, &CTimeStamp::slotChanged);

    fImpl->buttonBox->button(QDialogButtonBox::Open)->setText(tr("Set all to Oldest") );
    fImpl->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    fImpl->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    fImpl->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(false);
    connect(fImpl->buttonBox, &QDialogButtonBox::clicked, this, &CTimeStamp::slotClicked);
}

void CTimeStamp::setFileName( const QString & fileName )
{
    fImpl->fileName->setText(fileName);
    loadTimeStamps();
}

void CTimeStamp::loadTimeStamps()
{
    auto fileInfo = QFileInfo( fImpl->fileName->text() );
    auto creationTime = fileInfo.fileTime(QFile::FileBirthTime);
    auto accessTime = fileInfo.fileTime(QFile::FileAccessTime);
    auto metaModTime = fileInfo.fileTime(QFile::FileMetadataChangeTime);
    auto modTime = fileInfo.fileTime(QFile::FileModificationTime);

    fImpl->creationTime->setDateTime(creationTime);
    fImpl->accessTime->setDateTime(accessTime);
    fImpl->metaModTime->setDateTime(metaModTime);
    fImpl->modTime->setDateTime(modTime);
}

CTimeStamp::~CTimeStamp()
{
}

void CTimeStamp::accept()
{
    saveTimeStamps();
    QDialog::accept();
}

void CTimeStamp::saveTimeStamps()
{
    QString msg;
    bool aOK = NFileUtils::setTimeStamp(fImpl->fileName->text(), fImpl->creationTime->dateTime(), QFileDevice::FileBirthTime, &msg );
    if ( !aOK )
    {
        QMessageBox::critical(this, "Could not set time", QString("Error setting <b>Creation Time</b>: %1. This is not supported on all Operating Systems.").arg(msg));
    }

    aOK = NFileUtils::setTimeStamp(fImpl->fileName->text(), fImpl->accessTime->dateTime(), QFileDevice::FileAccessTime, &msg);
    if (!aOK)
    {
        QMessageBox::critical(this, "Could not set time", QString("Error setting <b>Access Time</b>: %1").arg(msg));
    }

    aOK = NFileUtils::setTimeStamp(fImpl->fileName->text(), fImpl->metaModTime->dateTime(), QFileDevice::FileMetadataChangeTime, &msg);
    if (!aOK)
    {
        QMessageBox::critical(this, "Could not set time", QString("Error setting <b>Metadata Change Time</b>: %1. This is not supported on all Operating Systems.").arg(msg));
    }

    aOK = NFileUtils::setTimeStamp(fImpl->fileName->text(), fImpl->modTime->dateTime(), QFileDevice::FileModificationTime, &msg);
    if (!aOK)
    {
        QMessageBox::critical(this, "Could not set time", QString("Error setting <b>Modification Time</b>: %1").arg(msg));
    }

    fImpl->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    fImpl->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    fImpl->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(false);
    
}

void CTimeStamp::slotClicked(QAbstractButton* btn)
{
    if (btn == fImpl->buttonBox->button(QDialogButtonBox::Apply))
    {
        saveTimeStamps();
        loadTimeStamps();
    }
    else if (btn == fImpl->buttonBox->button(QDialogButtonBox::Ok))
    {
        accept();
    }
    else if (btn == fImpl->buttonBox->button(QDialogButtonBox::Discard))
    {
        loadTimeStamps();
    }
    else if (btn == fImpl->buttonBox->button(QDialogButtonBox::Open)) // set to oldest
    {
        auto creationTime = fImpl->creationTime->dateTime();
        auto accessTime = fImpl->accessTime->dateTime();
        auto metaModTime = fImpl->metaModTime->dateTime();
        auto modTime = fImpl->modTime->dateTime();

        auto newTime = std::min(creationTime, accessTime);
        newTime = std::min(newTime, metaModTime);
        newTime = std::min(newTime, modTime);

        fImpl->creationTime->setDateTime(newTime);
        fImpl->accessTime->setDateTime(newTime);
        fImpl->metaModTime->setDateTime(newTime);
        fImpl->modTime->setDateTime(newTime);
    }
}

void CTimeStamp::slotSave()
{
    saveTimeStamps();
    loadTimeStamps();
}

void CTimeStamp::slotChanged()
{
    fImpl->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    fImpl->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);

    fImpl->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(true);
    fImpl->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
}
