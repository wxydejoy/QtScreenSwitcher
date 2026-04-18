#ifndef CUSTOMTOGGLE_H
#define CUSTOMTOGGLE_H

#include <QWidget>
#include <QPropertyAnimation>

class CustomToggle : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)

public:
    explicit CustomToggle(QWidget *parent = nullptr);
    
    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

    int offset() const { return m_offset; }
    void setOffset(int offset) { m_offset = offset; update(); }

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    bool m_checked = false;
    int m_offset = 2;
    QPropertyAnimation *m_animation;
    bool m_hover = false;
};

#endif // CUSTOMTOGGLE_H
