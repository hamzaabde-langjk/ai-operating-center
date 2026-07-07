#include "gui/timeline_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QStyle>

namespace xray::gui {

TimelineWidget::TimelineWidget(QWidget* parent) : QDockWidget("Timeline", parent) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setupUi();
}

void TimelineWidget::setupUi() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);

    auto* controlsLayout = new QHBoxLayout();

    m_playButton = new QPushButton("Play", container);
    m_pauseButton = new QPushButton("Pause", container);
    m_recordButton = new QPushButton("Record", container);

    controlsLayout->addWidget(m_playButton);
    controlsLayout->addWidget(m_pauseButton);
    controlsLayout->addWidget(m_recordButton);
    controlsLayout->addStretch();

    m_timeLabel = new QLabel("00:00:00.000", container);
    controlsLayout->addWidget(m_timeLabel);

    layout->addLayout(controlsLayout);

    m_timelineSlider = new QSlider(Qt::Horizontal, container);
    m_timelineSlider->setRange(0, 1000);
    m_timelineSlider->setValue(0);
    layout->addWidget(m_timelineSlider);

    connect(m_playButton, &QPushButton::clicked, this, [this]() {
        m_playing = true;
        emit playRequested();
    });

    connect(m_pauseButton, &QPushButton::clicked, this, [this]() {
        m_playing = false;
        emit pauseRequested();
    });

    connect(m_timelineSlider, &QSlider::valueChanged, this, [this](int value) {
        qint64 timestamp = static_cast<qint64>(value) * m_duration / 1000;
        emit seekRequested(timestamp);
    });

    setWidget(container);
}

void TimelineWidget::setDuration(qint64 nanoseconds) {
    m_duration = nanoseconds;
}

void TimelineWidget::setCurrentTime(qint64 nanoseconds) {
    if (m_duration > 0) {
        int value = static_cast<int>(nanoseconds * 1000 / m_duration);
        m_timelineSlider->setValue(value);
    }

    qint64 ms = nanoseconds / 1000000;
    qint64 seconds = ms / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;

    m_timeLabel->setText(QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes % 60, 2, 10, QChar('0'))
        .arg(seconds % 60, 2, 10, QChar('0'))
        .arg(ms % 1000, 3, 10, QChar('0')));
}

void TimelineWidget::addBookmark(qint64 timestamp, const QString& label) {
    // TODO: Add visual bookmark markers
}

void TimelineWidget::clearBookmarks() {
    // TODO: Clear bookmarks
}

} // namespace xray::gui
