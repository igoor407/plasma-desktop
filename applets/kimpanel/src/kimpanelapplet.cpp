/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#include "kimpanelapplet.h"

#include <KConfigDialog>
#include <KDesktopFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QToolButton>

#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include <math.h>

static const int s_magic_margin = 4;

KIMPanelApplet::KIMPanelApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent,args),
    m_layout(0),
    m_statusbar(0),
    m_statusbarGraphics(0),
    m_panel_agent(0)
{
    setHasConfigurationInterface(true);
//X     setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    // set our default size here
//X     resize((m_visibleIcons / m_rowCount) * s_defaultIconSize +
//X             (s_defaultSpacing * (m_visibleIcons + 1)),
//X            m_rowCount * 22 + s_defaultSpacing * 3);
}

KIMPanelApplet::~KIMPanelApplet()
{
}

void KIMPanelApplet::saveState(KConfigGroup &config) const
{
    Q_UNUSED(config);
}

void KIMPanelApplet::init()
{
    setBackgroundHints(Plasma::Applet::DefaultBackground);

    KConfigGroup cg = config();

    m_largestIconWidth = qMax((int)KIconLoader::SizeSmall,
        cg.readEntry("LargestIconWidth", (int)KIconLoader::SizeMedium));

    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/systemtray");

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(4,4,4,4);

    m_panel_agent = new PanelAgent(this);

    // Initialize widget which holds all im properties.
    m_statusbarGraphics = new KIMStatusBarGraphics(m_panel_agent);
    m_statusbarGraphics->setContentsMargins(0,0,0,0);

    m_statusbarGraphics->setCollapsible(true);
    m_statusbar = new KIMStatusBar();
    
   connect(m_statusbarGraphics,SIGNAL(iconCountChanged()),SLOT(adjustSelf()));
   connect(m_statusbarGraphics,SIGNAL(collapsed(bool)),SLOT(toggleCollapse(bool)));

    m_lookup_table = new KIMLookupTable(m_panel_agent);

    m_logoIcon = new Plasma::IconWidget(KIcon("draw-freehand"),"",this);
    m_logoIcon->hide();
    
    //m_layout->addItem(m_logoIcon);
    m_layout->addItem(m_statusbarGraphics);

    themeUpdated();

}

/*
QSizeF KIMPanelApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{

    QSizeF sizeHint = size();
    if (!m_layout) {
        return sizeHint;
    }
    sizeHint.setWidth(left + right + m_layout->effectiveSizeHint(which).width());
    sizeHint.setHeight(top + bottom + m_layout->effectiveSizeHint(which).height());
    kDebug() << sizeHint;
    return sizeHint;
}
*/

void KIMPanelApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::FormFactorConstraint) ||
        (constraints & Plasma::SizeConstraint)) {

        adjustSelf();
        //resize(preferredSize());
    }
}

void KIMPanelApplet::createConfigurationInterface(KConfigDialog *parent)
{
}

void KIMPanelApplet::configAccepted()
{
#if 0
    bool changed = false;
    int temp = m_uiConfig.icons->value();

    KConfigGroup cg = config();

    if (temp != m_largestIconWidth) {
        m_largestIconWidth = temp;
        cg.writeEntry("LargestIconWidth", m_largestIconWidth);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        adjustSelf();
    }
#endif
}

QList<QAction*> KIMPanelApplet::contextualActions()
{
    return m_statusbarGraphics->actions();
}

void KIMPanelApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

//    QRect r = rect().toRect();
//    m_background->setElementPrefix("lastelements");

    painter->save();

    //m_background->setElementPrefix(QString());
    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect, contentsRect);

    painter->restore();
}

void KIMPanelApplet::adjustSelf()
{
    int iconCount; 
    //int iconWidth; 
    int i,j;
    QSizeF sizeHint = m_layout->contentsRect().size();
    QSizeF r = sizeHint;
    //r.setHeight(r.height()-s_magic_margin*2);
    //r.setWidth(r.width()-s_magic_margin*2);

    if (m_statusbar->graphicsWidget() == 0) {
        //r = contentsRect();
        iconCount = m_statusbarGraphics->iconCount();
    } else {
        //r = contentsRect();
        iconCount = 1;
    }

    switch (formFactor()) {
    case Plasma::Horizontal:
        i = 1;
        while (r.height()/i > m_largestIconWidth)
            i++;
        j = (iconCount + (i - 1)) / i;
        sizeHint = QSizeF(j*r.height()/i, r.height());
        break;
    case Plasma::Vertical:
        i = 1;
        while (r.width()/i > m_largestIconWidth)
            i++;
        j = (iconCount + (i - 1)) / i;
        sizeHint = QSizeF(r.width(),j*r.width()/i);
        break;
    case Plasma::Planar:
    case Plasma::MediaCenter:
        sizeHint = QSizeF(iconCount*(qreal)KIconLoader::SizeMedium,(qreal)KIconLoader::SizeMedium);
        break;
    }
    
    qreal left, top, right, bottom;
    m_layout->getContentsMargins(&left,&top,&right,&bottom);
    kDebug() << left << top << sizeHint;
    sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);

    if (m_statusbar->graphicsWidget() == 0) {
        m_statusbarGraphics->getContentsMargins(&left,&top,&right,&bottom);
        sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);
    }

    setPreferredSize(sizeHint);
}

void KIMPanelApplet::toggleCollapse(bool b)
{
    if (b) {
        m_layout->removeItem(m_statusbarGraphics);
        m_layout->addItem(m_logoIcon);
        //m_statusbarGraphics->showLogo(true);
        m_statusbar->setGraphicsWidget(m_statusbarGraphics);
        //disconnect(m_statusbarGraphics,SIGNAL(iconCountChanged()),this,SLOT(adjustSelf()));
    } else {
        m_statusbar->setGraphicsWidget(0);
        //m_statusbarGraphics->showLogo(false);
        m_layout->addItem(m_statusbarGraphics);
        m_layout->removeItem(m_logoIcon);
        //connect(m_statusbarGraphics,SIGNAL(iconCountChanged()),SLOT(adjustSelf()));
    }
    m_statusbar->setVisible(b);
    m_logoIcon->setVisible(b);
    adjustSelf();
}

void KIMPanelApplet::themeUpdated()
{
    kDebug()<<"Update Theme"<<Plasma::Theme::defaultTheme()->themeName();
#if 0
    qreal left;
    qreal right;
    qreal top;
    qreal bottom;

    getContentsMargins(&left,&top,&right,&bottom);
    left = qMax(left,4.);
    right = qMax(right,2.);
    top = qMax(top,4.);
    bottom = qMax(bottom,4.);

    kDebug() << contentsRect();
    setContentsMargins(left, top, right, bottom);
    kDebug() << contentsRect();
#endif
}

K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanelApplet)

#include "kimpanelapplet.moc"
