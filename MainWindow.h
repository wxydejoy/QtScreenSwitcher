#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include "ScreenManager.h"

class MonitorCard : public QFrame {
    Q_OBJECT
public:
    MonitorCard(const MonitorInfo& info, ScreenManager* manager, QWidget* parent = nullptr);

signals:
    void requestRefresh();

private:
    int m_id;
    ScreenManager* m_manager;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void refreshMonitors();

private:
    void setupUi();
    void clearLayout(QLayout* layout);
    void updateWindowHeight();

    ScreenManager* m_screenManager;
    QVBoxLayout* m_cardsLayout;
    QWidget* m_centralWidget;
    QScrollArea* m_scrollArea;
    
    QPoint m_dragPosition;
};

#endif // MAINWINDOW_H
