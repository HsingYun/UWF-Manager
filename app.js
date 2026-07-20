"use strict";

const translations = {
  en: {
    pageTitle: "UWF Manager — Windows Unified Write Filter management",
    pageDescription: "A modern graphical interface for Windows Unified Write Filter, with staged changes, validation, and overlay monitoring.",
    skip: "Skip to content",
    homeAria: "UWF Manager home",
    languageLabel: "Language",
    sourceAria: "View the GitHub repository",
    themeLight: "Switch to light theme",
    themeDark: "Switch to dark theme",
    heroLabel: "Windows Unified Write Filter",
    heroTitle: "A modern interface for Windows UWF",
    heroSummary: "Bring live state, next-session configuration, staged changes, validation, and overlay monitoring into one workspace—without assembling command output by hand.",
    download: "Download latest release",
    source: "Source code",
    documentationAria: "Open Microsoft UWF documentation",
    heroMeta: "Windows 10 / 11 · GPL-3.0",
    galleryLabel: "Product screenshots",
    previousScreenshot: "Previous screenshot",
    nextScreenshot: "Next screenshot",
    closeScreenshot: "Close screenshot",
    lightboxLabel: "Screenshot viewer",
    galleryAlt1: "UWF Manager main workspace",
    galleryAlt2: "Overlay files dialog",
    galleryAlt3: "Registry picker dialog",
    galleryAlt4: "About UWF Manager dialog",
    galleryAlt5: "Review and apply changes dialog",
    galleryAlt6: "UWF status shown in the Windows taskbar",
    galleryTitle1: "Main management window",
    galleryText1: "Volume, exclusion, filter, and overlay configuration.",
    galleryTitle2: "Overlay file browser",
    galleryText2: "Paged inspection, filtering, export, and commit actions.",
    galleryTitle3: "Registry selector",
    galleryText3: "Selection of registry keys and named values.",
    galleryTitle4: "Version and license information",
    galleryText4: "Project version, links, acknowledgements, and license.",
    galleryTitle5: "Pending-change review",
    galleryText5: "Planned operations and equivalent uwfmgr commands.",
    galleryTitle6: "Taskbar status",
    galleryText6: "Filter state and overlay usage in the Windows taskbar.",
    featuresTitle: "Core capabilities",
    featureStatusTitle: "Unified operational view",
    featureStatusText: "Correlate live state, next-session configuration, pending edits, and overlay usage; keep key status available from the taskbar or floating hub.",
    featureVolumeTitle: "Stage, validate, review",
    featureVolumeText: "Stage all changes without writing to the system immediately. Validate constraints and review each operation before applying the batch as a whole.",
    featureChangeTitle: "Commit changes at any time",
    featureChangeText: "Use the file and registry selectors to prepare commit operations. Confirm commit or deletion scope and capability limits before execution, then receive an itemized result.",
    featureToolsTitle: "Single-file, portable operation",
    featureToolsText: "Distributed as a single executable. It creates no application configuration files or registry entries; runtime logs remain in memory and are discarded on exit. The application itself initiates no internet connections.",
    crashExceptionLabel: "Crash diagnostics",
    crashExceptionText: "UWF Manager does not create files to persist its own state during normal operation. In the unlikely event that an unhandled exception terminates the program, it writes a .txt crash report and a .dmp minidump beside the executable for diagnostics. If that directory is not writable, it uses UWF-CrashDumps under the Windows temporary directory.",
    requirementsTitle: "System requirements",
    requirementsCompatibilityLabel: "Compatibility mode",
    requirementsCompatibilityText: "The editions below are Microsoft's officially supported configurations. If the UWF driver and WMI provider were installed by other means, UWF Manager still runs in compatibility mode and exposes the capabilities it can verify.",
    requirementSystemLabel: "Windows",
    requirementSystemValue: "Windows 10 or 11 Enterprise, Education, or IoT Enterprise, including LTSC releases",
    requirementFeatureLabel: "Feature",
    requirementFeatureValue: "Windows Unified Write Filter optional feature installed",
    requirementPermissionLabel: "Permission",
    requirementPermissionValue: "Administrator privileges required",
    poweredByQt: "Powered by Qt",
    releases: "Releases"
  },
  "zh-CN": {
    pageTitle: "UWF Manager — Windows 统一写入筛选器管理工具",
    pageDescription: "现代化 Windows 统一写入筛选器图形界面，支持变更暂存、约束校验与覆盖层监控。",
    skip: "跳到正文",
    homeAria: "UWF Manager 主页",
    languageLabel: "语言",
    sourceAria: "查看 GitHub 仓库",
    themeLight: "切换到浅色主题",
    themeDark: "切换到深色主题",
    heroLabel: "Windows 统一写入筛选器",
    heroTitle: "现代化 Windows UWF 图形界面",
    heroSummary: "在一个工作区内统一查看当前状态、下次会话配置和待应用变更，完成约束校验与覆盖层监控，无需手工拼接和核对命令输出。",
    download: "下载最新版本",
    source: "源代码",
    documentationAria: "打开微软 UWF 官方文档",
    heroMeta: "Windows 10 / 11 · GPL-3.0",
    galleryLabel: "产品截图",
    previousScreenshot: "上一张截图",
    nextScreenshot: "下一张截图",
    closeScreenshot: "关闭截图",
    lightboxLabel: "截图查看器",
    galleryAlt1: "UWF Manager 主界面",
    galleryAlt2: "覆盖层文件对话框",
    galleryAlt3: "注册表选择对话框",
    galleryAlt4: "UWF Manager 关于对话框",
    galleryAlt5: "审阅并应用变更对话框",
    galleryAlt6: "Windows 任务栏中的 UWF 状态",
    galleryTitle1: "主管理界面",
    galleryText1: "卷、排除项、筛选器和覆盖层配置。",
    galleryTitle2: "覆盖层文件浏览器",
    galleryText2: "分页查看、筛选、导出和提交操作。",
    galleryTitle3: "注册表选择器",
    galleryText3: "选择注册表键和命名值。",
    galleryTitle4: "版本与许可证信息",
    galleryText4: "项目版本、链接、致谢和许可证。",
    galleryTitle5: "待应用变更审阅",
    galleryText5: "计划执行的操作和对应 uwfmgr 命令。",
    galleryTitle6: "任务栏状态",
    galleryText6: "显示筛选器状态和覆盖层占用量。",
    featuresTitle: "核心功能",
    featureStatusTitle: "统一运行视图",
    featureStatusText: "在同一界面关联当前状态、下次会话配置、待应用变更与覆盖层占用，并通过任务栏或浮动 HUB 持续查看关键状态。",
    featureVolumeTitle: "暂存、校验、预审",
    featureVolumeText: "先暂存所有变更，不立即写入系统；待校验约束并逐项核验后整体生效。",
    featureChangeTitle: "随时提交改动",
    featureChangeText: "通过文件（注册表）选择器进行提交变更操作，执行前会确认提交或删除的范围及能力限制，执行后返回逐项执行结果。",
    featureToolsTitle: "单文件、绿色运行",
    featureToolsText: "以单个可执行文件分发，不创建软件自身的配置文件或注册表项；运行日志仅保存在内存中，退出即清除；程序自身不发起任何互联网连接。",
    crashExceptionLabel: "崩溃诊断文件说明",
    crashExceptionText: "UWF Manager 正常运行时不会为保存自身状态创建文件。仅在极少数程序因未处理异常而终止的情况下，才会在程序目录生成 .txt 崩溃报告和 .dmp 小型转储，用于故障定位。若程序目录不可写，则保存到 Windows 临时目录中的 UWF-CrashDumps 文件夹。",
    requirementsTitle: "运行要求",
    requirementsCompatibilityLabel: "兼容模式说明",
    requirementsCompatibilityText: "下列版本为 Microsoft 官方支持范围。若通过其他方式安装了 UWF 驱动及 WMI 提供程序，UWF Manager 仍会以兼容模式运行，并根据实际探测结果提供可用功能。",
    requirementSystemLabel: "系统",
    requirementSystemValue: "Windows 10 或 11 企业版、教育版或 IoT 企业版（包括 LTSC 版本）",
    requirementFeatureLabel: "功能",
    requirementFeatureValue: "已安装 Windows UWF 可选组件",
    requirementPermissionLabel: "权限",
    requirementPermissionValue: "运行程序需要管理员权限",
    poweredByQt: "Powered by Qt",
    releases: "发布版本"
  }
};

const languageButtons = [...document.querySelectorAll("[data-language]")];
const themeButton = document.querySelector("#theme-toggle");
const galleryTrack = document.querySelector("#gallery-track");
const galleryItems = [...document.querySelectorAll("[data-gallery-item]")];
const galleryCurrent = document.querySelector("#gallery-current");
const galleryCaptionTitle = document.querySelector("#gallery-caption-title");
const galleryCaptionText = document.querySelector("#gallery-caption-text");
const galleryPrevious = document.querySelector("#gallery-prev");
const galleryNext = document.querySelector("#gallery-next");
const lightbox = document.querySelector("#lightbox");
const lightboxImage = document.querySelector("#lightbox-image");
const lightboxCaption = document.querySelector("#lightbox-caption");
const infoTips = [...document.querySelectorAll(".info-tip")];

let activeGalleryIndex = 0;
let lightboxIndex = 0;
let activeLanguage = "en";
let activeTheme = null;
let galleryWheelLocked = false;
let galleryScrollFrame = null;
let lastLightboxWheelTime = Number.NEGATIVE_INFINITY;

const systemColorScheme = window.matchMedia("(prefers-color-scheme: dark)");
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");

const themeIcons = {
  light: '<circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.42 1.42M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.42-1.42M17.66 6.34l1.41-1.41"/>',
  dark: '<path d="M20.5 14.4A8 8 0 0 1 9.6 3.5 8.5 8.5 0 1 0 20.5 14.4Z"/>'
};

function normalizeLanguage(language) {
  return language.toLowerCase().startsWith("zh") ? "zh-CN" : "en";
}

function readPreference(key) {
  try {
    return localStorage.getItem(key);
  } catch {
    return null;
  }
}

function writePreference(key, value) {
  try {
    localStorage.setItem(key, value);
  } catch {
    // 本地存储不可用时，选择仅在当前页面生效。
  }
}

function updateThemeButton() {
  if (!themeButton) return;
  const targetTheme = effectiveTheme() === "dark" ? "light" : "dark";
  const labelKey = targetTheme === "light" ? "themeLight" : "themeDark";
  const label = translations[activeLanguage][labelKey];
  themeButton.setAttribute("aria-label", label);
  themeButton.setAttribute("title", label);
  themeButton.querySelector("svg").innerHTML = themeIcons[targetTheme];
}

function effectiveTheme() {
  return activeTheme ?? (systemColorScheme.matches ? "dark" : "light");
}

function updateThemeColor() {
  const color = getComputedStyle(document.documentElement).getPropertyValue("--page").trim();
  document.querySelector('meta[name="theme-color"]')?.setAttribute("content", color);
}

function setTheme(theme, persist = true) {
  activeTheme = theme === "light" || theme === "dark" ? theme : null;
  if (activeTheme) document.documentElement.dataset.theme = activeTheme;
  else delete document.documentElement.dataset.theme;
  updateThemeButton();
  if (persist && activeTheme) writePreference("uwf-site-theme", activeTheme);
  requestAnimationFrame(updateThemeColor);
}

function syncGalleryPresentation() {
  const item = galleryItems[activeGalleryIndex];
  if (!item) return;
  galleryCurrent.textContent = String(activeGalleryIndex + 1);
  galleryCaptionTitle.textContent = item.querySelector("figcaption strong")?.textContent ?? "";
  galleryCaptionText.textContent = item.querySelector("figcaption p")?.textContent ?? "";
  galleryPrevious.disabled = activeGalleryIndex === 0;
  galleryNext.disabled = activeGalleryIndex === galleryItems.length - 1;
}

function setLanguage(language, persist = true) {
  activeLanguage = normalizeLanguage(language);
  const copy = translations[activeLanguage];

  document.documentElement.lang = activeLanguage;
  document.title = copy.pageTitle;
  document.querySelector('meta[name="description"]')?.setAttribute("content", copy.pageDescription);
  document.querySelector('meta[property="og:title"]')?.setAttribute("content", copy.pageTitle);
  document.querySelector('meta[property="og:description"]')?.setAttribute("content", copy.pageDescription);
  document.querySelector('meta[property="og:locale"]')?.setAttribute("content", activeLanguage === "zh-CN" ? "zh_CN" : "en_US");
  document.querySelector('meta[property="og:locale:alternate"]')?.setAttribute("content", activeLanguage === "zh-CN" ? "en_US" : "zh_CN");
  document.querySelector('meta[property="og:image"]')?.setAttribute("content", activeLanguage === "zh-CN"
    ? "https://async.page/UWF-Manager/res/imgs/1.cn.png"
    : "https://async.page/UWF-Manager/res/imgs/1.en.png");
  document.querySelector('meta[property="og:image:alt"]')?.setAttribute("content", copy.galleryAlt1);
  document.querySelector('meta[name="twitter:title"]')?.setAttribute("content", copy.pageTitle);
  document.querySelector('meta[name="twitter:description"]')?.setAttribute("content", copy.pageDescription);
  document.querySelector('meta[name="twitter:image"]')?.setAttribute("content", activeLanguage === "zh-CN"
    ? "https://async.page/UWF-Manager/res/imgs/1.cn.png"
    : "https://async.page/UWF-Manager/res/imgs/1.en.png");
  document.querySelector('meta[name="twitter:image:alt"]')?.setAttribute("content", copy.galleryAlt1);

  document.querySelectorAll("[data-i18n]").forEach((element) => {
    const value = copy[element.dataset.i18n];
    if (value !== undefined) element.textContent = value;
  });

  document.querySelectorAll("[data-i18n-aria]").forEach((element) => {
    const value = copy[element.dataset.i18nAria];
    if (value !== undefined) element.setAttribute("aria-label", value);
  });

  document.querySelectorAll("[data-i18n-title]").forEach((element) => {
    const value = copy[element.dataset.i18nTitle];
    if (value !== undefined) element.setAttribute("title", value);
  });

  document.querySelectorAll("[data-i18n-alt]").forEach((element) => {
    const value = copy[element.dataset.i18nAlt];
    if (value !== undefined) element.setAttribute("alt", value);
  });

  document.querySelectorAll("[data-src-en][data-src-zh]").forEach((image) => {
    const languageKey = activeLanguage === "zh-CN" ? "Zh" : "En";
    image.src = image.dataset[`src${languageKey}`];
    image.setAttribute("width", image.dataset[`width${languageKey}`]);
    image.setAttribute("height", image.dataset[`height${languageKey}`]);
  });

  document.querySelectorAll("[data-href-en][data-href-zh]").forEach((link) => {
    link.href = activeLanguage === "zh-CN" ? link.dataset.hrefZh : link.dataset.hrefEn;
  });

  languageButtons.forEach((button) => {
    const selected = button.dataset.language === activeLanguage;
    button.classList.toggle("active", selected);
    button.setAttribute("aria-pressed", String(selected));
  });

  updateThemeButton();
  syncGalleryPresentation();
  if (persist) writePreference("uwf-site-language", activeLanguage);
  if (lightbox?.open) updateLightbox(lightboxIndex);
}

function scrollToGalleryItem(index, behavior = reducedMotion.matches ? "auto" : "smooth") {
  const nextIndex = Math.max(0, Math.min(galleryItems.length - 1, index));
  const item = galleryItems[nextIndex];
  if (!galleryTrack || !item) return;
  const left = galleryTrack.scrollLeft + item.getBoundingClientRect().left - galleryTrack.getBoundingClientRect().left;
  galleryTrack.scrollTo({ left, behavior });
}

function updateGalleryIndex() {
  if (!galleryTrack || !galleryItems.length) return;
  const trackLeft = galleryTrack.getBoundingClientRect().left;
  let closestDistance = Number.POSITIVE_INFINITY;
  let closestIndex = 0;

  galleryItems.forEach((item, index) => {
    const distance = Math.abs(item.getBoundingClientRect().left - trackLeft);
    if (distance < closestDistance) {
      closestDistance = distance;
      closestIndex = index;
    }
  });

  if (closestIndex === activeGalleryIndex) return;
  activeGalleryIndex = closestIndex;
  syncGalleryPresentation();
}

function updateLightbox(index) {
  const item = galleryItems[index];
  if (!item) return;
  const image = item.querySelector("img");
  const title = item.querySelector("figcaption strong")?.textContent ?? "";
  const description = item.querySelector("figcaption p")?.textContent ?? "";
  const sourceChanged = lightboxImage.src !== image.src;
  lightboxImage.classList.toggle("loading", sourceChanged);
  lightboxImage.src = image.src;
  lightboxImage.setAttribute("width", image.getAttribute("width"));
  lightboxImage.setAttribute("height", image.getAttribute("height"));
  lightboxImage.alt = image.alt;
  lightboxCaption.textContent = description ? `${title} — ${description}` : title;
  if (lightboxImage.complete) lightboxImage.classList.remove("loading");
}

function openLightbox(index) {
  if (!lightbox || typeof lightbox.showModal !== "function") return;
  lightboxIndex = Math.max(0, Math.min(galleryItems.length - 1, index));
  lastLightboxWheelTime = Number.NEGATIVE_INFINITY;
  updateLightbox(lightboxIndex);
  lightbox.showModal();
}

function moveLightbox(offset) {
  lightboxIndex = (lightboxIndex + offset + galleryItems.length) % galleryItems.length;
  updateLightbox(lightboxIndex);
}

function dominantWheelDelta(event) {
  const delta = Math.abs(event.deltaX) > Math.abs(event.deltaY) ? event.deltaX : event.deltaY;
  if (event.deltaMode === WheelEvent.DOM_DELTA_LINE) return delta * 16;
  if (event.deltaMode === WheelEvent.DOM_DELTA_PAGE) return delta * window.innerHeight;
  return delta;
}

function commitLightboxSelection() {
  activeGalleryIndex = lightboxIndex;
  scrollToGalleryItem(activeGalleryIndex, "auto");
  syncGalleryPresentation();
}

languageButtons.forEach((button) => {
  button.addEventListener("click", () => setLanguage(button.dataset.language));
});

themeButton?.addEventListener("click", () => {
  setTheme(effectiveTheme() === "dark" ? "light" : "dark");
});

galleryPrevious?.addEventListener("click", () => scrollToGalleryItem(activeGalleryIndex - 1));
galleryNext?.addEventListener("click", () => scrollToGalleryItem(activeGalleryIndex + 1));

galleryTrack?.addEventListener("scroll", () => {
  if (galleryScrollFrame !== null) return;
  galleryScrollFrame = requestAnimationFrame(() => {
    galleryScrollFrame = null;
    updateGalleryIndex();
  });
}, { passive: true });
galleryTrack?.addEventListener("wheel", (event) => {
  const delta = dominantWheelDelta(event);
  if (Math.abs(delta) < 8) return;

  const targetIndex = activeGalleryIndex + (delta > 0 ? 1 : -1);
  if (targetIndex < 0 || targetIndex >= galleryItems.length) return;

  event.preventDefault();
  if (galleryWheelLocked) return;

  galleryWheelLocked = true;
  scrollToGalleryItem(targetIndex);
  window.setTimeout(() => {
    galleryWheelLocked = false;
  }, 450);
}, { passive: false });
galleryTrack?.addEventListener("keydown", (event) => {
  if (event.key !== "ArrowLeft" && event.key !== "ArrowRight") return;
  event.preventDefault();
  scrollToGalleryItem(activeGalleryIndex + (event.key === "ArrowRight" ? 1 : -1));
});

document.querySelectorAll("[data-gallery-open]").forEach((button) => {
  button.addEventListener("click", () => openLightbox(Number(button.dataset.galleryOpen)));
});

document.querySelector(".lightbox-close")?.addEventListener("click", () => lightbox.close());
document.querySelector(".lightbox-prev")?.addEventListener("click", () => moveLightbox(-1));
document.querySelector(".lightbox-next")?.addEventListener("click", () => moveLightbox(1));

lightboxImage?.addEventListener("load", () => lightboxImage.classList.remove("loading"));
lightboxImage?.addEventListener("error", () => lightboxImage.classList.remove("loading"));

lightbox?.addEventListener("click", (event) => {
  if (event.target === lightbox) lightbox.close();
});

lightbox?.addEventListener("keydown", (event) => {
  if (event.key !== "ArrowLeft" && event.key !== "ArrowRight") return;
  event.preventDefault();
  moveLightbox(event.key === "ArrowRight" ? 1 : -1);
});

lightbox?.addEventListener("wheel", (event) => {
  const delta = dominantWheelDelta(event);
  if (Math.abs(delta) < 8) return;

  event.preventDefault();
  const now = performance.now();
  if (now - lastLightboxWheelTime < 350) return;

  lastLightboxWheelTime = now;
  moveLightbox(delta > 0 ? 1 : -1);
}, { passive: false });

lightbox?.addEventListener("close", commitLightboxSelection);

infoTips.forEach((tip) => {
  tip.addEventListener("focus", () => tip.classList.remove("tooltip-dismissed"));
  tip.addEventListener("mouseleave", () => tip.classList.remove("tooltip-dismissed"));
});

document.addEventListener("keydown", (event) => {
  if (event.key !== "Escape" || lightbox?.open) return;
  const visibleTips = infoTips.filter((tip) => tip.matches(":hover, :focus"));
  if (!visibleTips.length) return;

  event.preventDefault();
  visibleTips.forEach((tip) => {
    tip.classList.add("tooltip-dismissed");
    if (tip === document.activeElement) tip.blur();
  });
});

window.addEventListener("scroll", () => {
  document.querySelector(".site-header")?.classList.toggle("scrolled", window.scrollY > 8);
}, { passive: true });

systemColorScheme.addEventListener("change", () => {
  if (activeTheme !== null) return;
  updateThemeButton();
  updateThemeColor();
});

const savedLanguage = readPreference("uwf-site-language");
const savedTheme = readPreference("uwf-site-theme");
const systemLanguage = navigator.languages?.[0] || navigator.language || "en";
setLanguage(savedLanguage || systemLanguage, Boolean(savedLanguage));
setTheme(savedTheme, false);
syncGalleryPresentation();
