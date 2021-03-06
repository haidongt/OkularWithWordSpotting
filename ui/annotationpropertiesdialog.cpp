/***************************************************************************
 *   Copyright (C) 2006 by Chu Xiaodong <xiaodongchu@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "annotationpropertiesdialog.h"

// qt/kde includes
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qheaderview.h>
#include <qtextedit.h>
#include <kcolorbutton.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kglobal.h>

// local includes
#include "core/document.h"
#include "core/page.h"
#include "core/annotations.h"
#include "annotationwidgets.h"


AnnotsPropertiesDialog::AnnotsPropertiesDialog( QWidget *parent, Okular::Document *document, int docpage, Okular::Annotation *ann )
    : KPageDialog( parent ), m_document( document ), m_page( docpage ), modified( false )
{
    setFaceType( Tabbed );
    m_annot=ann;
    const bool canEditAnnotations = m_document->canModifyPageAnnotation( ann );
    setCaptionTextbyAnnotType();
    if ( canEditAnnotations )
    {
        setButtons( Ok | Apply | Cancel );
        enableButton( Apply, false );
        connect( this, SIGNAL(applyClicked()), this, SLOT(slotapply()) );
        connect( this, SIGNAL(okClicked()), this, SLOT(slotapply()) );
    }
    else
    {
        setButtons( Close );
        setDefaultButton( Close );
    }

    m_annotWidget = AnnotationWidgetFactory::widgetFor( ann );

    QLabel* tmplabel;
  //1. Appearance
    //BEGIN tab1
    QFrame *page = new QFrame( this );
    addPage( page, i18n( "&Appearance" ) );
    QGridLayout * gridlayout = new QGridLayout( page );

    tmplabel = new QLabel( i18n( "&Color:" ), page );
    gridlayout->addWidget( tmplabel, 0, 0, Qt::AlignRight );
    colorBn = new KColorButton( page );
    colorBn->setColor( ann->style().color() );
    colorBn->setEnabled( canEditAnnotations );
    tmplabel->setBuddy( colorBn );
    gridlayout->addWidget( colorBn, 0, 1 );

    tmplabel = new QLabel( i18n( "&Opacity:" ), page );
    gridlayout->addWidget( tmplabel, 1, 0, Qt::AlignRight );
    m_opacity = new KIntNumInput( page );
    m_opacity->setRange( 0, 100 );
    m_opacity->setValue( (int)( ann->style().opacity() * 100 ) );
    m_opacity->setSuffix( i18nc( "Suffix for the opacity level, eg '80 %'", " %" ) );
    m_opacity->setEnabled( canEditAnnotations );
    tmplabel->setBuddy( m_opacity );
    gridlayout->addWidget( m_opacity, 1, 1 );

    QWidget * configWidget = 0;
    if ( m_annotWidget && ( configWidget = m_annotWidget->styleWidget() ) )
    {
        gridlayout->addWidget( configWidget, 2, 0, 1, 2 );
        configWidget->setEnabled( canEditAnnotations );
    }

    gridlayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding ), 3, 0 );
    //END tab1

    //BEGIN tab 2
    page = new QFrame( this );
    addPage( page, i18n( "&General" ) );
//    m_tabitem[1]->setIcon( KIcon( "fonts" ) );
    gridlayout = new QGridLayout( page );
    tmplabel = new QLabel( i18n( "&Author:" ), page );
    AuthorEdit = new KLineEdit( ann->author(), page );
    AuthorEdit->setEnabled( canEditAnnotations );
    tmplabel->setBuddy( AuthorEdit );
    gridlayout->addWidget( tmplabel, 0, 0, Qt::AlignRight );
    gridlayout->addWidget( AuthorEdit, 0, 1 );

    tmplabel = new QLabel( page );
    tmplabel->setText( i18n( "Created: %1", KGlobal::locale()->formatDateTime( ann->creationDate(), KLocale::LongDate, true ) ) );
    tmplabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
    gridlayout->addWidget( tmplabel, 1, 0, 1, 2 );

    m_modifyDateLabel = new QLabel( page );
    m_modifyDateLabel->setText( i18n( "Modified: %1", KGlobal::locale()->formatDateTime( ann->modificationDate(), KLocale::LongDate, true ) ) );
    m_modifyDateLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
    gridlayout->addWidget( m_modifyDateLabel, 2, 0, 1, 2 );

    gridlayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding ), 3, 0 );
    //END tab 2

    QWidget * extraWidget = 0;
    if ( m_annotWidget && ( extraWidget = m_annotWidget->extraWidget() ) )
    {
        addPage( extraWidget, extraWidget->windowTitle() );
    }

    //BEGIN connections
    connect( colorBn, SIGNAL(changed(QColor)), this, SLOT(setModified()) );
    connect( m_opacity, SIGNAL(valueChanged(int)), this, SLOT(setModified()) );
    connect( AuthorEdit, SIGNAL(textChanged(QString)), this, SLOT(setModified()) );
    if ( m_annotWidget )
    {
        connect( m_annotWidget, SIGNAL(dataChanged()), this, SLOT(setModified()) );
    }
    //END

#if 0
    kDebug() << "Annotation details:";
    kDebug().nospace() << " => unique name: '" << ann->uniqueName() << "'";
    kDebug() << " => flags:" << QString::number( m_annot->flags(), 2 );
#endif

    resize( sizeHint() );
}
AnnotsPropertiesDialog::~AnnotsPropertiesDialog()
{
    delete m_annotWidget;
}


void AnnotsPropertiesDialog::setCaptionTextbyAnnotType()
{
    Okular::Annotation::SubType type=m_annot->subType();
    QString captiontext;
    switch(type)
    {
        case Okular::Annotation::AText:
            if(((Okular::TextAnnotation*)m_annot)->textType()==Okular::TextAnnotation::Linked)
                captiontext = i18n( "Note Properties" );
            else
                captiontext = i18n( "Inline Note Properties" );
            break;
        case Okular::Annotation::ALine:
            captiontext = i18n( "Line Properties" );
            break;
        case Okular::Annotation::AGeom:
            captiontext = i18n( "Geometry Properties" );
            break;
        case Okular::Annotation::AHighlight:
            captiontext = i18n( "Highlight Properties" );
            break;
        case Okular::Annotation::AStamp:
            captiontext = i18n( "Stamp Properties" );
            break;
        case Okular::Annotation::AInk:
            captiontext = i18n( "Ink Properties" );
            break;
        case Okular::Annotation::ACaret:
            captiontext = i18n( "Caret Properties" );
            break;
        case Okular::Annotation::AFileAttachment:
            captiontext = i18n( "File Attachment Properties" );
            break;
        case Okular::Annotation::ASound:
            captiontext = i18n( "Sound Properties" );
            break;
        case Okular::Annotation::AMovie:
            captiontext = i18n( "Movie Properties" );
            break;
        default:
            captiontext = i18n( "Annotation Properties" );
            break;
    }
        setCaption( captiontext );
}

void AnnotsPropertiesDialog::setModified()
{
    modified = true;
    enableButton( Apply, true );
}

void AnnotsPropertiesDialog::slotapply()
{
    if ( !modified )
        return;

    m_document->prepareToModifyAnnotationProperties( m_annot );
    m_annot->setAuthor( AuthorEdit->text() );
    m_annot->setModificationDate( QDateTime::currentDateTime() );
    m_annot->style().setColor( colorBn->color() );
    m_annot->style().setOpacity( (double)m_opacity->value() / 100.0 );

    if ( m_annotWidget )
        m_annotWidget->applyChanges();

    m_document->modifyPageAnnotationProperties( m_page, m_annot );

    m_modifyDateLabel->setText( i18n( "Modified: %1", KGlobal::locale()->formatDateTime( m_annot->modificationDate(), KLocale::LongDate, true ) ) );

    modified = false;
    enableButton( Apply, false );
}

#include "annotationpropertiesdialog.moc"

