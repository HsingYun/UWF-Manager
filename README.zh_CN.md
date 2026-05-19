# UWF Manager

Windows 统一写入筛选器（Unified Write Filter, UWF）的 Qt 图形管理器：在系统自带的 `uwfmgr.exe` 命令行之外，提供一个查看和配置 UWF 状态的图形操作界面。

[English](README.md)

![截图](snapshot/snapshot.png)

## 关于 UWF

统一写入筛选器是 Windows 企业版、教育版、IoT 企业版及对应 LTSC 版本自带的扇区级写保护驱动。当一个卷被设为受保护时，所有对它的写入都会被拦截并重定向到一份 *overlay*（内存中的一块区域，或指定磁盘卷上的一个稀疏文件），底层扇区不会被改动。Overlay 在重启时丢弃，卷恢复到之前的状态。可以声明文件级和注册表项级的排除，让特定路径绕过 overlay 直接写入磁盘；也可以在重启前选择性地把 overlay 中的内容 *持久化*（提交回底层介质）。

UWF 按卷配置，状态分两套并存：*当前*会话（只读，反映驱动此刻在执行的策略）与*下次*会话（可写，重启后生效）。所有配置变更——启用保护、设置 overlay 类型与大小、增加排除项——都作用于下次会话。

UWF 是较早的增强型写保护过滤器（EWF）和基于文件的写保护过滤器（FBWF）的官方继任者，是 Windows 上构建无状态自助机、收银终端、ATM、医疗设备、机房工作站、数字标牌等场景的标准机制。

> **说明**：*服务模式*（`uwfmgr filter enable-servicing` / `disable-servicing`，用于挂起筛选器以执行 Windows Update 或计划维护）没有计划实现。

## 功能

- 筛选器开关，区分当前会话与下次会话
- Overlay 配置：类型（RAM / Disk）、最大容量、警告与紧急阈值
- 单卷保护开关、按盘符 / 按卷 ID 绑定
- 单卷的文件 / 目录排除列表
- 注册表排除列表（系统盘）
- 持久化 overlay 内容到磁盘或注册表（文件、目录、文件删除、注册表键）
- Overlay 文件条目枚举（只读）
- 导入 uwfmgr 命令——粘贴或从文件加载命令脚本，把每一行转为一条待应用变更
- 系统重启 / 关机
- 程序内置日志查看器
- 设计上无状态——程序自身不留任何痕迹：不写配置文件、不写注册表，内置日志只存在于内存环形缓冲中，进程退出即消失

## 不在计划范围

- HORM（一次休眠、多次恢复，Hibernate Once / Resume Many）
- 服务模式
- 可用空间直通、持久化 overlay、只读介质模式、交换文件创建
- 在界面上展示域机密键 / TSCAL 的持久化状态

## 运行要求

- Windows 10 / 11 企业版、教育版、IoT 企业版或对应 LTSC 版本
- 已启用 "统一写入筛选器" Windows 功能
- 管理员权限——应用变更时需要

注：程序不会因为这些条件不满足而拒绝启动。

## 构建

- C++20
- CMake ≥ 3.16
- Qt 6（Widgets、Svg、LinguistTools）
- Windows 目标编译器（MSVC、clang-cl、MSYS2 clang64、mingw-w64）

```sh
cmake -S . -B build
cmake --build build --config Release
```

## 由 AI 生成的代码

本仓库中的大部分源码由 AI 编程助手在人工审阅与指导下生成。使用时请相应对待：代码可运行，但任何非显而易见的行为，请在依赖前对照源码、WMI 文档或运行中的系统加以验证。

## 许可

GNU General Public License v3.0，见 [LICENSE](LICENSE)。
