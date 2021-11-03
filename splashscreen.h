#ifndef SPLASH_H
#define SPLASH_H

#include <QSplashScreen>
#include <QPainter>

class SplashScreen : public QSplashScreen {
public:
    SplashScreen(const QPixmap& pixmap);
    ~SplashScreen();
    virtual void drawContents(QPainter *painter);
    void setMessageRect(QRect rect);

public slots:
    void showStatusMessage(const QString &message, int align, const QColor &color = Qt::black);

public:
    QString message;
    int alignement;
    QColor color;
    QRect rect;
};

#endif // SPLASH_H
