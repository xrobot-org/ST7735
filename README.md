# ST7735

ST7735 显示屏驱动  
ST7735 display driver

---

## 简介 / Overview

本模块为 ST7735 彩色 LCD 显示屏驱动，适配 0.96/1.8 英寸多种面板，支持 SPI 通信，支持亮度调节和多种显示方向配置。  
This module provides a flexible and high-performance driver for ST7735 color LCDs (0.96", 1.8", various panels), supporting SPI interface, brightness control and multiple display orientations.

---

## 所需硬件 / Required Hardware

- `st7735_spi`：SPI 接口 / SPI interface
- `st7735_spi_cs`：SPI 片选 / SPI chip select
- `st7735_spi_rs`：数据/命令选择引脚 / Data/Command selection pin
- `st7735_pwm`：背光 PWM 控制 / Backlight PWM control

---

## 构造参数 / Constructor Arguments

| 参数 (name) | 类型 (type)           | 说明 (description)  | 默认值 (default) |
| ----------- | --------------------- | ------------------- | ---------------- |
| panel       | `ST7735::PanelType`   | 面板型号/厂商 Panel | `HANNSTAR_PANEL` |
| type        | `ST7735::ScreenType`  | 屏幕尺寸型号        | `SCREEN_0_9`     |
| orientation | `ST7735::Orientation` | 屏幕方向            | `LANDSCAPE`      |
| format      | `ST7735::PixelFormat` | 像素格式            | `FORMAT_RGB565`  |

> 可在构造时自由指定这些参数。默认适配 0.96寸 Hannstar 横屏 RGB565 格式。
> You can specify these parameters when constructing. The default is to adapt to a 0.96-inch Hannstar horizontal screen in RGB565 format.

---

## 依赖 / Depends

- None
