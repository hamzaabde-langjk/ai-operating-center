#pragma once

#include <QDockWidget>
#include <QSlider>
#include <QPushButton>

namespace xray::gui {

class TimelineWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);

    void setDuration(qint64 nanoseconds);
    void setCurrentTime(qint64 nanoseconds);
    void addBookmark(qint64 timestamp, const QString& label);
    void clearBookmarks();

signals:
    void seekRequested(qint64 timestamp);
    void playRequested();
    void pauseRequested();
    void speedChanged(float speed);

private:
    void setupUi();

    QSlider* m_timelineSlider{nullptr};
    QPushButton* m_playButton{nullptr};
    QPushButton* m_pauseButton{nullptr};
    QPushButton* m_recordButton{nullptr};
    QLabel* m_timeLabel{nullptr};

    qint64 m_duration{0};
    bool m_playing{false};
    float m_speed{1.0f};
};

} // namespace xray::gui
