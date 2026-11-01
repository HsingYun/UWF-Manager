// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/MaintenanceValidationText.h"

#include "application/localization/HormText.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
QString maintenanceValidationText(const app::MaintenanceValidationIssue& issue) {
  if (const auto* horm = std::get_if<app::HormIssue>(&issue)) return app::hormIssueText(*horm);
  switch (std::get<app::MaintenancePlanIssue>(issue)) {
    case app::MaintenancePlanIssue::HormActive:
      return PresentationText::tr("HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.");
    case app::MaintenancePlanIssue::UnprotectAndHormConflict:
      return PresentationText::tr("Unprotect all and enabling HORM cannot be applied together.");
    case app::MaintenancePlanIssue::FullCommitAndHormConflict:
      return PresentationText::tr("Full volume commit and enabling HORM cannot be applied together.");
    case app::MaintenancePlanIssue::StorageAndHormConflict:
      return PresentationText::tr("Configuring the storage location and enabling HORM cannot be applied together.");
    case app::MaintenancePlanIssue::StorageAndFullCommitConflict:
      return PresentationText::tr("Configuring the storage location and full volume commit require different current UWF modes.");
    case app::MaintenancePlanIssue::StorageVolumeRequired:
      return PresentationText::tr("Choose a storage volume before applying.");
    case app::MaintenancePlanIssue::StorageAndRamConflict:
      return PresentationText::tr("Configuring a disk storage location conflicts with a requested RAM overlay.");
    case app::MaintenancePlanIssue::BatchProtectionUnsupported:
      return PresentationText::tr("Batch volume protection is not supported.");
    case app::MaintenancePlanIssue::StorageUnsupported:
      return PresentationText::tr("Configuring the storage location is not available on this system.");
    case app::MaintenancePlanIssue::StorageRequiresFilterDisabled:
      return PresentationText::tr("Disable UWF and restart before configuring the storage location.");
    case app::MaintenancePlanIssue::FullCommitUnsupported:
      return PresentationText::tr("Full volume commit is not available on this system.");
    case app::MaintenancePlanIssue::FullCommitRequiresReadOnlyMedia:
      return PresentationText::tr("Full volume commit is available only while Read-Only Media mode is active.");
    case app::MaintenancePlanIssue::FullCommitFilterChange:
      return PresentationText::tr("Apply the filter change and restart before requesting full volume commit.");
    case app::MaintenancePlanIssue::StorageRequiresHormDisabled:
      return PresentationText::tr("Disable HORM and restart before configuring the storage location.");
    case app::MaintenancePlanIssue::FullCommitRequiresHormDisabled:
      return PresentationText::tr("Disable HORM and restart before using full volume commit.");
    case app::MaintenancePlanIssue::ResultUnconfirmed:
      return PresentationText::tr("The result could not be confirmed");
    case app::MaintenancePlanIssue::Canceled:
      return PresentationText::tr("Canceled");
    case app::MaintenancePlanIssue::UnknownFailure:
      return PresentationText::tr("The operation failed with an unknown error.");
    case app::MaintenancePlanIssue::HormStateUnavailable:
      return PresentationText::tr("The HORM state is unknown. Refresh before continuing.");
    case app::MaintenancePlanIssue::FullCommitStateUnavailable:
    case app::MaintenancePlanIssue::StorageStateUnavailable:
    case app::MaintenancePlanIssue::StorageCapacityUnavailable:
    case app::MaintenancePlanIssue::StorageInventoryUnavailable:
      return PresentationText::tr("The current UWF state could not be read. Refresh before continuing.");
    case app::MaintenancePlanIssue::StorageVolumeUnavailable:
      return PresentationText::tr("The reviewed storage volume is no longer available. Return to editing and choose another volume.");
    case app::MaintenancePlanIssue::Count:
      break;
  }
  return {};
}
}  // namespace uwf::presentation
