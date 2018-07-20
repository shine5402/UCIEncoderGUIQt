#include "qlabel_doubleclick.h"

QLabel_DoubleClick::QLabel_DoubleClick(QWidget *parent) : QLabel(parent)
{

}

QLabel_DoubleClick::~QLabel_DoubleClick()
{

}

void QLabel_DoubleClick::mouseDoubleClickEvent(QMouseEvent *)
{
    emit doubleClicked();
}
