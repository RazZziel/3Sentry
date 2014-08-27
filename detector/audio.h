#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QHash>

class QMediaPlayer;

class Audio : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
public:
    enum Type { Active, Autosearch, Collide,
                Deploy, Disabled, Fizzler, Pickup,
                Retire, Search, ShotAt, Tipped };

    explicit Audio(QObject *parent = 0);

    bool isPlaying();

    void play(Type type, int index=-1);

private:
    QString getFilename(Type type, int index=-1);

    QMediaPlayer *m_player;
    QHash<Type,int> m_lastSampleIndex;
};

#endif // AUDIO_H
