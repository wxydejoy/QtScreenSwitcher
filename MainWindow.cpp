#include "MainWindow.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

MonitorCard::MonitorCard(const MonitorInfo& info, ScreenManager* manager, QWidget* parent)
    : QFrame(parent), m_id(info.id), m_manager(manager)
{
    setFrameShape(QFrame::StyledPanel);
    setObjectName("monitorCard");
    
    // Add shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 4);
   // setGraphicsEffect(shadow);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(20);
    
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    
    QLabel* titleLabel = new QLabel(info.name);
    titleLabel->setObjectName("cardTitle");
    
    QString statusText = info.isActive ?
        QString("%1x%2 · %3,%4")
            .arg(info.rect.width())
            .arg(info.rect.height())
            .arg(info.rect.x())
            .arg(info.rect.y()) :
        "Disconnected · Hidden From Desktop";
    QLabel* resLabel = new QLabel(statusText);
    resLabel->setObjectName("cardInfo");
    
    infoLayout->addWidget(titleLabel);
    infoLayout->addWidget(resLabel);
    
    // Desktop Toggle
    QWidget* desktopGroup = new QWidget(this);
    desktopGroup->setObjectName("controlGroup");
    QVBoxLayout* desktopLayout = new QVBoxLayout(desktopGroup);
    desktopLayout->setContentsMargins(12, 10, 12, 10);
    desktopLayout->setSpacing(6);
    QLabel* desktopLabel = new QLabel("Desktop");
    desktopLabel->setObjectName("controlLabel");
    desktopLabel->setAlignment(Qt::AlignCenter);

    QPushButton* activeBtn = new QPushButton(info.isActive ? "ON" : "OFF");
    activeBtn->setObjectName("activeBtn");
    activeBtn->setCheckable(true);
    activeBtn->setChecked(info.isActive);
    activeBtn->setFixedSize(72, 32);
    activeBtn->setCursor(Qt::PointingHandCursor);

    desktopLayout->addWidget(desktopLabel);
    desktopLayout->addWidget(activeBtn, 0, Qt::AlignCenter);

    // Backlight Toggle
    QWidget* backlightGroup = new QWidget(this);
    backlightGroup->setObjectName("controlGroup");
    QVBoxLayout* backlightLayout = new QVBoxLayout(backlightGroup);
    backlightLayout->setContentsMargins(12, 10, 12, 10);
    backlightLayout->setSpacing(6);
    QLabel* backlightLabel = new QLabel("Backlight");
    backlightLabel->setObjectName("controlLabel");
    backlightLabel->setAlignment(Qt::AlignCenter);

    QPushButton* toggleBtn = new QPushButton(info.isPowerOn ? "ON" : "OFF");
    toggleBtn->setObjectName("toggleBtn");
    toggleBtn->setCheckable(true);
    toggleBtn->setChecked(info.isPowerOn); 
    toggleBtn->setFixedSize(72, 32);
    toggleBtn->setEnabled(info.isActive);
    toggleBtn->setCursor(Qt::PointingHandCursor);

    backlightLayout->addWidget(backlightLabel);
    backlightLayout->addWidget(toggleBtn, 0, Qt::AlignCenter);

    connect(activeBtn, &QPushButton::clicked, [this, info](bool checked){
        if (m_manager->setMonitorActive(m_id, checked)) {
            emit requestRefresh();
        }
    });

    connect(toggleBtn, &QPushButton::clicked, [this, toggleBtn](bool checked){
        if (m_manager->setMonitorPower(m_id, checked)) {
            toggleBtn->setText(checked ? "ON" : "OFF");
        } else {
            // Revert on failure
            toggleBtn->setChecked(!checked);
        }
    });

    layout->addLayout(infoLayout);
    layout->addStretch();
    layout->addWidget(desktopGroup);
    layout->addWidget(backlightGroup);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_screenManager(new ScreenManager(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(480, 420);
    
    setupUi();
    refreshMonitors();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("m_centralWidget");
    m_centralWidget->setAttribute(Qt::WA_StyledBackground);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(0);
    
    // Custom Content Wrapper for Shadow
    QWidget* contentWrapper = new QWidget();
    contentWrapper->setObjectName("contentWrapper");
    contentWrapper->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    // Main Shadow for the whole window
    QGraphicsDropShadowEffect* windowShadow = new QGraphicsDropShadowEffect(this);
    windowShadow->setBlurRadius(20);
    windowShadow->setColor(QColor(0, 0, 0, 60));
    windowShadow->setOffset(0, 0);
    contentWrapper->setGraphicsEffect(windowShadow);
    
    // Title Bar
    QWidget* titleBar = new QWidget();
    titleBar->setObjectName("titleBar");
    titleBar->setAttribute(Qt::WA_StyledBackground);
    titleBar->setFixedHeight(50);
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 10, 0);
    
    QLabel* windowTitle = new QLabel("Screen Switcher");
    windowTitle->setObjectName("windowTitle");
    titleLayout->addWidget(windowTitle);
    titleLayout->addStretch();
    
    QPushButton* minBtn = new QPushButton("–");
    minBtn->setObjectName("titleBtn");
    minBtn->setFixedSize(30, 30);
    connect(minBtn, &QPushButton::clicked, this, &MainWindow::showMinimized);
    
    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedSize(30, 30);
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::hide);
    
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(closeBtn);
    
    contentLayout->addWidget(titleBar);
    
    // Body
    QWidget* body = new QWidget();
    body->setObjectName("body");
    body->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(24, 20, 24, 20);
    bodyLayout->setSpacing(20);
    

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    
    QWidget* scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    m_cardsLayout = new QVBoxLayout(scrollContent);
    m_cardsLayout->setAlignment(Qt::AlignTop);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(12);
    m_scrollArea->setWidget(scrollContent);
    
    bodyLayout->addWidget(m_scrollArea);

    QHBoxLayout* footer = new QHBoxLayout();
    footer->setContentsMargins(0, 10, 0, 0);
    
    QPushButton* refreshBtn = new QPushButton("Refresh");
    refreshBtn->setObjectName("refreshBtn");
    refreshBtn->setFixedHeight(40);
    
    QPushButton* allOnBtn = new QPushButton("All On");
    allOnBtn->setObjectName("allOnBtn");
    allOnBtn->setFixedHeight(40);
    
    QPushButton* allOffBtn = new QPushButton("All Off");
    allOffBtn->setObjectName("allOffBtn");
    allOffBtn->setFixedHeight(40);
    
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshMonitors);
    connect(allOnBtn, &QPushButton::clicked, [this](){ 
        m_screenManager->setAllMonitorsPower(true); 
        refreshMonitors();
    });
    connect(allOffBtn, &QPushButton::clicked, [this](){ 
        m_screenManager->setAllMonitorsPower(false); 
        refreshMonitors();
    });

    footer->addWidget(refreshBtn);
    footer->addStretch();
    footer->addWidget(allOnBtn);
    footer->addWidget(allOffBtn);
    
    bodyLayout->addLayout(footer);
    contentLayout->addWidget(body);
    
    mainLayout->addWidget(contentWrapper);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::refreshMonitors()
{
    m_screenManager->refresh();
    clearLayout(m_cardsLayout);
    
    auto monitors = m_screenManager->getMonitors();
    for (const auto& info : monitors) {
        MonitorCard* card = new MonitorCard(info, m_screenManager);
        connect(card, &MonitorCard::requestRefresh, this, &MainWindow::refreshMonitors);
        m_cardsLayout->addWidget(card);
    }

    updateWindowHeight();
}

void MainWindow::updateWindowHeight()
{
    const int cardCount = m_cardsLayout ? m_cardsLayout->count() : 0;
    const int cardHeight = 112;
    const int cardSpacing = 12;
    const int chromeHeight = 250;
    const int targetHeight = chromeHeight + (cardCount * cardHeight) + (qMax(0, cardCount - 1) * cardSpacing);
    const int boundedHeight = qBound(420, targetHeight, 760);

    resize(width(), boundedHeight);
}

void MainWindow::clearLayout(QLayout* layout)
{
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isVisible()) {
        hide();
        event->ignore();
    }
}
