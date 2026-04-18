# Qt Screen Switcher

一个现代化、极简、高颜值的多屏开关工具。

## 核心功能
- **实时监测**：自动检测所有连接的显示器及其分辨率。
- **一键控制**：通过 DDC/CI 协议快速打开或关闭指定显示器背光。
- **系统托盘**：后台运行，通过托盘图标快速唤起界面或退出。
- **全局热键**：支持自定义全局快捷键（待进一步完善）。
- **极简 UI**：基于 Qt 6 开发，采用现代化卡片式布局和 QSS 样式。

## 开发环境
- **Qt 6.5+** (推荐) 或 Qt 5.15
- **C++ 17**
- **CMake 3.16+**
- **Windows 10/11** (需要 Windows SDK)

## 编译步骤
1. 打开 `CMakeLists.txt` 所在的文件夹。
2. 使用 CMake 生成工程文件：
   ```bash
   mkdir build
   cd build
   cmake ..
   ```
3. 编译：
   ```bash
   cmake --build . --config Release
   ```
4. 运行 `Release/QtScreenSwitcher.exe`。

## 项目结构
- `main.cpp`: 程序入口。
- `MainWindow.*`: 主界面逻辑与 UI 布局。
- `ScreenManager.*`: Windows API 核心封装（显示器枚举与控制）。
- `TrayIcon.*`: 系统托盘图标与菜单。
- `ShortcutManager.*`: 全局热键管理。
- `SettingsManager.*`: 配置保存（开机自启、主题等）。
- `ui/style.qss`: 现代化样式表。
