#pragma once

#include <QWidget>
#include <QVector>

class QTimer;

struct TimelineSampleData
{
    TimelineSampleData(uint64_t t = 0, float r = 0, float p = 0)
    {
        ValueInPercentage = p;
        RawValue = r;
        TimeStamp = t;
    }

    float       ValueInPercentage;
    float       RawValue;
    uint64_t    TimeStamp;
    QString     Description;
};

struct TimeLineTrack
{
    TimeLineTrack(QString const& Name = "", int MaxSamples = 50)
    {
        Description = Name;
        Init(MaxSamples);
    }

    QString                     Description;
    QVector<TimelineSampleData> CachedData;
    QVector<int>                DisplayIndexes; // By Index

    void Init(int MaxDisplaySample)
    {
        m_MaxSamples = MaxDisplaySample;
        DisplayIndexes.resize(MaxDisplaySample);
        CachedData.resize(MaxDisplaySample);
    }

    void Push(TimelineSampleData const& Data)
    {
        if (CachedData.size() >= m_MaxSamples)
        {
            CachedData.pop_front();
        }
        CachedData.push_back(Data);
    }

private:
    int m_MaxSamples;
};

/*
 * Time line data visualizer
 */
class TimeLineView : public QWidget
{
    Q_OBJECT

public:
    TimeLineView(QWidget* parent);
    ~TimeLineView() override;

    void Start();
    void Stop();

    void AppendSampleData(int TrackIndex, TimelineSampleData const& Data);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    //void onTimeOutRefresh();

private:
    QTimer* m_RefreshTimer;

private:
    int                     m_MaxSamples;
    QVector<TimeLineTrack>  m_Tracks[2]; // double buffer
};