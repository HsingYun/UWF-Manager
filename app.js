(() => {
  "use strict";

  const root = document.documentElement;
  const languageButton = document.querySelector("#language-toggle");
  const themeButton = document.querySelector("#theme-toggle");
  const menuButton = document.querySelector("#menu-toggle");
  const mobileNav = document.querySelector("#mobile-nav");
  const dialog = document.querySelector("#screenshot-dialog");
  const dialogImage = document.querySelector("#dialog-image");
  const dialogCaption = document.querySelector("#dialog-caption");
  const dialogClose = document.querySelector("#dialog-close");

  // English stays in the HTML, so content edits have a single source of truth.
  const english = {};
  document.querySelectorAll("[data-i18n]").forEach((element) => {
    english[element.dataset.i18n] = element.textContent;
  });
  document.querySelectorAll("[data-i18n-aria]").forEach((element) => {
    english[element.dataset.i18nAria] = element.getAttribute("aria-label");
  });
  Object.assign(english, {
    pageTitle: document.title,
    pageDescription: document.querySelector('meta[name="description"]').content,
    openNavigation: menuButton.getAttribute("aria-label"),
    closeNavigation: "Close navigation",
    switchChinese: languageButton.getAttribute("aria-label"),
    switchEnglish: "Switch to English",
    themeLight: "Switch to light theme",
    themeDark: themeButton.getAttribute("aria-label"),
  });

  const chinese = {
    skip: "跳到正文",
    home: "UWF Manager 主页",
    navigation: "主导航",
    navConfiguration: "配置管理",
    navPersistence: "持久化",
    navRequirements: "系统要求",
    navDownload: "下载",
    mobileNavigation: "移动导航",
    productPosition: "Windows 统一写入筛选器管理工具",
    heroDescription:
      "管理卷保护、覆盖层与排除项，分别查看当前会话状态和下次会话配置。",
    downloadWindows: "下载 Windows 版本",
    viewSource: "查看源码",
    productFacts: "产品信息",
    specPlatform: "平台",
    specInterfaces: "接口",
    specLanguages: "界面语言",
    specLicense: "开源许可",
    enlargeScreenshot: "放大管理工作区截图",
    workspaceAlt: "UWF Manager 管理工作区，展示当前会话和下次会话的保护设置",
    workspaceCaption: "界面使用示例配置；点击截图可放大查看。",
    featureNavigation: "功能分区",
    navWorkflow: "导入与审阅",
    navMonitoring: "运行监控",
    configurationTitle: "覆盖层配置与 HORM",
    configurationDescription: "配置覆盖层、检查文件，并验证 HORM 启用条件。",
    overlayTitle: "覆盖层设置",
    overlayDescription:
      "设置覆盖层类型、容量、占用阈值、持久化选项及支持的运行模式。",
    enlargeOverlay: "放大覆盖层设置截图",
    overlayAlt: "UWF Manager 的高级覆盖层设置",
    overlayFilesTitle: "覆盖层文件浏览",
    overlayFilesDescription:
      "浏览并筛选覆盖层文件、导出列表，并使用可用的提交操作。",
    enlargeOverlayFiles: "放大覆盖层文件浏览截图",
    overlayFilesAlt: "覆盖层文件浏览器，包含路径筛选、文件详情和列表导出操作",
    hormTitle: "HORM：一次休眠，多次恢复",
    hormDescription:
      "在后续启动时恢复至同一休眠状态。检查 HORM 前置条件，将启用或禁用操作加入配置草稿，经审阅后应用。",
    hormPreparation:
      "检查项目包括当前 UWF 保护、RAM 只读媒体配置、S4 完整休眠，以及符合 UWF 条件的已挂载固定卷保护。启用 HORM 后，仍需休眠一次以保存基准状态。",
    hormRestrictions:
      "HORM 开启时，配置修改、单项提交、自动暂存提交和整卷提交均不可用。维护前需关闭 HORM 并重启。",
    enlargeHorm: "放大 HORM 前置条件检查截图",
    hormAlt:
      "HORM 配置对话框，显示 UWF、RAM 只读媒体模式、完整休眠与固定卷保护检查",
    workflowTitle: "配置导入与审阅",
    workflowDescription:
      "准备配置草稿、审阅计划操作，然后应用变更并查看执行结果。",
    importTitle: "命令导入",
    importDescription:
      "将支持的 uwfmgr 命令导入配置草稿并查看校验结果。导入不会直接执行命令。",
    enlargeImport: "放大命令导入截图",
    importAlt: "命令导入对话框，包含 uwfmgr 命令及逐项校验结果",
    reviewTitle: "配置审阅",
    reviewDescription:
      "应用配置前，审阅待应用变更、执行条件及可导出的 uwfmgr 命令。",
    enlargeReview: "放大变更审阅截图",
    reviewAlt: "应用变更之前审阅配置计划",
    persistenceTitle: "文件暂存与增强模式",
    persistenceDescription:
      "维护待提交文件列表，并选择关机或重启前的提交方式。",
    stagingTitle: "文件暂存",
    stagingDescription: "暂存文件和目录，在通过应用发起安全关机或重启前提交。",
    enlargeStaging: "放大文件暂存截图",
    stagingAlt: "文件暂存列表，包含已保存路径、状态和文件管理操作",
    enhancedTitle: "可选的增强模式",
    enhancedDescription:
      "通过 Windows 服务，在系统关机或重启前提交符合条件的暂存文件。",
    enlargeEnhanced: "放大增强模式截图",
    enhancedAlt: "增强模式对话框，显示 Windows 服务状态和自动提交设置",
    commitTitle: "选择性提交",
    commitDescription: "提交选定的文件或注册表项目，并逐项查看执行结果。",
    commitConditions:
      "可用操作取决于当前保护状态和 UWF 配置。执行前请确认所选范围。",
    enlargeCommit: "放大选择性提交截图",
    commitAlt: "提交确认对话框，显示所选范围和准备结果",
    monitoringTitle: "运行状态",
    hubDescription: "在任务栏或浮动面板中查看筛选器状态与覆盖层占用。",
    enlargeHub: "放大浮动状态面板截图",
    hubAlt: "浮动面板显示 UWF 筛选器状态和覆盖层占用",
    requirementsTitle: "系统要求",
    osLabel: "操作系统",
    osValue:
      "Windows 10 / 11 企业版、教育版或 IoT 企业版，包括适用的 LTSC 版本。",
    featureLabel: "Windows 功能",
    featureValue: "已安装并启用 Windows 可选功能中的统一写入筛选器。",
    permissionsLabel: "权限",
    permissionsValue: "图形界面及受保护的命令行操作需要管理员权限。",
    logsTitle: "运行日志",
    logsDescription: "查看带时间戳的日志条目，检查操作结果并排查问题。",
    enlargeLogs: "放大运行日志截图",
    logsAlt: "运行日志查看器，包含带时间戳的消息和诊断信息",
    factOpen: "GPL-3.0 开源许可",
    footerContact: "联系作者",
    projectLinks: "项目链接",
    footerSource: "源代码",
    footerIssues: "问题反馈",
    footerReleases: "发行版本",
    screenshotDialog: "产品截图",
    closeScreenshot: "关闭截图",
    pageTitle: "UWF Manager — Windows 统一写入筛选器管理工具",
    pageDescription:
      "Windows 统一写入筛选器配置与管理工具，支持卷保护、覆盖层、排除项和变更审阅。",
    openNavigation: "打开导航",
    closeNavigation: "关闭导航",
    switchChinese: "切换到中文",
    switchEnglish: "切换到英语",
    themeLight: "切换到浅色主题",
    themeDark: "切换到深色主题",
  };

  const screenshots = [...document.querySelectorAll("img[data-screenshot]")];
  const views = {};
  for (const image of screenshots) {
    views[image.dataset.screenshot] = {
      width: Number(image.getAttribute("width")),
      height: Number(image.getAttribute("height")),
      caption: image.dataset.caption,
      altKey: image.dataset.i18nAlt,
      radius: Number(image.dataset.radius ?? 20),
      inset: Number(image.dataset.inset ?? 0),
    };
    english[image.dataset.i18nAlt] = image.alt;
  }

  function readPreference(key) {
    try {
      return localStorage.getItem(key);
    } catch {
      return null;
    }
  }

  function savePreference(key, value) {
    try {
      localStorage.setItem(key, value);
    } catch {
      /* The page also works without storage access. */
    }
  }

  const savedLanguage = readPreference("uwf-site-language");
  const savedTheme = readPreference("uwf-site-theme");
  let language =
    savedLanguage === "en" || savedLanguage === "zh-CN"
      ? savedLanguage
      : root.lang === "zh-CN"
        ? "zh-CN"
        : "en";
  let theme =
    savedTheme === "light" || savedTheme === "dark"
      ? savedTheme
      : root.dataset.theme === "dark"
        ? "dark"
        : "light";
  let dialogView = "workspace";
  let dialogOpener = null;
  let previousOverflow = "";

  function text(key) {
    return language === "zh-CN" ? (chinese[key] ?? english[key]) : english[key];
  }

  function screenshotPath(view) {
    return `res/screenshots/${view}.${language === "zh-CN" ? "zh" : "en"}.${theme}.png`;
  }

  function updateScreenshot(image, view) {
    image.dataset.screenshot = view;
    const details = views[view];
    const source = screenshotPath(view);
    const changed = image.getAttribute("src") !== source;
    if (changed) image.src = source;
    setScreenshotSize(
      image,
      (!changed && image.naturalWidth) || details.width,
      (!changed && image.naturalHeight) || details.height,
    );
    image.style.setProperty("--capture-radius", details.radius);
    image.style.setProperty("--capture-inset", details.inset);
    image.alt = text(details.altKey);
  }

  function setScreenshotSize(image, width, height) {
    image.width = width;
    image.height = height;
    image.style.setProperty("--capture-width", width);
    image.style.setProperty("--capture-height", height);
  }

  function updateWindowPair(pair) {
    const columns = [...pair.querySelectorAll("img[data-screenshot]")].map(
      (image) =>
        `${(Number(image.getAttribute("width")) / Number(image.getAttribute("height"))).toFixed(6)}fr`,
    );
    pair
      .closest(".feature-section")
      .style.setProperty("--window-columns", columns.join(" "));
  }

  function updateScreenshots() {
    screenshots.forEach((image) =>
      updateScreenshot(image, image.dataset.screenshot),
    );
    document.querySelectorAll(".window-pair").forEach(updateWindowPair);
    if (dialog.open) {
      updateScreenshot(dialogImage, dialogView);
      dialogCaption.textContent = text(views[dialogView].caption);
    }
  }

  function updateControls() {
    languageButton.textContent = language === "zh-CN" ? "EN" : "中文";
    languageButton.lang = language === "zh-CN" ? "en" : "zh-CN";
    languageButton.setAttribute(
      "aria-label",
      text(language === "zh-CN" ? "switchEnglish" : "switchChinese"),
    );
    themeButton.setAttribute(
      "aria-label",
      text(theme === "dark" ? "themeLight" : "themeDark"),
    );
    menuButton.setAttribute(
      "aria-label",
      text(mobileNav.hidden ? "openNavigation" : "closeNavigation"),
    );
  }

  function updateMetadata() {
    document.title = text("pageTitle");
    const imageUrl = new URL(
      screenshotPath("workspace"),
      document.querySelector('link[rel="canonical"]').href,
    ).href;
    const metadata = {
      'meta[name="description"]': text("pageDescription"),
      'meta[name="theme-color"]': getComputedStyle(root)
        .getPropertyValue("--background")
        .trim(),
      'meta[property="og:title"]': text("pageTitle"),
      'meta[property="og:description"]': text("pageDescription"),
      'meta[property="og:locale"]': language === "zh-CN" ? "zh_CN" : "en_US",
      'meta[property="og:locale:alternate"]':
        language === "zh-CN" ? "en_US" : "zh_CN",
      'meta[property="og:image"]': imageUrl,
      'meta[property="og:image:alt"]': text(views.workspace.altKey),
      'meta[name="twitter:title"]': text("pageTitle"),
      'meta[name="twitter:description"]': text("pageDescription"),
      'meta[name="twitter:image"]': imageUrl,
    };
    for (const [selector, value] of Object.entries(metadata)) {
      document.querySelector(selector).setAttribute("content", value);
    }
  }

  function applyLanguage() {
    root.lang = language;
    document.querySelectorAll("[data-i18n]").forEach((element) => {
      element.textContent = text(element.dataset.i18n);
    });
    document.querySelectorAll("[data-i18n-aria]").forEach((element) => {
      element.setAttribute("aria-label", text(element.dataset.i18nAria));
    });
    updateControls();
    updateScreenshots();
    updateMetadata();
  }

  function setMenuOpen(open, returnFocus = false) {
    mobileNav.hidden = !open;
    menuButton.setAttribute("aria-expanded", String(open));
    menuButton
      .querySelector("use")
      .setAttribute("href", open ? "#i-close" : "#i-menu");
    updateControls();
    if (returnFocus) menuButton.focus();
  }

  languageButton.addEventListener("click", () => {
    language = language === "en" ? "zh-CN" : "en";
    savePreference("uwf-site-language", language);
    applyLanguage();
  });
  themeButton.addEventListener("click", () => {
    theme = theme === "dark" ? "light" : "dark";
    root.dataset.theme = theme;
    savePreference("uwf-site-theme", theme);
    updateControls();
    updateScreenshots();
    updateMetadata();
  });

  menuButton.addEventListener("click", () => setMenuOpen(mobileNav.hidden));
  mobileNav.querySelectorAll("a").forEach((link) => {
    link.addEventListener("click", () => setMenuOpen(false));
  });
  document.addEventListener("click", (event) => {
    if (
      !mobileNav.hidden &&
      !mobileNav.contains(event.target) &&
      !menuButton.contains(event.target)
    )
      setMenuOpen(false);
  });
  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape" && !mobileNav.hidden && !dialog.open) {
      event.preventDefault();
      setMenuOpen(false, true);
    }
  });
  window.addEventListener("resize", () => {
    if (!mobileNav.hidden && getComputedStyle(menuButton).display === "none")
      setMenuOpen(false);
  });

  for (const image of [...screenshots, dialogImage]) {
    image.addEventListener("load", () => {
      setScreenshotSize(image, image.naturalWidth, image.naturalHeight);
      const pair = image.closest(".window-pair");
      if (pair) updateWindowPair(pair);
    });
  }

  document.querySelectorAll("[data-lightbox]").forEach((button) => {
    button.addEventListener("click", () => {
      dialogView = button.dataset.lightbox;
      dialogOpener = button;
      updateScreenshot(dialogImage, dialogView);
      dialogCaption.textContent = text(views[dialogView].caption);
      dialog.showModal();
      previousOverflow = root.style.overflow;
      root.style.overflow = "hidden";
      dialogClose.focus({ preventScroll: true });
    });
  });
  dialogClose.addEventListener("click", () => dialog.close());
  // Escape and focus containment are supplied by the native modal dialog.
  dialog.addEventListener("click", (event) => {
    if (event.target !== dialog) return;
    const bounds = dialog.getBoundingClientRect();
    if (
      event.clientX < bounds.left ||
      event.clientX > bounds.right ||
      event.clientY < bounds.top ||
      event.clientY > bounds.bottom
    )
      dialog.close();
  });
  dialog.addEventListener("close", () => {
    root.style.overflow = previousOverflow;
    if (dialogOpener?.isConnected) dialogOpener.focus({ preventScroll: true });
    dialogOpener = null;
  });

  root.dataset.theme = theme;
  applyLanguage();
})();
