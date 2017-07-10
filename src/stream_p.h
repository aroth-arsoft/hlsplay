#pragma once

#include <QtCore/QThread>
#include <QtNetwork/QNetworkAccessManager>
#include <QFile>
#include <QMediaPlaylist>

class QTimer;

class StreamDownloader::Playlist : public QMediaPlaylist
{
    Q_OBJECT
public:
    Playlist(QObject * parent=nullptr);
    ~Playlist();

public:
    void load(const QNetworkRequest &request, const char *format = Q_NULLPTR);

public:
    void reload();

private:
    void loadFinished();
    void reloadImpl();
    void reloadTimer();

signals:
    void triggerReload();
    void updated();

private:
    QMediaPlaylist * pending;
    QTimer * timer;
    QNetworkRequest lastRequest;
};

class StreamDownloader::StreamDownloaderPrivate : public QObject
{
    Q_OBJECT
    friend class StreamDownloader::BackgroundThread;
public:
    BackgroundThread * backgroundThread;
    QUrl baseUrl;
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;
    Playlist * playlist;
    QFile outfile;
    int partIndex;

    StreamDownloaderPrivate();
    ~StreamDownloaderPrivate();

    void downloadPlaylist();
    void downloadNextPart();
    void doDownload(const QUrl &url);
#ifndef QT_NO_SSL
    void sslErrors(const QList<QSslError> & errors);
#endif
    void partFinished(QNetworkReply *reply);
    void playlistUpdated();

signals:
    void triggerDownloadPlayload();
};

class StreamDownloader::BackgroundThread : public QThread
{
    StreamDownloader * owner;
public:
    BackgroundThread(StreamDownloader * o);
    void run() override;
    void timer();
};
