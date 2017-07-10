#pragma once

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(stream)

class Stream : public QObject
{
    Q_OBJECT
public:
    Stream(QObject * parent=nullptr);
};

class StreamDownloader : public QObject
{
    Q_OBJECT
public:
    StreamDownloader(QObject * parent=nullptr);
    ~StreamDownloader();

    void setUrl(const QUrl & url);

public slots:
    void play();
    void stop();

public:
    bool isPlaying() const;

protected:
    class Playlist;
    class BackgroundThread;
    class StreamDownloaderPrivate;
    Q_DECLARE_PRIVATE(StreamDownloader);
    StreamDownloaderPrivate * d_ptr;
};
