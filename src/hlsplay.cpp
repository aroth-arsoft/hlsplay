#include "hlsplay.h"
#include "ui_hlsplay.h"
#include "stream.h"

hlsplay::hlsplay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::hlsplay),
    streamDownloader(new StreamDownloader(this))
{
    ui->setupUi(this);
}

hlsplay::~hlsplay()
{
    delete ui;
    delete streamDownloader;
}

void hlsplay::playPause()
{
    if(!streamDownloader->isPlaying())
    {
        streamDownloader->setUrl(QUrl::fromUserInput(ui->url->text()));
        streamDownloader->play();
        ui->url->setEnabled(false);
    }
    else
    {
        streamDownloader->stop();
        ui->url->setEnabled(true);
    }
}
