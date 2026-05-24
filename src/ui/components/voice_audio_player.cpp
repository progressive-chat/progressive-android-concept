#include "ui/components/voice_audio_player.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QEnterEvent>
#include <QLinearGradient>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QFontMetrics>
#include <QCursor>

#include <random>
#include <algorithm>



// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

VoiceAudioPlayer::VoiceAudioPlayer(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

#if HAS_QT_MULTIMEDIA
    m_player = new QMediaPlayer(this);
    m_audio  = new QAudioOutput(this);
    m_player->setAudioOutput(m_audio);
    m_audio->setVolume(m_volume);
    m_fallback = false;

    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &VoiceAudioPlayer::onStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &VoiceAudioPlayer::onStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &VoiceAudioPlayer::onError);
    connect(m_player, &QMediaPlayer::positionChanged,
            this, [this](qint64 pos) {
                m_position = pos;
                emit positionChanged(pos);
            });
    connect(m_player, &QMediaPlayer::durationChanged,
            this, [this](qint64 dur) {
                m_duration = dur;
                if (dur > 0 && m_waveform.isEmpty())
                    generateWaveform();
            });
#else
    m_fallback = true;
#endif

    m_timer = new QTimer(this);
    m_timer->setInterval(50);

    m_fallbackTimer = new QTimer(this);
    m_fallbackTimer->setInterval(100);

    connect(m_timer, &QTimer::timeout, this, &VoiceAudioPlayer::onTick);
    connect(m_fallbackTimer, &QTimer::timeout, this, [this]() {
        if (!m_fallback || !m_playing)
            return;
        m_position += qint64(m_fallbackTimer->interval() * m_speed);
        if (m_position >= m_duration) {
            m_position = m_duration;
            m_playing = false;
            m_fallbackTimer->stop();
            m_timer->stop();
            emit positionChanged(m_position);
            emit playbackFinished();
        } else {
            emit positionChanged(m_position);
        }
        update();
    });

    generateWaveform();
    updateLayout();
}

VoiceAudioPlayer::~VoiceAudioPlayer()
{
#if HAS_QT_MULTIMEDIA
    m_player->stop();
#endif
    m_fallbackTimer->stop();
    if (!m_tempFile.isEmpty())
        QFile::remove(m_tempFile);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::loadAudio(const QString &filePath)
{
#if HAS_QT_MULTIMEDIA
    m_player->stop();
#endif
    m_fallbackTimer->stop();
    m_timer->stop();
    m_playing = false;
    m_position = 0;
    m_duration = 0;

    const QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isReadable()) {
        emit errorOccurred(QStringLiteral("File not found: %1").arg(filePath));
        update();
        return;
    }

#if HAS_QT_MULTIMEDIA
    m_fallback = false;
    m_loaded = false;
    m_player->setSource(QUrl::fromLocalFile(fi.absoluteFilePath()));
#else
    m_fallback = true;
    m_loaded = true;
    m_duration = 30000;
    generateWaveform();
    emit positionChanged(0);
#endif
    update();
}

void VoiceAudioPlayer::loadAudio(const QByteArray &data, const QString &mimeType)
{
    Q_UNUSED(mimeType);

    if (!m_tempFile.isEmpty())
        QFile::remove(m_tempFile);

    QTemporaryFile tmp(QDir::tempPath() + QStringLiteral("/voice_XXXXXX"));
    tmp.setAutoRemove(false);
    if (!tmp.open()) {
        emit errorOccurred(QStringLiteral("Failed to create temporary file"));
        return;
    }
    tmp.write(data);
    tmp.close();
    m_tempFile = tmp.fileName();
    loadAudio(m_tempFile);
}

void VoiceAudioPlayer::setMiniMode(bool mini)
{
    if (m_mini == mini)
        return;
    m_mini = mini;
    updateLayout();
    updateGeometry();
    update();
}

bool VoiceAudioPlayer::isMiniMode() const
{
    return m_mini;
}

void VoiceAudioPlayer::setPlaybackSpeed(float speed)
{
    m_speed = std::clamp(speed, 0.25f, 4.0f);
#if HAS_QT_MULTIMEDIA
    m_player->setPlaybackRate(m_speed);
#endif

    m_speedIdx = -1;
    for (int i = 0; i < 3; ++i) {
        if (qFuzzyCompare(m_speed, kSpeeds[i])) {
            m_speedIdx = i;
            break;
        }
    }
    update();
}

float VoiceAudioPlayer::playbackSpeed() const
{
    return m_speed;
}

void VoiceAudioPlayer::setVolume(float volume)
{
    m_volume = std::clamp(volume, 0.0f, 1.0f);
#if HAS_QT_MULTIMEDIA
    m_audio->setVolume(m_volume);
#endif
    update();
}

float VoiceAudioPlayer::volume() const
{
    return m_volume;
}

qint64 VoiceAudioPlayer::position() const
{
    return m_position;
}

qint64 VoiceAudioPlayer::duration() const
{
    return m_duration;
}

QSize VoiceAudioPlayer::minimumSizeHint() const
{
    const int h = m_mini ? kMiniH : kFullH;
    return QSize(200, h);
}

QSize VoiceAudioPlayer::sizeHint() const
{
    return minimumSizeHint();
}

// ---------------------------------------------------------------------------
// paintEvent
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int h = height();
    const int w = width();
    if (w <= 0 || h <= 0)
        return;

    {
        QPainterPath bg;
        bg.addRoundedRect(QRectF(0, 0, w, h), kRadius, kRadius);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(30, 41, 59));
        p.drawPath(bg);
    }

    {
        const QRectF &r = m_l.playBtn;
        if (r.isValid()) {
            p.setPen(Qt::NoPen);
            p.setBrush(m_loaded ? QColor(59, 130, 246) : QColor(71, 85, 105));
            p.drawEllipse(r);

            p.setBrush(Qt::white);
            const qreal cx = r.center().x();
            const qreal cy = r.center().y();
            const qreal s = r.width() * 0.40;

            if (m_playing) {
                const qreal bw = s * 0.36;
                const qreal bh = s;
                const qreal gap = bw * 0.8;
                p.drawRoundedRect(QRectF(cx - bw - gap * 0.5, cy - bh * 0.5, bw, bh), 2, 2);
                p.drawRoundedRect(QRectF(cx + gap * 0.5,       cy - bh * 0.5, bw, bh), 2, 2);
            } else {
                QPainterPath tri;
                tri.moveTo(cx - s * 0.35, cy - s * 0.55);
                tri.lineTo(cx - s * 0.35, cy + s * 0.55);
                tri.lineTo(cx + s * 0.65, cy);
                tri.closeSubpath();
                p.drawPath(tri);
            }
        }
    }

    if (m_l.waveform.isValid() && !m_waveform.isEmpty()) {
        const QRectF &wr = m_l.waveform;
        const int bars = qMin(kWaveBars, m_waveform.size());
        const qreal step = wr.width() / bars;
        const qreal barW = qMax(1.0, step - 0.5);
        const qreal maxH = wr.height() * 0.85;
        const qreal baseY = wr.center().y();
        const int playedTo = (m_loaded && m_duration > 0)
            ? int(bars * (double(m_position) / double(m_duration)))
            : 0;

        for (int i = 0; i < bars; ++i) {
            const qreal x = wr.x() + i * step;
            const qreal amp = m_waveform[i];
            const qreal barH = qMax(2.0, amp * maxH);

            QColor color = (i <= playedTo)
                ? QColor(59, 130, 246)
                : QColor(100, 116, 139);
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            p.drawRoundedRect(QRectF(x, baseY - barH * 0.5, barW, barH), 1, 1);
        }
    }

    {
        const QRectF &r = m_l.timeLabel;
        if (r.isValid()) {
            p.setPen(QColor(200, 210, 220));
            QFont f = font();
            f.setPixelSize(12);
            f.setStyleHint(QFont::Monospace);
            p.setFont(f);
            p.drawText(r, Qt::AlignRight | Qt::AlignVCenter,
                       QStringLiteral("%1 / %2")
                           .arg(formatTime(m_position), formatTime(m_duration)));
        }
    }

    if (m_mini)
        return;

    {
        const QRectF &r = m_l.speedBtn;
        if (r.isValid()) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(71, 85, 105));
            p.drawRoundedRect(r, 4, 4);

            p.setPen(QColor(200, 210, 220));
            QFont f = font();
            f.setPixelSize(10);
            p.setFont(f);
            p.drawText(r, Qt::AlignCenter, QStringLiteral("%1x").arg(m_speed));
        }
    }

    {
        const QRectF &ic = m_l.volIcon;
        if (!ic.isValid())
            return;

        const qreal cx = ic.center().x();
        const qreal cy = ic.center().y();
        const qreal iw = kVolIconW;
        const qreal ih = kVolIconH;

        p.setPen(QPen(QColor(200, 210, 220), 1.5));
        p.setBrush(Qt::NoBrush);

        QRectF cone(cx - iw * 0.45, cy - ih * 0.35, iw * 0.27, ih * 0.7);
        p.drawRect(cone);

        QPainterPath wedge;
        wedge.moveTo(cone.right(), cone.top());
        wedge.lineTo(cone.right(), cone.bottom());
        wedge.lineTo(cone.right() + iw * 0.22, cy);
        wedge.closeSubpath();
        p.setBrush(QColor(200, 210, 220));
        p.drawPath(wedge);

        if (m_volume > 0.0f) {
            p.setPen(QPen(QColor(200, 210, 220), 1.2));
            p.setBrush(Qt::NoBrush);
            const qreal arcStart = cx + iw * 0.05;
            for (int i = 0; i < 3; ++i) {
                const qreal r = ih * 0.22 + i * ih * 0.15;
                const qreal ax = arcStart + i * iw * 0.13;
                QRectF arcR(ax - r, cy - r, r * 2, r * 2);
                p.drawArc(arcR, -60 * 16, 120 * 16);
            }
        } else {
            p.setPen(QPen(QColor(239, 68, 68), 1.2));
            const qreal mx = cx + iw * 0.1;
            const qreal md = ih * 0.4;
            p.drawLine(QPointF(mx - md, cy - md), QPointF(mx + md, cy + md));
            p.drawLine(QPointF(mx + md, cy - md), QPointF(mx - md, cy + md));
        }

        if (m_volHovered && m_l.volSlider.isValid()) {
            const QRectF &sr = m_l.volSlider;
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(71, 85, 105));
            p.drawRoundedRect(sr, 2, 2);

            if (m_volume > 0.0f) {
                QRectF fill = sr;
                fill.setWidth(sr.width() * m_volume);
                p.setBrush(QColor(59, 130, 246));
                p.drawRoundedRect(fill, 2, 2);
            }

            const qreal thumbX = sr.x() + sr.width() * m_volume;
            const qreal thumbR = 5;
            p.setBrush(QColor(200, 210, 220));
            p.drawEllipse(QPointF(thumbX, sr.center().y()), thumbR, thumbR);
        }
    }
}

// ---------------------------------------------------------------------------
// Mouse events
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::mousePressEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();

    if (m_l.playBtn.contains(pos)) {
        togglePlay();
        return;
    }

    if (!m_mini && m_l.speedBtn.contains(pos)) {
        cycleSpeed();
        return;
    }

    if (!m_mini && m_l.volIcon.contains(pos)) {
        m_volDragging = true;
        const QRectF sr = m_l.volSlider.isValid()
            ? m_l.volSlider
            : QRectF(m_l.volIcon.right() + 4,
                     m_l.volIcon.center().y() - 10, 48, 20);
        const qreal frac = qBound(0.0, (pos.x() - sr.x()) / sr.width(), 1.0);
        setVolume(float(frac));
        return;
    }

    if (m_loaded && m_l.waveform.contains(pos))
        seekWaveform(pos);
}

void VoiceAudioPlayer::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();

    if (m_volDragging) {
        const QRectF sr = m_l.volSlider.isValid()
            ? m_l.volSlider
            : QRectF(m_l.volIcon.right() + 4,
                     m_l.volIcon.center().y() - 10, 48, 20);
        const qreal frac = qBound(0.0, (pos.x() - sr.x()) / sr.width(), 1.0);
        setVolume(float(frac));
        return;
    }

    const bool overVol = !m_mini && m_l.volIcon.contains(pos);
    if (overVol != m_volHovered) {
        m_volHovered = overVol;
        updateLayout();
        update();
    }

    const bool clickable =
        m_l.playBtn.contains(pos) ||
        m_l.waveform.contains(pos) ||
        (!m_mini && m_l.speedBtn.contains(pos)) ||
        overVol;
    setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void VoiceAudioPlayer::enterEvent(QEnterEvent * /*event*/)
{
    m_hovered = true;
    update();
}

void VoiceAudioPlayer::leaveEvent(QEvent * /*event*/)
{
    m_hovered = false;
    if (m_volHovered) {
        m_volHovered = false;
        updateLayout();
    }
    m_volDragging = false;
    update();
}

// ---------------------------------------------------------------------------
// resizeEvent
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::resizeEvent(QResizeEvent * /*event*/)
{
    updateLayout();
}

// ---------------------------------------------------------------------------
// Private slots  (Qt Multimedia)
// ---------------------------------------------------------------------------

#if HAS_QT_MULTIMEDIA

void VoiceAudioPlayer::onStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        if (!m_playing) {
            m_playing = true;
            m_timer->start();
            if (!m_fallback)
                emit playbackStarted();
        }
        break;
    case QMediaPlayer::PausedState:
        if (m_playing) {
            m_playing = false;
            m_timer->stop();
            emit playbackPaused();
        }
        break;
    case QMediaPlayer::StoppedState:
        if (m_playing) {
            m_playing = false;
            m_timer->stop();
            emit playbackPaused();
        }
        break;
    }
    update();
}

void VoiceAudioPlayer::onStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        m_loaded = true;
        m_fallback = false;
        m_duration = m_player->duration();
        if (m_waveform.isEmpty())
            generateWaveform();
        update();
        break;
    case QMediaPlayer::EndOfMedia:
        m_playing = false;
        m_timer->stop();
        m_fallbackTimer->stop();
        m_position = m_duration;
        emit positionChanged(m_position);
        emit playbackFinished();
        update();
        break;
    case QMediaPlayer::InvalidMedia:
        m_fallback = true;
        m_loaded = true;
        if (m_duration <= 0)
            m_duration = 30000;
        generateWaveform();
        emit errorOccurred(QStringLiteral("Media could not be loaded – using simulation"));
        update();
        break;
    default:
        break;
    }
}

void VoiceAudioPlayer::onError(QMediaPlayer::Error error, const QString &errorStr)
{
    Q_UNUSED(error);
    m_fallback = true;
    m_loaded = true;
    if (m_duration <= 0)
        m_duration = 30000;
    m_timer->stop();
    m_fallbackTimer->stop();
    m_playing = false;
    generateWaveform();
    emit errorOccurred(errorStr);
    update();
}

#endif // HAS_QT_MULTIMEDIA

// ---------------------------------------------------------------------------
// onTick  (common)
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::onTick()
{
#if HAS_QT_MULTIMEDIA
    if (m_playing && !m_fallback)
        m_position = m_player->position();
#endif
    update();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void VoiceAudioPlayer::generateWaveform()
{
    m_waveform.resize(kWaveBars);

    static std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.18f, 0.18f);

    float val = 0.35f;
    for (auto &v : m_waveform) {
        val += dist(rng);
        val = std::clamp(val, 0.06f, 1.0f);
        v = val;
    }

    update();
}

void VoiceAudioPlayer::updateLayout()
{
    const int pH  = kPadH;
    const int pV  = m_mini ? kMiniPadV : kPadV;
    const int gap = m_mini ? kMiniGap : kGap;
    const int playSz = m_mini ? kMiniPlaySz : kPlaySz;
    const int timeW  = m_mini ? kMiniTimeW : kTimeW;

    const qreal centerY = height() / 2.0;
    qreal right = width() - pH;
    qreal left  = pH;

    m_l.timeLabel = QRectF(right - timeW, centerY - 12, timeW, 24);
    right -= timeW;

    if (!m_mini) {
        right -= gap;
        const qreal volW = m_volHovered
            ? (kVolIconW + kVolSliderW + 6)
            : kVolIconW;
        m_l.volIcon = QRectF(right - kVolIconW, centerY - kVolIconH * 0.5,
                             kVolIconW, kVolIconH);
        m_l.volSlider = m_volHovered
            ? QRectF(right - kVolIconW - kVolSliderW - 6,
                     centerY - kVolSliderH * 0.5,
                     kVolSliderW, kVolSliderH)
            : QRectF();
        right -= volW;

        right -= gap;
        m_l.speedBtn = QRectF(right - kSpeedW, centerY - kSpeedH * 0.5,
                              kSpeedW, kSpeedH);
        right -= kSpeedW;
    } else {
        m_l.speedBtn  = QRectF();
        m_l.volIcon   = QRectF();
        m_l.volSlider = QRectF();
    }

    right -= gap;

    m_l.playBtn = QRectF(left, centerY - playSz * 0.5, playSz, playSz);
    left += playSz + gap;

    m_l.waveform = QRectF(left, pV, right - left, height() - 2.0 * pV);
}

void VoiceAudioPlayer::togglePlay()
{
    if (!m_loaded)
        return;

    if (m_playing) {
        m_fallbackTimer->stop();
        m_timer->stop();
        m_playing = false;
#if HAS_QT_MULTIMEDIA
        if (!m_fallback)
            m_player->pause();
#endif
        emit playbackPaused();
    } else {
#if HAS_QT_MULTIMEDIA
        if (!m_fallback) {
            m_player->play();
        } else
#endif
        {
            if (m_duration <= 0)
                m_duration = 30000;
            if (m_position >= m_duration)
                m_position = 0;
            m_playing = true;
            m_timer->start();
            m_fallbackTimer->start();
            emit playbackStarted();
        }
    }
    update();
}

void VoiceAudioPlayer::cycleSpeed()
{
    m_speedIdx = (m_speedIdx + 1) % 3;
    setPlaybackSpeed(kSpeeds[m_speedIdx]);
}

void VoiceAudioPlayer::seekWaveform(const QPointF &pos)
{
    if (!m_loaded || m_duration <= 0)
        return;

    const QRectF &wr = m_l.waveform;
    const qreal frac = qBound(0.0, (pos.x() - wr.x()) / wr.width(), 1.0);
    m_position = qint64(m_duration * frac);

#if HAS_QT_MULTIMEDIA
    if (!m_fallback)
        m_player->setPosition(m_position);
#endif
    emit positionChanged(m_position);
    update();
}

QString VoiceAudioPlayer::formatTime(qint64 ms)
{
    if (ms <= 0)
        return QStringLiteral("0:00");

    const int totalSec = int(ms / 1000);
    const int min = totalSec / 60;
    const int sec = totalSec % 60;

    if (min > 0)
        return QStringLiteral("%1:%2").arg(min).arg(sec, 2, 10, QLatin1Char('0'));
    return QStringLiteral("0:%1").arg(sec, 2, 10, QLatin1Char('0'));
}
