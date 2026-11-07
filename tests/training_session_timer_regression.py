#!/usr/bin/env python3
"""Guard the training session clock against a 10x duration regression."""
from pathlib import Path

source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
if "m_timer->setInterval(1000);  // 1 Hz" not in source:
    raise SystemExit("Training session timer must tick once per second")
if "m_timer->setInterval(100);  // 10 Hz" in source:
    raise SystemExit("Training session clock must not count 100 ms ticks as seconds")
if "connect(m_timer, &QTimer::timeout, this, &TrainingTab::onTimerTick);" not in source:
    raise SystemExit("Training session clock timer must remain connected to onTimerTick")
start = source.index("void TrainingTab::onTimerTick()")
end = source.index("void TrainingTab::onMeasurementSample", start)
tick = source[start:end]
if tick.count("m_sessionTime++;") != 1:
    raise SystemExit("Each timer tick must increment elapsed session time exactly once")
print("Training session clock cadence regression passed")
