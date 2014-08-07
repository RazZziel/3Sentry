#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>

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

    void playRandom(Type type);

private:
    QString getRandomFilename(Type type);

    QMediaPlayer *m_player;
};

#endif // AUDIO_H
