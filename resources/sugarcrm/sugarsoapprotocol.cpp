/*
  This file is part of FatCRM, a desktop application for SugarCRM written by KDAB.

  Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Authors: Jeremy Entressangle <jeremy.entressangle@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sugarsoapprotocol.h"
#include "sugarjob.h"
#include "sugarsession.h"
#include "listentriesscope.h"
#include <QNetworkReply>
#include <KLocalizedString>
#include <KDebug>

SugarSoapProtocol::SugarSoapProtocol()
{
}

int SugarSoapProtocol::login(const QString &user, const QString &password, QString &sessionId, QString &errorMessage)
{
    Q_ASSERT(mSession->soap() != nullptr);

    const QByteArray passwordHash = password.toUtf8();

    KDSoapGenerated::TNS__User_auth userAuth;
    userAuth.setUser_name(user);
    userAuth.setPassword(QString::fromAscii(passwordHash));
    userAuth.setVersion(QLatin1String(".01"));

    KDSoapGenerated::TNS__Set_entry_result entry_result = mSession->soap()->login(userAuth, QLatin1String("FatCRM"));
    if (entry_result.error().number() == "0") {
        sessionId = entry_result.id();
        return KJob::NoError;
    } else {
        errorMessage = i18nc("@info:status", "Login for user %1 on %2 failed: %3", user, mSession->host(), mSession->soap()->lastError());
        int faultcode = mSession->soap()->lastErrorCode();
        if (faultcode == QNetworkReply::UnknownNetworkError ||
           faultcode == QNetworkReply::HostNotFoundError) {
            return SugarJob::CouldNotConnectError;
        } else {
            return SugarJob::LoginError;
        }
    }
}

void SugarSoapProtocol::logout()
{
    if (mSession->sessionId().isEmpty() && mSession->soap() != nullptr) {
        KDSoapGenerated::TNS__Error_value errorValue = mSession->soap()->logout(mSession->sessionId());
        if (errorValue.number() != "0") {
            kDebug() << "logout returned error" << errorValue.number() << errorValue.name() << errorValue.description();
        }
        if (!mSession->soap()->lastError().isEmpty()) {
            kDebug() << "logout had fault" << mSession->soap()->lastError();
        }
    }
    mSession->forgetSession();
}

int SugarSoapProtocol::getEntriesCount(const ListEntriesScope &scope, const QString &moduleName, const QString &query,
                                       int &entriesCount, QString &errorMessage)
{
    KDSoapGenerated::TNS__Get_entries_count_result entry_result =
            mSession->soap()->get_entries_count(mSession->sessionId(), moduleName, query, scope.deleted());
    if (entry_result.error().number() == "0") {
        entriesCount = entry_result.result_count();
        return KJob::NoError;
    } else if (entry_result.error().number() == "10"){
        errorMessage = entry_result.error().description();
        kDebug() << "getEntriesCount returned error" << entry_result.error().number() << entry_result.error().name() << entry_result.error().description();
        return SugarJob::CouldNotConnectError;
    } else {
        errorMessage = entry_result.error().description();
        kDebug() << "getEntriesCount returned error" << entry_result.error().number() << entry_result.error().name() << entry_result.error().description()
                                                           << " Soap error:" << mSession->soap()->lastError();;
        return SugarJob::SoapError;
    }
}

int SugarSoapProtocol::listEntries(const ListEntriesScope &scope, const QString &moduleName, const QString &query,
                                   const QString &orderBy, const QStringList &selectedFields, EntriesListResult &entriesListResult,
                                    QString &errorMessage)
{
    const int offset = scope.offset();
    const int maxResults = 100;
    const int fetchDeleted = scope.deleted();

    KDSoapGenerated::TNS__Select_fields Fields;
    Fields.setItems(selectedFields);

    KDSoapGenerated::TNS__Get_entry_list_result entry_result =
            mSession->soap()->get_entry_list(mSession->sessionId(), moduleName, query, orderBy, offset, Fields, maxResults, fetchDeleted);

    entriesListResult.entryList = entry_result.entry_list();
    entriesListResult.fieldList = entry_result.field_list();
    entriesListResult.nextOffset = entry_result.next_offset();
    entriesListResult.resultCount = entry_result.result_count();

    if (entry_result.error().number() == "0") {
        return KJob::NoError;
    } else if (entry_result.error().number() == "10"){
        errorMessage = entry_result.error().description();
        kDebug() << "listEntries returned error" << entry_result.error().number() << entry_result.error().name() << entry_result.error().description();
        return SugarJob::CouldNotConnectError;
    } else {
        errorMessage = entry_result.error().description();
        kDebug() << "listEntries returned error" << entry_result.error().number() << entry_result.error().name() << entry_result.error().description()
                                                           << " Soap error:" << mSession->soap()->lastError();
        return SugarJob::SoapError;
    }
}

