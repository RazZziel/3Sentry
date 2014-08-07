#include <QMetaEnum>
#include <QDir>
#include <QMediaPlayer>
#include <QDebug>
#include <QDateTime>
#include "audio.h"

#define AUDIO_PATH "/home/raziel/Sound/eva00_Sound/Sounds/portal turret/npc/turret_floor"

Audio::Audio(QObject *parent) :
    QObject(parent),
    m_player(new QMediaPlayer())
{
    m_player->setVolume(50);

    qsrand(QTime::currentTime().msec());
}

bool Audio::isPlaying()
{
    QMediaPlayer::MediaStatus status = m_player->mediaStatus();

    return (status != QMediaPlayer::NoMedia &&
            status != QMediaPlayer::EndOfMedia);
}

void Audio::playRandom(Type type)
{
    QString filename = getRandomFilename(type);

    qDebug() << "Playing" << filename;

    m_player->setMedia(QUrl::fromLocalFile(filename));
    m_player->play();
}


QString Audio::getRandomFilename(Type type)
{
    QMetaObject metaObject = Audio::staticMetaObject;
    QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("Type"));

    QString blob = QString("turret_%2_*.wav").arg(QString(metaEnum.valueToKey(type)).toLower());

    QFileInfoList entries = QDir(AUDIO_PATH).entryInfoList(QStringList() << blob,
                                                           QDir::Files | QDir::Readable,
                                                           QDir::Unsorted);

    QFileInfo randomFile = entries.value(qrand() % entries.count());

    return randomFile.absoluteFilePath();
}
