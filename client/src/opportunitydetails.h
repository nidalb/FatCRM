#ifndef OPPORTUNITYDETAILS_H
#define OPPORTUNITYDETAILS_H

#include "ui_opportunitydetails.h"
#include "editcalendarbutton.h"

#include <akonadi/item.h>

#include <QtGui/QWidget>

class OpportunityDetails : public QWidget
{
    Q_OBJECT
public:
    explicit OpportunityDetails( QWidget *parent = 0 );

    ~OpportunityDetails();

    void setItem( const Akonadi::Item &item );
    void clearFields();
    void addAccountData( const QString &name,  const QString &id );
    void removeAccountData( const QString &accountName );
    void addCampaignData( const QString &name,  const QString &id );
    void removeCampaignData( const QString &campaignName );
    void addAssignedToData( const QString &name, const QString &id );
    void reset();

    inline QMap<QString, QString> opportunityData() {return mData;}

Q_SIGNALS:
    void saveOpportunity();
    void modifyOpportunity();

    friend class OpportunitiesPage;
private:
    void initialize();
    bool isEditing();

    EditCalendarButton *mCalendarButton;
    QMap<QString, QString> mData; // this
    QMap<QString, QString> mAccountsData;
    QMap<QString, QString> mAssignedToData;
    QMap<QString, QString> mCampaignsData;
    bool mModifyFlag;
    Ui_OpportunityDetails mUi;

private Q_SLOTS:
    void slotEnableSaving();
    void slotSaveOpportunity();
    void slotSetModifyFlag( bool );
    void slotSetDateClosed();
    void slotClearDate();
    void slotResetCursor( const QString& );
};


#endif /* OPPORTUNITYDETAILS_H */

