/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "OverlayHubView.h"

#include <QTimer>

namespace uwf::ui {

OverlayHubView::OverlayHubView(QWidget* parent, const Qt::WindowFlags flags)
    : QWidget(parent, flags), m_confirmationTimer(new QTimer(this)), m_retryTimer(new QTimer(this)), m_healthTimer(new QTimer(this)) {
  m_confirmationTimer->setSingleShot(true);
  m_confirmationTimer->setTimerType(Qt::CoarseTimer);
  connect(m_confirmationTimer, &QTimer::timeout, this, [this]() {
    if (!m_presentationRequested || m_displayState != DisplayState::Attaching) return;
    repaint();
    if (verifyPresentation()) {
      transitionToConfirmed();
      return;
    }
    detachPresentation();
    transitionToUnavailable();
  });

  m_retryTimer->setSingleShot(true);
  m_retryTimer->setTimerType(Qt::CoarseTimer);
  connect(m_retryTimer, &QTimer::timeout, this, [this]() {
    if (m_presentationRequested) refreshPresentation();
  });

  m_healthTimer->setTimerType(Qt::CoarseTimer);
  connect(m_healthTimer, &QTimer::timeout, this, [this]() {
    if (m_presentationRequested) refreshPresentation();
  });
}

void OverlayHubView::setPresentationRequested(const bool requested) {
  if (m_presentationRequested == requested) {
    if (requested && m_displayState == DisplayState::Confirmed && !verifyPresentation()) refreshPresentation();
    return;
  }
  m_presentationRequested = requested;
  if (!requested) {
    m_confirmationTimer->stop();
    m_retryTimer->stop();
    m_healthTimer->stop();
    detachPresentation();
    setDisplayState(DisplayState::Unavailable);
    return;
  }
  refreshPresentation();
}

bool OverlayHubView::presentationVerified() const { return m_presentationRequested && m_displayState == DisplayState::Confirmed && verifyPresentation(); }

bool OverlayHubView::attachPresentation() {
  show();
  return true;
}

void OverlayHubView::detachPresentation() { hide(); }

void OverlayHubView::requestPresentationRefresh() {
  if (m_presentationRequested) refreshPresentation();
}

void OverlayHubView::notifyPresentationChanged() {
  if (!m_presentationRequested) return;
  if (verifyPresentation()) {
    transitionToConfirmed();
  } else if (m_displayState == DisplayState::Confirmed) {
    refreshPresentation();
  }
}

void OverlayHubView::refreshPresentation() {
  if (!m_presentationRequested) return;
  m_retryTimer->stop();
  m_healthTimer->stop();

  if (!attachPresentation()) {
    detachPresentation();
    transitionToUnavailable();
    return;
  }

  if (verifyPresentation())
    transitionToConfirmed();
  else
    transitionToAttaching();
}

void OverlayHubView::transitionToAttaching() {
  m_retryTimer->stop();
  m_healthTimer->stop();
  setDisplayState(DisplayState::Attaching);
  if (!m_presentationRequested) return;
  m_confirmationTimer->setInterval(confirmationTimeoutMs());
  m_confirmationTimer->start();
}

void OverlayHubView::transitionToConfirmed() {
  m_confirmationTimer->stop();
  m_retryTimer->stop();
  setDisplayState(DisplayState::Confirmed);
  if (!m_presentationRequested) return;
  const int interval = healthCheckIntervalMs();
  if (interval > 0) {
    m_healthTimer->setInterval(interval);
    m_healthTimer->start();
  }
}

void OverlayHubView::transitionToUnavailable() {
  m_confirmationTimer->stop();
  m_healthTimer->stop();
  setDisplayState(DisplayState::Unavailable);
  if (!m_presentationRequested) return;
  const int interval = retryIntervalMs();
  if (interval > 0) {
    m_retryTimer->setInterval(interval);
    m_retryTimer->start();
  }
}

void OverlayHubView::setDisplayState(const DisplayState state) {
  if (state == m_displayState) return;
  m_displayState = state;
  emit displayStateChanged();
}

}  // namespace uwf::ui
