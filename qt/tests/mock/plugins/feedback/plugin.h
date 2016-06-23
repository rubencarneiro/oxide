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

#ifndef _OXIDE_QT_TESTS_MOCK_FEEDBACK_PLUGIN_H_
#define _OXIDE_QT_TESTS_MOCK_FEEDBACK_PLUGIN_H_

#include <QFeedbackInterface>
#include <QObject>

#include <map>
#include <memory>
#include <utility>

#include "qt/tests/mock/plugins/feedback/proxy.h"

QT_BEGIN_NAMESPACE
class QFeedbackActuator;
class QFeedbackHapticsEffect;
class QTimer;
QT_END_NAMESPACE

class FeedbackHapticsMock : public QObject,
                            public QFeedbackHapticsInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtFeedbackPlugin" FILE "plugin.json")
  Q_INTERFACES(QFeedbackHapticsInterface)

 public:
  FeedbackHapticsMock(QObject* parent = nullptr);
  ~FeedbackHapticsMock() override;

  // QFeedbackHapticsInterface implementation
  QList<QFeedbackActuator*> actuators() override;
  PluginPriority pluginPriority() override;
  void setActuatorProperty(const QFeedbackActuator& actuator,
                           ActuatorProperty property,
                           const QVariant& value) override;
  QVariant actuatorProperty(const QFeedbackActuator& actuator,
                            ActuatorProperty property) override;
  bool isActuatorCapabilitySupported(const QFeedbackActuator& actuator,
                                     QFeedbackActuator::Capability cap) override;
  void updateEffectProperty(const QFeedbackHapticsEffect* effect,
                            EffectProperty property) override;
  void setEffectState(const QFeedbackHapticsEffect* effect,
                      QFeedbackEffect::State state) override;
  QFeedbackEffect::State effectState(
      const QFeedbackHapticsEffect* effect) override;

 private Q_SLOTS:

 private:
  struct EffectData {
    EffectData();
    EffectData(int id);
    EffectData(EffectData&& other);
    ~EffectData();

    int id;
    std::unique_ptr<QTimer> timer;
    int remaining_time;
  };

  void killEffectTimer(const QFeedbackHapticsEffect* effect);
  void startEffectTimer(const QFeedbackHapticsEffect* effect);

  std::unique_ptr<QFeedbackActuator> actuator_;
  FeedbackHapticsMockProxy proxy_;
  std::map<const QFeedbackHapticsEffect*, EffectData> effect_data_;
};

#endif // _OXIDE_QT_TESTS_MOCK_FEEDBACK_PLUGIN_H_
