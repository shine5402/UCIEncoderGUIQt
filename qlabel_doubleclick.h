#ifndef QLABEL_DOUBLECLICK_H
#define QLABEL_DOUBLECLICK_H

#include <QObject>
#include <QLabel>

class QLabel_DoubleClick : public QLabel
{
    Q_OBJECT
public:
    explicit QLabel_DoubleClick(QWidget *parent = nullptr);
    ~QLabel_DoubleClick();
protected:
    void mouseDoubleClickEvent(QMouseEvent *);
signals:
    void doubleClicked();
public slots:
};

#endif // QLABEL_DOUBLECLICK_H
