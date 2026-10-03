# ST7735

ST7735 彩色 LCD 显示屏驱动模块（SPI），适配 0.96 / 1.8 英寸面板 / Driver module for ST7735 color LCDs over SPI (0.96" and 1.8" panels)

## 1. 模块作用 / Purpose

构造时，ST7735 把 `spi_cs`、`spi_rs` 配置为推挽输出，背光 PWM 设为 10 kHz 并以 100% 亮度使能，SPI 设为模式 0（CPOL 低、第一边沿），执行初始化序列（Hannstar 面板开启反色并使用 BGR 顺序，BOE 面板关闭反色并使用 RGB 顺序），然后清屏为白色并显示 `XRobot ST7735 Driver` 与 `xrobot.work` 两行文字。模块由构造函数完成初始化，之后由调用方通过公共函数绘图。

支持 PWM 背光亮度调节和四种显示方向（`PORTRAIT`、`PORTRAIT_ROT180`、`LANDSCAPE`、`LANDSCAPE_ROT180`）。

Upon construction, ST7735 configures `spi_cs` and `spi_rs` as push-pull outputs, starts the backlight PWM at 10 kHz with full brightness, sets SPI mode 0 (CPOL low, first edge) and runs the init sequence (Hannstar panels: display inversion on, BGR order; BOE panels: inversion off, RGB order). It then clears the screen to white and prints the two lines `XRobot ST7735 Driver` and `xrobot.work`. Initialization is completed by the constructor; drawing is then done through the public functions by the caller.

PWM backlight brightness and four display orientations are supported (`PORTRAIT`, `PORTRAIT_ROT180`, `LANDSCAPE`, `LANDSCAPE_ROT180`).

## 2. 屏幕尺寸与绘图接口 / Screen Size and Drawing API

屏幕尺寸（竖屏宽 x 高，横屏时交换）：

- `SCREEN_0_9`：80 x 160（0.96"）
- `SCREEN_1_8`、`SCREEN_1_8A`：128 x 160（1.8"）

其他 Module 通过实例调用以下公共函数，坐标以像素为单位，颜色为 RGB565（见 `ST7735::Color`）：

- `FillRect(x, y, width, height, color)`：填充矩形。
- `FillRGBRect(x, y, data, width, height)`：写入一块像素数据，每像素 2 字节，高字节在前。
- `ShowString(fg, bg, x, y, width, height, size, text)`：在区域内绘制 ASCII 字符串，`size` 为 12（6x12 字体）或 16（8x16 字体）。
- `ShowChar(fg, bg, x, y, ch, size)`：绘制单个字符。
- `SetBrightness(duty)`：设置背光占空比，范围 0.0 到 1.0。
- `GetWidth()`、`GetHeight()`：当前方向下的宽和高。

`FillRect` 与 `FillRGBRect` 的范围超出屏幕时调用被忽略。

Screen size (portrait width x height, swapped in landscape):

- `SCREEN_0_9`: 80 x 160 (0.96")
- `SCREEN_1_8`, `SCREEN_1_8A`: 128 x 160 (1.8")

Other Modules call the following public functions on the instance; coordinates are in pixels and colors are RGB565 (see `ST7735::Color`):

- `FillRect(x, y, width, height, color)`: fill a rectangle.
- `FillRGBRect(x, y, data, width, height)`: write a block of pixels, 2 bytes per pixel, high byte first.
- `ShowString(fg, bg, x, y, width, height, size, text)`: draw an ASCII string inside the area; `size` is 12 (6x12 font) or 16 (8x16 font).
- `ShowChar(fg, bg, x, y, ch, size)`: draw one character.
- `SetBrightness(duty)`: set the backlight duty cycle in the range 0.0 to 1.0.
- `GetWidth()`, `GetHeight()`: width and height in the current orientation.

A `FillRect` or `FillRGBRect` call whose area exceeds the screen is ignored.

## 3. 构造接口 / Constructor

```cpp
ST7735(LibXR::GPIO& spi_cs, LibXR::GPIO& spi_rs, LibXR::PWM& pwm,
       LibXR::SPI& spi,
       PanelType panel = ST7735::PanelType::HANNSTAR_PANEL,
       ScreenType type = ST7735::ScreenType::SCREEN_0_9,
       Orientation orientation = ST7735::Orientation::LANDSCAPE,
       PixelFormat format = ST7735::PixelFormat::FORMAT_RGB565);
```

依赖：

- `spi_cs`：SPI 片选 GPIO。
- `spi_rs`：数据/命令选择 GPIO（RS / DC）。
- `pwm`：背光 PWM。
- `spi`：屏幕所在的 SPI 总线。

配置参数：

- `panel`：面板厂商 `HANNSTAR_PANEL` 或 `BOE_PANEL`，默认 `HANNSTAR_PANEL`。
- `type`：屏幕尺寸 `SCREEN_0_9`、`SCREEN_1_8` 或 `SCREEN_1_8A`，默认 `SCREEN_0_9`。
- `orientation`：显示方向 `PORTRAIT`、`PORTRAIT_ROT180`、`LANDSCAPE` 或 `LANDSCAPE_ROT180`，默认 `LANDSCAPE`。
- `format`：写入 `COLOR_MODE` 的像素格式 `FORMAT_RGB444`、`FORMAT_RGB565` 或 `FORMAT_RGB666`，默认 `FORMAT_RGB565`。绘图函数写入 RGB565 颜色，每像素 2 字节。

Dependencies:

- `spi_cs`: the SPI chip-select GPIO.
- `spi_rs`: the data/command select GPIO (RS / DC).
- `pwm`: the backlight PWM.
- `spi`: the SPI bus the display is on.

Configuration parameters:

- `panel`: panel vendor, `HANNSTAR_PANEL` or `BOE_PANEL`, default `HANNSTAR_PANEL`.
- `type`: screen size, `SCREEN_0_9`, `SCREEN_1_8` or `SCREEN_1_8A`, default `SCREEN_0_9`.
- `orientation`: display orientation, `PORTRAIT`, `PORTRAIT_ROT180`, `LANDSCAPE` or `LANDSCAPE_ROT180`, default `LANDSCAPE`.
- `format`: pixel format written to `COLOR_MODE`, `FORMAT_RGB444`, `FORMAT_RGB565` or `FORMAT_RGB666`, default `FORMAT_RGB565`. The drawing functions write RGB565 colors, 2 bytes per pixel.

## 4. Topic

无 / None

## 5. 配置示例 / Configuration Example

`xrobot instance add xrobot-org/ST7735` 写入的实例，依赖填写为 BSP 通过 `XR_REGISTER`（硬件注册）注册的名称：

An instance written by `xrobot instance add xrobot-org/ST7735`, with the dependencies set to names registered by the BSP's `XR_REGISTER` (Registration):

```yaml
modules:
  - module: xrobot-org/ST7735
    id: st7735_0
    args:
      - spi_cs: st7735_spi_cs
      - spi_rs: st7735_spi_rs
      - pwm: st7735_pwm
      - spi: spi2
      - panel: ST7735::PanelType::HANNSTAR_PANEL
      - type: ST7735::ScreenType::SCREEN_0_9
      - orientation: ST7735::Orientation::LANDSCAPE
      - format: ST7735::PixelFormat::FORMAT_RGB565
```

## 6. 依赖与硬件 / Dependencies and Hardware

依赖：LibXR。

硬件：一块 ST7735 彩色 LCD（0.96" 或 1.8"），通过 SPI 连接，片选与数据/命令选择各用一个输出 GPIO，背光由一路 PWM 驱动。

Dependencies: LibXR.

Hardware: one ST7735 color LCD (0.96" or 1.8") on SPI, with one output GPIO each for chip select and data/command select, and the backlight driven by a PWM output.
