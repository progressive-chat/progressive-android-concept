#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QByteArray>
#include <QString>

#if HAS_QT_MULTIMEDIA
#include <QMediaPlayer>
#include <QAudioOutput>
#endif

class VoiceAudioPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VoiceAudioPlayer(QWidget *parent = nullptr);
    ~VoiceAudioPlayer() override;

    void loadAudio(const QString &filePath);
    void loadAudio(const QByteArray &data, const QString &mimeType);

    void setMiniMode(bool mini);
    bool isMiniMode() const;

    void setPlaybackSpeed(float speed);
    float playbackSpeed() const;

    void setVolume(float volume);
    float volume() const;

    qint64 position() const;
    qint64 duration() const;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void playbackStarted();
    void playbackPaused();
    void playbackFinished();
    void positionChanged(qint64 ms);
    void errorOccurred(const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
#if HAS_QT_MULTIMEDIA
    void onStateChanged(QMediaPlayer::PlaybackState state);
    void onStatusChanged(QMediaPlayer::MediaStatus status);
    void onError(QMediaPlayer::Error error, const QString &errorStr);
#endif
    void onTick();

private:
    void generateWaveform();
    void updateLayout();
    void togglePlay();
    void cycleSpeed();
    void seekWaveform(const QPointF &pos);

    static QString formatTime(qint64 ms);

#if HAS_QT_MULTIMEDIA
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
#endif
    QTimer *m_timer = nullptr;
    QTimer *m_fallbackTimer = nullptr;

    QVector<float> m_waveform;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    bool m_playing = false;
    bool m_loaded = false;
    bool m_mini = false;
    bool m_hovered = false;
    bool m_volHovered = false;
    bool m_volDragging = false;
    float m_speed = 1.0f;
    float m_volume = 1.0f;
    int m_speedIdx = 0;
    bool m_fallback = true;
    QString m_tempFile;

    struct Layout {
        QRectF playBtn;
        QRectF waveform;
        QRectF speedBtn;
        QRectF volIcon;
        QRectF volSlider;
        QRectF timeLabel;
    } m_l;

    static constexpr int kMiniH = 40;
    static constexpr int kFullH = 60;
    static constexpr int kPadH = 8;
    static constexpr int kPadV = 10;
    static constexpr int kMiniPadV = 6;
    static constexpr int kGap = 8;
    static constexpr int kMiniGap = 6;
    static constexpr int kPlaySz = 36;
    static constexpr int kMiniPlaySz = 28;
    static constexpr int kSpeedW = 42;
    static constexpr int kSpeedH = 22;
    static constexpr int kVolIconW = 22;
    static constexpr int kVolIconH = 18;
    static constexpr int kVolSliderW = 48;
    static constexpr int kVolSliderH = 4;
    static constexpr int kTimeW = 80;
    static constexpr int kMiniTimeW = 70;
    static constexpr int kRadius = 8;
    static constexpr int kWaveBars = 200;
    static constexpr float kSpeeds[3] = {1.0f, 1.5f, 2.0f};
};
