#pragma once

#include <QMainWindow>

namespace Ui {
class hlsplay;
}

class StreamDownloader;

class hlsplay : public QMainWindow
{
    Q_OBJECT

public:
    explicit hlsplay(QWidget *parent = 0);
    ~hlsplay();

public slots:
    void playPause();

private:
    Ui::hlsplay *ui;
    StreamDownloader * streamDownloader;
};
