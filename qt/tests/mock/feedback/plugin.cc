// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "plugin.h"

#include <QCoreApplication>
#include <QFeedbackActuator>
#include <QFeedbackEffect>
#include <QTimer>
#include <QVariant>

FeedbackHapticsMock::EffectData::EffectData()
    : id(-1),
      remaining_time(0) {}

FeedbackHapticsMock::EffectData::EffectData(int id)
    : id(id),
      remaining_time(0) {}

FeedbackHapticsMock::EffectData::EffectData(EffectData&& other)
    : id(std::move(other.id)),
      timer(std::move(other.timer)),
      remaining_time(std::move(other.remaining_time)) {}

FeedbackHapticsMock::EffectData::~EffectData() {}

void FeedbackHapticsMock::killEffectTimer(
    const QFeedbackHapticsEffect* effect) {
  EffectData& data = effect_data_[effect];

  data.remaining_time = data.timer->remainingTime();
  data.timer->stop();
}

void FeedbackHapticsMock::startEffectTimer(
    const QFeedbackHapticsEffect* effect) {
  EffectData& data = effect_data_[effect];

  bool resume = false;
  if (!data.timer) {
    data.timer.reset(new QTimer());
    connect(data.timer.get(), &QTimer::timeout, [this, effect] {
      killEffectTimer(effect);
      Q_EMIT const_cast<QFeedbackHapticsEffect*>(effect)->stateChanged();
    });
    resume = true;
  }

  data.timer->start(resume ? data.remaining_time : effect->duration());
}

FeedbackHapticsMock::FeedbackHapticsMock(QObject* parent)
    : QObject(parent),
      actuator_(createFeedbackActuator(nullptr, 1)) {
  QCoreApplication::instance()->setProperty("_oxide_feedback_mock_api",
                                            QVariant::fromValue(&proxy_));
}

FeedbackHapticsMock::~FeedbackHapticsMock() {
  Q_ASSERT(!QCoreApplication::instance());
}

QList<QFeedbackActuator*> FeedbackHapticsMock::actuators() {
  return QList<QFeedbackActuator*>() << actuator_.get();
}

QFeedbackInterface::PluginPriority FeedbackHapticsMock::pluginPriority() {
  return PluginHighPriority;
}

void FeedbackHapticsMock::setActuatorProperty(const QFeedbackActuator& actuator,
                                              ActuatorProperty property,
                                              const QVariant& value) {
  Q_ASSERT(false);
}

QVariant FeedbackHapticsMock::actuatorProperty(
    const QFeedbackActuator& actuator,
    ActuatorProperty property) {
  switch (property) {
    case Name:
      return QString::fromLocal8Bit("Oxide test actuator");
    case State:
      return QFeedbackActuator::Ready;
    case Enabled:
      return true;
  }
}

bool FeedbackHapticsMock::isActuatorCapabilitySupported(
    const QFeedbackActuator& actuator,
    QFeedbackActuator::Capability cap) {
  return false;
}

void FeedbackHapticsMock::updateEffectProperty(
    const QFeedbackHapticsEffect* effect,
    EffectProperty property) {}

void FeedbackHapticsMock::setEffectState(const QFeedbackHapticsEffect* effect,
                                         QFeedbackEffect::State state) {
  static int g_next_id = 0;

  switch (state) {
    case QFeedbackEffect::Stopped: {
      EffectData& data = effect_data_[effect];
      killEffectTimer(effect);
      Q_EMIT proxy_.effectStopped(data.id);
      effect_data_.erase(effect);
      break;
    }

    case QFeedbackEffect::Paused: {
      EffectData& data = effect_data_[effect];
      killEffectTimer(effect);
      break;
    }

    case QFeedbackEffect::Running: {
      auto it = effect_data_.find(effect);
      if (it == effect_data_.end()) {
        effect_data_.insert(std::make_pair(effect, EffectData(g_next_id++)));
      }
      startEffectTimer(effect);
      Q_EMIT proxy_.effectStarted(effect_data_[effect].id,
                                  effect->duration(),
                                  effect->intensity());
      break;
    }
  }
}

QFeedbackEffect::State FeedbackHapticsMock::effectState(
    const QFeedbackHapticsEffect* effect) {
  if (effect_data_.find(effect) == effect_data_.end()) {
    return QFeedbackEffect::Stopped;
  }

  const EffectData& data = effect_data_[effect];

  if (!data.timer->isActive()) {
    return QFeedbackEffect::Paused;
  }

  return QFeedbackEffect::Running;
}
