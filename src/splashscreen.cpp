#include "splashscreen.h"
#include <QThread>

SplashScreen::SplashScreen(const QPixmap &pixmap) : QSplashScreen(pixmap) {}
SplashScreen::~SplashScreen(){}

void SplashScreen::drawContents(QPainter *painter) {
    QPixmap textPix = QSplashScreen::pixmap();
    painter->setPen(this->color);
    painter->drawText(this->rect, this->alignement, this->message);
}

void SplashScreen::setMessageRect(QRect rect) {
    this->rect = rect;
}

void SplashScreen::showStatusMessage(const QString &message, int align, const QColor &color) {
    this->message = message;
    this->color = color;
    this->alignement = align;
    this->showMessage(this->message, this->alignement, this->color);
    QThread::msleep(100);
}
