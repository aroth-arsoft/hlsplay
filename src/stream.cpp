#include "stream.h"
#include "stream_p.h"
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QMediaPlaylist>

#include <QDebug>

// in one source file
Q_LOGGING_CATEGORY(stream, "stream", QtDebugMsg)

StreamDownloader::Playlist::Playlist(QObject * parent)
    : QMediaPlaylist(parent)
    , timer(new QTimer)
    , pending(new QMediaPlaylist)
{
    timer->setSingleShot(true);
    connect(this, &Playlist::triggerReload, this, &Playlist::reloadImpl, Qt::QueuedConnection);
    connect(pending, &QMediaPlaylist::loaded, this, &Playlist::loadFinished);
    connect(timer, &QTimer::timeout, this, &Playlist::reloadImpl);
}

StreamDownloader::Playlist::~Playlist()
{
    delete pending;
}

void StreamDownloader::Playlist::load(const QNetworkRequest &request, const char *format)
{
    lastRequest = request;
    pending->clear();
    pending->load(request, format);
}

void StreamDownloader::Playlist::reload()
{
    emit triggerReload();
}

void StreamDownloader::Playlist::reloadImpl()
{
    pending->load(lastRequest);
}

void StreamDownloader::Playlist::loadFinished()
{
    bool hasBeenUpdated = false;
    for(int pendingIndex = 0; pendingIndex < pending->mediaCount(); ++pendingIndex)
    {
        QMediaContent pendingMedia = pending->media(pendingIndex);
        bool found = false;
        for(int index = 0; !found && index < mediaCount(); ++index)
        {
            QMediaContent curmedia = media(index);
            found = (curmedia == pendingMedia);
        }
        if(!found)
        {
            qWarning() << "add" << pendingMedia.canonicalUrl();
            addMedia(pendingMedia);
            hasBeenUpdated = true;
        }
    }
    if(hasBeenUpdated)
    {
        emit updated();
    }
    else
    {
        timer->start(1000);
    }
}

void StreamDownloader::Playlist::reloadTimer()
{
}

Stream::Stream(QObject * parent)
    : QObject(parent)
{
}

StreamDownloader::StreamDownloaderPrivate::StreamDownloaderPrivate()
    : QObject()
    , backgroundThread(nullptr)
    , playlist(nullptr)
    , partIndex(-1)
{
    connect(&manager, &QNetworkAccessManager::finished, this, &StreamDownloaderPrivate::partFinished);
}

StreamDownloader::StreamDownloaderPrivate::~StreamDownloaderPrivate()
{
    delete backgroundThread;
}

void StreamDownloader::StreamDownloaderPrivate::downloadPlaylist()
{
    if(!playlist)
    {
        playlist = new Playlist(this);
        connect(playlist, &Playlist::updated, this, &StreamDownloaderPrivate::playlistUpdated);
    }
    QNetworkRequest request(baseUrl);
    playlist->load(request);
}

void StreamDownloader::StreamDownloaderPrivate::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

#ifndef QT_NO_SSL
    connect(reply, &QNetworkReply::sslErrors, this, &StreamDownloaderPrivate::sslErrors);
#endif

    currentDownloads.append(reply);
}

#ifndef QT_NO_SSL
void StreamDownloader::StreamDownloaderPrivate::sslErrors(const QList<QSslError> & errors)
{
    for(const QSslError &error : errors)
    {
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
    }
}
#endif

void StreamDownloader::StreamDownloaderPrivate::partFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error())
    {
        qWarning() << "Download" << url << "failed:" << reply->errorString();
    }
    else
    {
        QByteArray data = reply->readAll();
        qWarning() << "Downloaded" << url << "ok" << data.size();
        outfile.write(data);
        downloadNextPart();
    }
    reply->deleteLater();
}

void StreamDownloader::StreamDownloaderPrivate::playlistUpdated()
{
    downloadNextPart();
}

void StreamDownloader::StreamDownloaderPrivate::downloadNextPart()
{
    bool download = false;
    if(partIndex < 0)
    {
        playlist->setCurrentIndex(0);
        partIndex = 0;
        download = true;
    }
    else
    {
        int next = playlist->nextIndex();
        qDebug() << "downloadNextPart" << next << partIndex;
        if(next >= partIndex)
        {
            playlist->setCurrentIndex(next);
            ++partIndex;
            download = true;
        }
    }
    QUrl url;
    if(download)
    {
        QMediaContent media = playlist->currentMedia();
        qDebug() << "downloadNextPart" << media.canonicalUrl();
        url = media.canonicalUrl();
    }
    if(url.isValid())
        doDownload(url);
    else
    {
        qDebug() << "downloadNextPart redownload playlist";
        // re-download playlist
        playlist->reload();
    }
}

StreamDownloader::BackgroundThread::BackgroundThread(StreamDownloader * o)
    : owner(o)
{
}

void StreamDownloader::BackgroundThread::run()
{
    QTimer * timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &BackgroundThread::timer);
    qDebug() << "BackgroundThread::run()";
    owner->d_func()->downloadPlaylist();
    QThread::run();
    delete timer;
}

void StreamDownloader::BackgroundThread::timer()
{
}


StreamDownloader::StreamDownloader(QObject * parent)
    : QObject(parent)
    , d_ptr(new StreamDownloaderPrivate)
{
    Q_D(StreamDownloader);
    d->outfile.setFileName("/tmp/stream.mp4");
    d->outfile.open(QIODevice::WriteOnly|QIODevice::Truncate);
}

StreamDownloader::~StreamDownloader()
{
    Q_D(StreamDownloader);
    stop();
    if(d->backgroundThread)
        delete d->backgroundThread;
    delete d_ptr;
}

void StreamDownloader::play()
{
    Q_D(StreamDownloader);
    if(!d->backgroundThread)
        d->backgroundThread = new BackgroundThread(this);
    if(!d->backgroundThread->isRunning())
        d->backgroundThread->start();
}

void StreamDownloader::stop()
{
    Q_D(StreamDownloader);
    if(!d->backgroundThread)
        return;
    if(!d->backgroundThread->isFinished())
    {
        d->backgroundThread->quit();
        d->backgroundThread->wait();
    }
}

bool StreamDownloader::isPlaying() const
{
    Q_D(const StreamDownloader);
    if(d->backgroundThread != nullptr)
        return d->backgroundThread->isRunning();
    else
        return false;
}

void StreamDownloader::setUrl(const QUrl & url)
{
    Q_D(StreamDownloader);
    d->baseUrl = url;
}
