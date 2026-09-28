# ST7735

ST7735 彩色 LCD 显示屏驱动模块（SPI），适配 0.96 / 1.8 英寸面板，支持 PWM 背光亮度调节和四种显示方向。
Driver module for ST7735 color LCDs over SPI (0.96" and 1.8" panels), with PWM
backlight brightness and four display orientations.

构造时模块将 `spi_cs`、`spi_rs` 配置为推挽输出，背光 PWM 设为 10 kHz 并以 100% 亮度
使能，SPI 设为模式 0（CPOL 低、第一边沿），执行初始化序列（Hannstar 面板开启反色并
使用 BGR 顺序，BOE 面板关闭反色并使用 RGB 顺序），然后清屏为白色并显示
`XRobot ST7735 Driver` 与 `xrobot.work` 两行文字。模块没有线程和 RamFS 命令。

During construction the module configures `spi_cs` and `spi_rs` as push-pull
outputs, starts the backlight PWM at 10 kHz with full brightness, sets SPI mode
0 (CPOL low, first edge) and runs the init sequence (Hannstar panels: display
inversion on, BGR order; BOE panels: inversion off, RGB order). It then clears
the screen to white and prints `XRobot ST7735 Driver` and `xrobot.work`. The
module has no thread and no RamFS command.

屏幕尺寸 / Screen size（竖屏宽 x 高 / portrait width x height，横屏时交换 / swapped in landscape）:

- `SCREEN_0_9`：80 x 160（0.96"）
- `SCREEN_1_8`、`SCREEN_1_8A`：128 x 160（1.8"）

### 绘图接口 / Drawing API

其他模块可以通过实例调用以下公共函数；坐标以像素为单位，颜色为 RGB565（见 `ST7735::Color`）。
Other Modules can call these public functions on the instance; coordinates are
in pixels, colors are RGB565 (see `ST7735::Color`).

- `FillRect(x, y, width, height, color)`：填充矩形。/ Fill a rectangle.
- `FillRGBRect(x, y, data, width, height)`：写入一块像素数据（每像素 2 字节，高字节在前）。
  / Write a block of pixels (2 bytes per pixel, high byte first).
- `ShowString(fg, bg, x, y, width, height, size, text)`：在区域内绘制 ASCII 字符串；`size` 为 12（6x12 字体）或 16（8x16 字体）。
  / Draw an ASCII string inside the area; `size` 12 (6x12 font) or 16 (8x16 font).
- `ShowChar(fg, bg, x, y, ch, size)`：绘制单个字符。/ Draw one character.
- `SetBrightness(duty)`：背光占空比 0.0..1.0。/ Backlight duty cycle 0.0..1.0.
- `GetWidth()`、`GetHeight()`：当前方向下的宽高。/ Width and height in the current orientation.

超出屏幕范围的 `FillRect` / `FillRGBRect` 调用会被忽略。
`FillRect` / `FillRGBRect` calls that exceed the screen are ignored.

## 依赖 / Dependencies

无其他模块依赖，仅使用 LibXR。
No other Modules; LibXR only.

## 构造接口 / Constructor

```cpp
ST7735(LibXR::GPIO& spi_cs, LibXR::GPIO& spi_rs, LibXR::PWM& pwm,
       LibXR::SPI& spi,
       PanelType panel = ST7735::PanelType::HANNSTAR_PANEL,
       ScreenType type = ST7735::ScreenType::SCREEN_0_9,
       Orientation orientation = ST7735::Orientation::LANDSCAPE,
       PixelFormat format = ST7735::PixelFormat::FORMAT_RGB565);
```

依赖 / Dependencies:

- `spi_cs`：SPI 片选 GPIO。/ SPI chip-select GPIO.
- `spi_rs`：数据/命令选择 GPIO（RS / DC）。/ Data/command select GPIO (RS / DC).
- `pwm`：背光 PWM。/ Backlight PWM.
- `spi`：屏幕所在的 SPI 总线。/ The SPI bus the display is on.

配置 / Configuration:

- `panel`：面板厂商 `HANNSTAR_PANEL` 或 `BOE_PANEL`，默认 `HANNSTAR_PANEL`。
  / Panel vendor, `HANNSTAR_PANEL` or `BOE_PANEL`, default `HANNSTAR_PANEL`.
- `type`：屏幕尺寸 `SCREEN_0_9`、`SCREEN_1_8`、`SCREEN_1_8A`，默认 `SCREEN_0_9`。
  / Screen size, default `SCREEN_0_9`.
- `orientation`：`PORTRAIT`、`PORTRAIT_ROT180`、`LANDSCAPE`、`LANDSCAPE_ROT180`，默认 `LANDSCAPE`。
  / Display orientation, default `LANDSCAPE`.
- `format`：写入 `COLOR_MODE` 的像素格式 `FORMAT_RGB444`、`FORMAT_RGB565`、`FORMAT_RGB666`，默认 `FORMAT_RGB565`；
  绘图函数按每像素 2 字节写入，因此应使用 RGB565。
  / Pixel format written to `COLOR_MODE`, default `FORMAT_RGB565`; the drawing
  functions write 2 bytes per pixel, so use RGB565.

## 使用 / Use

```sh
xrobot module add xrobot-org/ST7735
xrobot setup
xrobot instance add xrobot-org/ST7735
```

`xrobot instance add` 在 `User/xrobot.yaml` 中写入一个实例，依赖项留空，默认值按源码写出；
把依赖项填为 BSP 中用 `XR_REGISTER` 注册的对象名：
`xrobot instance add` writes an instance to `User/xrobot.yaml` with empty
dependencies and the source defaults; set the dependencies to the names of
objects the BSP registers with `XR_REGISTER`:

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

BSP 侧 / BSP side:

```cpp
XR_REGISTER(st7735_spi_cs, LibXR::GPIO);
XR_REGISTER(st7735_spi_rs, LibXR::GPIO);
XR_REGISTER(st7735_pwm, LibXR::PWM);
XR_REGISTER(spi2, LibXR::SPI);
```

填好后再次运行 `xrobot setup`，生成 `User/xrobot_main.hpp`。
Run `xrobot setup` again to generate `User/xrobot_main.hpp`.

`xrobot module show .`（在本仓库中）或 `xrobot module show Modules/xrobot-org/ST7735`
（在 BSP 中）打印当前的构造函数。
`xrobot module show .` in this repository, or
`xrobot module show Modules/xrobot-org/ST7735` in a BSP, prints the current
constructor.
