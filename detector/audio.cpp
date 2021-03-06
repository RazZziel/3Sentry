#include <QMetaEnum>
#include <QDir>
#include <QMediaPlayer>
#include <QDebug>
#include <QDateTime>
#include "audio.h"

#define AUDIO_PATH "/home/raziel/Sound/eva00_Sound/Sounds/portal turret/npc/turret_floor"

Audio::Audio(QObject *parent) :
    QObject(parent),
    m_player(new QMediaPlayer()),
    m_lastType(Retire)
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

bool Audio::isPlaying(Type type)
{
    return isPlaying() && (m_lastType == type);
}

void Audio::play(Type type, int index)
{
    QString filename = getFilename(type, index);

    qDebug() << "Playing" << filename;

    m_player->setMedia(QUrl::fromLocalFile(filename));
    m_player->play();

    m_lastType = type;
}


QString Audio::getFilename(Type type, int index)
{
    QMetaObject metaObject = Audio::staticMetaObject;
    QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("Type"));

    QString blob = QString("turret_%2_*.wav").arg(QString(metaEnum.valueToKey(type)).toLower());

    QFileInfoList entries = QDir(AUDIO_PATH).entryInfoList(QStringList() << blob,
                                                           QDir::Files | QDir::Readable,
                                                           QDir::Unsorted);

    if (index < 0 || index >= entries.count())
    {
        do
        {
            index = qrand() % entries.count();
        }
        while (m_lastSampleIndex.contains(type) &&
               m_lastSampleIndex.value(type) == index);

        m_lastSampleIndex[type] = index;
    }

    QFileInfo randomFile = entries.value(index);

    return randomFile.absoluteFilePath();
}
