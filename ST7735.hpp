#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: ST7735 彩色 LCD 显示屏驱动模块（SPI），适配 0.96 / 1.8 英寸面板 / Driver module for ST7735 color LCDs over SPI (0.96" and 1.8" panels)
depends: []
=== END MANIFEST === */
// clang-format on

#include <cstdint>
#include <memory>

#include "font.h"
#include "gpio.hpp"
#include "libxr_type.hpp"
#include "pwm.hpp"
#include "semaphore.hpp"
#include "spi.hpp"
#include "thread.hpp"

/**
 * @brief ST7735 彩色 LCD 驱动，通过 SPI 写屏，提供填充、位图与字符串绘制。
 *        Driver for ST7735 color LCDs; writes the screen over SPI and provides fill,
 *        bitmap and string drawing.
 */
class ST7735
{
 public:
  /**
   * @brief ST7735 命令寄存器地址。
   *        ST7735 command register addresses.
   */
  enum Command : uint8_t
  {
    NOP = 0x00,
    SW_RESET = 0x01,
    READ_ID = 0x04,
    READ_STATUS = 0x09,
    READ_POWER_MODE = 0x0A,
    READ_MADCTL = 0x0B,
    READ_PIXEL_FORMAT = 0x0C,
    READ_IMAGE_MODE = 0x0D,
    READ_SIGNAL_MODE = 0x0E,
    SLEEP_IN = 0x10,
    SLEEP_OUT = 0x11,
    PARTIAL_DISPLAY_ON = 0x12,
    NORMAL_DISPLAY_OFF = 0x13,
    DISPLAY_INVERSION_OFF = 0x20,
    DISPLAY_INVERSION_ON = 0x21,
    GAMMA_SET = 0x26,
    DISPLAY_OFF = 0x28,
    DISPLAY_ON = 0x29,
    CASET = 0x2A,
    RASET = 0x2B,
    WRITE_RAM = 0x2C,
    RGBSET = 0x2D,
    READ_RAM = 0x2E,
    PTLAR = 0x30,
    TE_LINE_OFF = 0x34,
    TE_LINE_ON = 0x35,
    MADCTL = 0x36,
    IDLE_MODE_OFF = 0x38,
    IDLE_MODE_ON = 0x39,
    COLOR_MODE = 0x3A,
    FRAME_RATE_CTRL1 = 0xB1,
    FRAME_RATE_CTRL2 = 0xB2,
    FRAME_RATE_CTRL3 = 0xB3,
    FRAME_INVERSION_CTRL = 0xB4,
    DISPLAY_SETTING = 0xB6,
    PWR_CTRL1 = 0xC0,
    PWR_CTRL2 = 0xC1,
    PWR_CTRL3 = 0xC2,
    PWR_CTRL4 = 0xC3,
    PWR_CTRL5 = 0xC4,
    VCOMH_VCOML_CTRL1 = 0xC5,
    VMOF_CTRL = 0xC7,
    WRID2 = 0xD1,
    WRID3 = 0xD2,
    NV_CTRL1 = 0xD9,
    READ_ID1 = 0xDA,
    READ_ID2 = 0xDB,
    READ_ID3 = 0xDC,
    NV_CTRL2 = 0xDE,
    NV_CTRL3 = 0xDF,
    PV_GAMMA_CTRL = 0xE0,
    NV_GAMMA_CTRL = 0xE1,
    EXT_CTRL = 0xF0,
    PWR_CTRL6 = 0xFC,
    VCOM4_LEVEL = 0xFF
  };

  /**
   * @brief 屏幕尺寸。
   *        Screen size.
   */
  enum ScreenType : uint8_t
  {
    SCREEN_1_8 = 0x00,  ///< 1.8 英寸，128 x 160 1.8 inch, 128 x 160
    SCREEN_0_9 = 0x01,  ///< 0.96 英寸，80 x 160 0.96 inch, 80 x 160
    SCREEN_1_8A = 0x02  ///< 1.8 英寸变体，128 x 160 1.8 inch variant, 128 x 160
  };

  /**
   * @brief 面板厂商。
   *        Panel vendor.
   */
  enum PanelType : uint8_t
  {
    HANNSTAR_PANEL = 0x00,  ///< Hannstar 面板 Hannstar panel
    BOE_PANEL = 0x01        ///< BOE 面板 BOE panel
  };

  /**
   * @brief 显示方向。
   *        Display orientation.
   */
  enum Orientation : uint8_t
  {
    PORTRAIT = 0x00,         ///< 竖屏 Portrait
    PORTRAIT_ROT180 = 0x01,  ///< 竖屏旋转 180 度 Portrait rotated by 180 degrees
    LANDSCAPE = 0x02,        ///< 横屏 Landscape
    LANDSCAPE_ROT180 = 0x03  ///< 横屏旋转 180 度 Landscape rotated by 180 degrees
  };

  /**
   * @brief 像素格式，枚举值等于 COLOR_MODE 寄存器取值。
   *        Pixel format; the enumerator value equals the COLOR_MODE register value.
   */
  enum PixelFormat : uint8_t
  {
    FORMAT_RGB444 = 0x03,           ///< 12 位 RGB 12-bit RGB
    FORMAT_RGB565 = 0x05,           ///< 16 位 RGB 16-bit RGB
    FORMAT_RGB666 = 0x06,           ///< 18 位 RGB 18-bit RGB
    FORMAT_DEFAULT = FORMAT_RGB565  ///< 默认格式 Default format
  };

  static constexpr uint16_t WIDTH_1_8 = 128;   ///< 1.8 英寸屏宽 1.8 inch width
  static constexpr uint16_t HEIGHT_1_8 = 160;  ///< 1.8 英寸屏高 1.8 inch height
  static constexpr uint16_t WIDTH_0_9 = 80;    ///< 0.96 英寸屏宽 0.96 inch width
  static constexpr uint16_t HEIGHT_0_9 = 160;  ///< 0.96 英寸屏高 0.96 inch height

  /**
   * @brief 方向到 MADCTL 取值的对照表，每个方向两列。
   *        MADCTL values for each orientation, two columns per orientation.
   */
  static constexpr uint32_t OrientationTab[4][2] = {
      {0x40U, 0xC0U}, {0x80U, 0x00U}, {0x20U, 0x60U}, {0xE0U, 0xA0U}};

  /**
   * @brief 颜色分量顺序，枚举值等于 MADCTL 中的 RGB/BGR 位。
   *        Color component order; the enumerator value equals the RGB/BGR bit of MADCTL.
   */
  enum RGBOrder : uint8_t
  {
    LCD_RGB = 0x00,  ///< RGB 顺序 RGB order
    LCD_BGR = 0x08   ///< BGR 顺序 BGR order
  };

  /**
   * @brief 常用颜色，RGB565 取值。
   *        Common colors as RGB565 values.
   */
  enum Color : uint16_t
  {
    WHITE = 0xFFFF,
    BLACK = 0x0000,
    BLUE = 0x001F,
    BRED = 0xF81F,
    GRED = 0xFFE0,
    GBLUE = 0x07FF,
    RED = 0xF800,
    MAGENTA = 0xF81F,
    GREEN = 0x07E0,
    CYAN = 0x7FFF,
    YELLOW = 0xFFE0,
    BROWN = 0xBC40,
    BRRED = 0xFC07,
    GRAY = 0x8430,
    DARKBLUE = 0x01CF,
    LIGHTBLUE = 0x7D7C,
    GRAYBLUE = 0x5458
  };

  /**
   * @brief 构造 ST7735：配置 GPIO、背光 PWM 与 SPI，执行初始化序列并清屏显示启动文字。
   *        Construct ST7735: configure the GPIOs, the backlight PWM and the SPI, run the
   *        init sequence, clear the screen and print the start-up text.
   *
   * @param spi_cs SPI 片选 GPIO。
   *               SPI chip-select GPIO.
   * @param spi_rs 数据/命令选择 GPIO。
   *               Data/command select GPIO.
   * @param pwm 背光 PWM。
   *            Backlight PWM.
   * @param spi 屏幕所在的 SPI 总线。
   *            SPI bus of the display.
   * @param panel 面板厂商。
   *              Panel vendor.
   * @param type 屏幕尺寸。
   *             Screen size.
   * @param orientation 显示方向。
   *                    Display orientation.
   * @param format 写入 COLOR_MODE 的像素格式。
   *               Pixel format written to COLOR_MODE.
   */
  ST7735(
      LibXR::GPIO& spi_cs,
      LibXR::GPIO& spi_rs,
      LibXR::PWM& pwm,
      LibXR::SPI& spi,
      PanelType panel = ST7735::PanelType::HANNSTAR_PANEL,
      ScreenType type = ST7735::ScreenType::SCREEN_0_9,
      Orientation orientation = ST7735::Orientation::LANDSCAPE,
      PixelFormat format = ST7735::PixelFormat::FORMAT_RGB565)
      : panel_(panel), type_(type), orientation_(orientation), color_coding_(format)
  {
    st7735_spi_cs_ = std::addressof(spi_cs);
    st7735_spi_rs_ = std::addressof(spi_rs);
    st7735_pwm_ = std::addressof(pwm);
    st7735_spi_ = std::addressof(spi);

    st7735_spi_cs_->SetConfig({.direction = LibXR::GPIO::Direction::OUTPUT_PUSH_PULL,
                               .pull = LibXR::GPIO::Pull::NONE});

    st7735_spi_rs_->SetConfig({.direction = LibXR::GPIO::Direction::OUTPUT_PUSH_PULL,
                               .pull = LibXR::GPIO::Pull::NONE});

    st7735_spi_cs_->Write(true);
    st7735_spi_rs_->Write(true);

    st7735_pwm_->SetConfig({.frequency = 10000});

    st7735_pwm_->Enable();

    SetBrightness(1.0f);

    st7735_spi_->SetConfig({.clock_polarity = LibXR::SPI::ClockPolarity::LOW,
                            .clock_phase = LibXR::SPI::ClockPhase::EDGE_1});

    Init();

    FillRect(0, 0, width_, height_, Color::WHITE);

    char init_msg[] = "XRobot ST7735 Driver";
    char project_url[] = "xrobot.work";

    ShowString(Color::BLACK, Color::WHITE, 0, 0, width_, 16, 12, init_msg);
    ShowString(Color::BLACK, Color::WHITE, 0, 12, width_, 16, 12, project_url);
  }

  /**
   * @brief 写命令寄存器，data 非空时随后写入参数。
   *        Write a command register, followed by the arguments when data is not empty.
   *
   * @param reg 命令寄存器地址。
   *            Command register address.
   * @param data 命令参数。
   *             Command arguments.
   */
  void WriteReg(uint8_t reg, LibXR::RawData data)
  {
    st7735_spi_cs_->Write(false);
    st7735_spi_rs_->Write(false);
    st7735_spi_->Write(reg, spi_op_);
    st7735_spi_rs_->Write(true);
    if (data.size_ > 0)
    {
      st7735_spi_->Write(data, spi_op_);
    }
    st7735_spi_cs_->Write(true);
  }

  /**
   * @brief 以数据模式写入一段原始数据。
   *        Write a block of raw data in data mode.
   *
   * @param data 待写入的数据。
   *             Data to write.
   */
  void SendData(LibXR::RawData data)
  {
    st7735_spi_cs_->Write(false);
    st7735_spi_->Write(data, spi_op_);
    st7735_spi_cs_->Write(true);
  }

  /**
   * @brief 执行 ST7735 初始化序列。
   *        Run the ST7735 init sequence.
   */
  void Init()
  {
    uint8_t tmp;

    // Software reset, 0 args, delay 120ms
    tmp = 0x00U;
    WriteReg(Command::SW_RESET, {&tmp, 0});
    LibXR::Thread::Sleep(120);

    tmp = 0x00U;
    WriteReg(Command::SW_RESET, {&tmp, 0});
    LibXR::Thread::Sleep(120);

    // Out of sleep mode, 0 args, no delay
    tmp = 0x00U;
    WriteReg(Command::SLEEP_OUT, {&tmp, 1});

    // Frame rate ctrl - normal mode, 3 args
    WriteReg(Command::FRAME_RATE_CTRL1, {&tmp, 0});
    tmp = 0x01U;
    SendData({&tmp, 1});
    tmp = 0x2CU;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});

    // Frame rate control - idle mode, 3 args
    tmp = 0x01U;
    WriteReg(Command::FRAME_RATE_CTRL2, {&tmp, 1});
    tmp = 0x2CU;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});

    // Frame rate ctrl - partial mode, 6 args
    tmp = 0x01U;
    WriteReg(Command::FRAME_RATE_CTRL3, {&tmp, 1});
    tmp = 0x2CU;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});
    tmp = 0x01U;
    SendData({&tmp, 1});
    tmp = 0x2CU;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});

    // Display inversion ctrl, 1 arg, no delay: No inversion
    tmp = 0x07U;
    WriteReg(Command::FRAME_INVERSION_CTRL, {&tmp, 1});

    // Power control, 3 args, no delay: -4.6V , AUTO mode
    tmp = 0xA2U;
    WriteReg(Command::PWR_CTRL1, {&tmp, 1});
    tmp = 0x02U;
    SendData({&tmp, 1});
    tmp = 0x84U;
    SendData({&tmp, 1});

    // Power control, 1 arg, no delay: VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    tmp = 0xC5U;
    WriteReg(Command::PWR_CTRL2, {&tmp, 1});

    // Power control, 2 args, no delay: Opamp current small, Boost frequency
    tmp = 0x0AU;
    WriteReg(Command::PWR_CTRL3, {&tmp, 1});
    tmp = 0x00U;
    SendData({&tmp, 1});

    // Power control, 2 args, no delay: BCLK/2, Opamp current small & Medium low
    tmp = 0x8AU;
    WriteReg(Command::PWR_CTRL4, {&tmp, 1});
    tmp = 0x2AU;
    SendData({&tmp, 1});

    // Power control, 2 args, no delay
    tmp = 0x8AU;
    WriteReg(Command::PWR_CTRL5, {&tmp, 1});
    tmp = 0xEEU;
    SendData({&tmp, 1});

    // Power control, 1 arg, no delay
    tmp = 0x0EU;
    WriteReg(Command::VCOMH_VCOML_CTRL1, {&tmp, 1});

    // choose panel_
    if (panel_ == PanelType::HANNSTAR_PANEL)
    {
      WriteReg(Command::DISPLAY_INVERSION_ON, {&tmp, 0});
    }
    else
    {
      WriteReg(Command::DISPLAY_INVERSION_OFF, {&tmp, 0});
    }
    // Set color mode, 1 arg, no delay
    WriteReg(Command::COLOR_MODE, {&color_coding_, 1});

    // Positive gamma correction, 16 args, no delay
    tmp = 0x02U;
    WriteReg(Command::PV_GAMMA_CTRL, {&tmp, 1});
    tmp = 0x1CU;
    SendData({&tmp, 1});
    tmp = 0x07U;
    SendData({&tmp, 1});
    tmp = 0x12U;
    SendData({&tmp, 1});
    tmp = 0x37U;
    SendData({&tmp, 1});
    tmp = 0x32U;
    SendData({&tmp, 1});
    tmp = 0x29U;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});
    tmp = 0x29U;
    SendData({&tmp, 1});
    tmp = 0x25U;
    SendData({&tmp, 1});
    tmp = 0x2BU;
    SendData({&tmp, 1});
    tmp = 0x39U;
    SendData({&tmp, 1});
    tmp = 0x00U;
    SendData({&tmp, 1});
    tmp = 0x01U;
    SendData({&tmp, 1});
    tmp = 0x03U;
    SendData({&tmp, 1});
    tmp = 0x10U;
    SendData({&tmp, 1});

    // Negative gamma correction, 16 args, no delay
    tmp = 0x03U;
    WriteReg(Command::NV_GAMMA_CTRL, {&tmp, 1});
    tmp = 0x1DU;
    SendData({&tmp, 1});
    tmp = 0x07U;
    SendData({&tmp, 1});
    tmp = 0x06U;
    SendData({&tmp, 1});
    tmp = 0x2EU;
    SendData({&tmp, 1});
    tmp = 0x2CU;
    SendData({&tmp, 1});
    tmp = 0x29U;
    SendData({&tmp, 1});
    tmp = 0x2DU;
    SendData({&tmp, 1});
    tmp = 0x2EU;
    SendData({&tmp, 1});
    tmp = 0x2EU;
    SendData({&tmp, 1});
    tmp = 0x37U;
    SendData({&tmp, 1});
    tmp = 0x3FU;
    SendData({&tmp, 1});
    tmp = 0x00U;
    SendData({&tmp, 1});
    tmp = 0x00U;
    SendData({&tmp, 1});
    tmp = 0x02U;
    SendData({&tmp, 1});
    tmp = 0x10U;
    SendData({&tmp, 1});

    // Normal display on, no args, no delay
    tmp = 0x00U;
    WriteReg(Command::NORMAL_DISPLAY_OFF, {&tmp, 1});

    // Display on, no delay
    WriteReg(Command::DISPLAY_ON, {&tmp, 1});

    // Set the display Orientation and the default display window
    SetOrientation();
  }

  /**
   * @brief 按显示方向与屏幕尺寸设置宽高、显示窗口与 MADCTL。
   *        Set the width, height, display window and MADCTL from the orientation and
   *        screen size.
   */
  void SetOrientation()
  {
    uint8_t tmp;

    if ((orientation_ == Orientation::PORTRAIT) ||
        (orientation_ == Orientation::PORTRAIT_ROT180))
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {
        width_ = WIDTH_0_9;
        height_ = HEIGHT_0_9;
      }
      else if (type_ == ScreenType::SCREEN_1_8 || type_ == ScreenType::SCREEN_1_8A)
      {
        width_ = WIDTH_1_8;
        height_ = HEIGHT_1_8;
      }
    }
    else
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {
        width_ = HEIGHT_0_9;
        height_ = WIDTH_0_9;
      }
      else if (type_ == ScreenType::SCREEN_1_8 || type_ == ScreenType::SCREEN_1_8A)
      {
        width_ = HEIGHT_1_8;
        height_ = WIDTH_1_8;
      }
    }

    SetDisplayWindow(0U, 0U);

    tmp = panel_ == PanelType::HANNSTAR_PANEL
              ? static_cast<uint8_t>(
                    OrientationTab[static_cast<uint8_t>(orientation_)][1]) |
                    RGBOrder::LCD_BGR
              : static_cast<uint8_t>(
                    OrientationTab[static_cast<uint8_t>(orientation_)][1]) |
                    RGBOrder::LCD_RGB;

    WriteReg(Command::MADCTL, {&tmp, 1});
  }

  /**
   * @brief 设置覆盖整个屏幕的显示窗口，起点按面板与方向修正。
   *        Set the display window covering the whole screen, with the origin corrected
   *        for the panel and orientation.
   *
   * @param Xpos 窗口起点的列。
   *             Column of the window origin.
   * @param Ypos 窗口起点的行。
   *             Row of the window origin.
   */
  void SetDisplayWindow(uint32_t Xpos, uint32_t Ypos)
  {
    uint8_t tmp;

    // Cursor calibration
    if (orientation_ <= Orientation::PORTRAIT_ROT180)
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {  // 0.96 ST7735
        if (panel_ == PanelType::HANNSTAR_PANEL)
        {
          Xpos += 26;
          Ypos += 1;
        }
        else
        {  // BOE Panel
          Xpos += 24;
          Ypos += 0;
        }
      }
      else if (type_ == ScreenType::SCREEN_1_8A)
      {
        if (panel_ == PanelType::BOE_PANEL)
        {
          Xpos += 2;
          Ypos += 1;
        }
      }
    }
    else
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {
        if (panel_ == PanelType::HANNSTAR_PANEL)
        {  // 0.96 ST7735
          Xpos += 1;
          Ypos += 26;
        }
        else
        {  // BOE Panel
          Xpos += 1;
          Ypos += 24;
        }
      }
      else if (type_ == ScreenType::SCREEN_1_8A)
      {
        if (panel_ == PanelType::BOE_PANEL)
        {
          Xpos += 1;
          Ypos += 2;
        }
      }
    }

    // Column addr set, 4 args, no delay: XSTART = Xpos, XEND = (Xpos + width_ -
    // 1)
    WriteReg(Command::CASET, {&tmp, 0});
    tmp = (uint8_t)(Xpos >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Xpos & 0xFFU);
    SendData({&tmp, 1});
    tmp = (uint8_t)((Xpos + width_ - 1U) >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)((Xpos + width_ - 1U) & 0xFFU);
    SendData({&tmp, 1});

    // Row addr set, 4 args, no delay: YSTART = Ypos, YEND = (Ypos + height_ -
    // 1)
    WriteReg(Command::RASET, {&tmp, 0});
    tmp = (uint8_t)(Ypos >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Ypos & 0xFFU);
    SendData({&tmp, 1});
    tmp = (uint8_t)((Ypos + height_ - 1U) >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)((Ypos + height_ - 1U) & 0xFFU);
    SendData({&tmp, 1});
  }

  /**
   * @brief 用单一颜色填充矩形，矩形超出屏幕时忽略该调用。
   *        Fill a rectangle with one color; the call is ignored when the rectangle
   *        exceeds the screen.
   *
   * @param Xpos 矩形左上角的列。
   *             Column of the top-left corner.
   * @param Ypos 矩形左上角的行。
   *             Row of the top-left corner.
   * @param Width 矩形宽度，单位像素。
   *              Rectangle width in pixels.
   * @param Height 矩形高度，单位像素。
   *               Rectangle height in pixels.
   * @param Color RGB565 颜色。
   *              RGB565 color.
   */
  void FillRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height,
                uint32_t Color)
  {
    if (((Xpos + Width) > width_) || ((Ypos + Height) > height_)) return;

    SetWindow(Xpos, Ypos, Xpos + Width - 1, Ypos + Height - 1);

    // 分配整块颜色数据 / Allocate a block of color data
    uint32_t pixelCount = Width * Height;
    static uint8_t buf[2048];  // 2KB
    uint32_t remain = pixelCount;
    uint8_t hi = Color >> 8, lo = Color & 0xFF;

    // 填充缓冲区 / Fill the buffer
    for (uint32_t i = 0; i < sizeof(buf) / 2; ++i)
    {
      buf[2 * i] = hi;
      buf[2 * i + 1] = lo;
    }

    // 分批发送 / Send in batches
    while (remain > 0)
    {
      uint32_t chunk = remain > (sizeof(buf) / 2) ? (sizeof(buf) / 2) : remain;
      SendData({buf, chunk * 2});
      remain -= chunk;
    }
  }

  /**
   * @brief 设置绘图窗口并开始写显存，坐标按面板与方向修正。
   *        Set the drawing window and start writing the RAM, with the coordinates
   *        corrected for the panel and orientation.
   *
   * @param Xpos0 窗口起始列。
   *              First column of the window.
   * @param Ypos0 窗口起始行。
   *              First row of the window.
   * @param Xpos1 窗口结束列。
   *              Last column of the window.
   * @param Ypos1 窗口结束行。
   *              Last row of the window.
   */
  void SetWindow(uint32_t Xpos0, uint32_t Ypos0, uint32_t Xpos1, uint32_t Ypos1)
  {
    uint8_t tmp;

    if (orientation_ <= Orientation::PORTRAIT_ROT180)
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {  // 0.96寸
        if (panel_ == PanelType::HANNSTAR_PANEL)
        {
          Xpos0 += 26;
          Xpos1 += 26;
          Ypos0 += 1;
          Ypos1 += 1;
        }
        else
        {  // BOE Panel
          Xpos0 += 24;
          Xpos1 += 24;
        }
      }
      else if (type_ == ScreenType::SCREEN_1_8A)
      {
        if (panel_ == PanelType::BOE_PANEL)
        {
          Xpos0 += 2;
          Xpos1 += 2;
          Ypos0 += 1;
          Ypos1 += 1;
        }
      }
    }
    else
    {
      if (type_ == ScreenType::SCREEN_0_9)
      {
        if (panel_ == PanelType::HANNSTAR_PANEL)
        {
          Xpos0 += 1;
          Xpos1 += 1;
          Ypos0 += 26;
          Ypos1 += 26;
        }
        else
        {  // BOE Panel
          Ypos0 += 24;
          Ypos1 += 24;
        }
      }
      else if (type_ == ScreenType::SCREEN_1_8A)
      {
        if (panel_ == PanelType::BOE_PANEL)
        {
          Xpos0 += 1;
          Xpos1 += 1;
          Ypos0 += 2;
          Ypos1 += 2;
        }
      }
    }

    WriteReg(Command::CASET, {nullptr, 0});
    tmp = (uint8_t)(Xpos0 >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Xpos0 & 0xFFU);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Xpos1 >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Xpos1 & 0xFFU);
    SendData({&tmp, 1});

    WriteReg(Command::RASET, {nullptr, 0});
    tmp = (uint8_t)(Ypos0 >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Ypos0 & 0xFFU);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Ypos1 >> 8U);
    SendData({&tmp, 1});
    tmp = (uint8_t)(Ypos1 & 0xFFU);
    SendData({&tmp, 1});

    WriteReg(Command::WRITE_RAM, {nullptr, 0});
  }

  /**
   * @brief 在指定区域内绘制 ASCII 字符串，超出区域宽度时换行。
   *        Draw an ASCII string inside the given area, wrapping at the area width.
   *
   * @param point_color 前景色，RGB565。
   *                    Foreground color, RGB565.
   * @param back_color 背景色，RGB565。
   *                   Background color, RGB565.
   * @param x 区域左上角的列。
   *          Column of the top-left corner of the area.
   * @param y 区域左上角的行。
   *          Row of the top-left corner of the area.
   * @param width 区域宽度，单位像素。
   *              Area width in pixels.
   * @param height 区域高度，单位像素。
   *               Area height in pixels.
   * @param size 字体高度，12（6x12）或 16（8x16）。
   *             Font height, 12 (6x12) or 16 (8x16).
   * @param data 以 '\0' 结尾的字符串，遇到可显示 ASCII 范围之外的字符时停止。
   *             Null-terminated string; drawing stops at a character outside the
   *             printable ASCII range.
   */
  void ShowString(uint16_t point_color, uint16_t back_color, uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height, uint8_t size, const char* data)
  {
    uint8_t x0 = x;
    width += x;
    height += y;
    while ((*data <= '~') && (*data >= ' '))
    {
      if (x >= width)
      {
        x = x0;
        y += size;
      }
      if (y >= height) break;
      ShowChar(point_color, back_color, x, y, *data, size);
      x += size / 2;
      data++;
    }
  }

  /**
   * @brief 绘制单个 ASCII 字符。
   *        Draw one ASCII character.
   *
   * @param point_color 前景色，RGB565。
   *                    Foreground color, RGB565.
   * @param back_color 背景色，RGB565。
   *                   Background color, RGB565.
   * @param x 字符左上角的列。
   *          Column of the top-left corner of the character.
   * @param y 字符左上角的行。
   *          Row of the top-left corner of the character.
   * @param num 待绘制的字符。
   *            Character to draw.
   * @param size 字体高度，12（6x12）或 16（8x16）。
   *             Font height, 12 (6x12) or 16 (8x16).
   */
  void ShowChar(uint16_t point_color, uint16_t back_color, uint16_t x, uint16_t y,
                uint8_t num, uint8_t size)
  {
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint16_t x0 = x;
    uint16_t colortemp = point_color;

    uint16_t length = size == 12 ? 6 : 8;
    static uint16_t write[16 * 8] = {0};
    uint16_t count;

    num = num - ' ';
    count = 0;

    for (t = 0; t < size; t++)
    {
      if (size == 12)
        temp = asc2_1206[num][t];
      else
        temp = asc2_1608[num][t];

      for (t1 = 0; t1 < 8; t1++)
      {
        if (temp & 0x80)
          point_color = (colortemp & 0xFF) << 8 | colortemp >> 8;
        else
          point_color = (back_color & 0xFF) << 8 | back_color >> 8;

        write[count * length + t / 2] = point_color;
        count++;
        if (count >= size) count = 0;

        temp <<= 1;
        y++;
        if (y >= height_)
        {
          point_color = colortemp;
          return;
        }
        if ((y - y0) == size)
        {
          y = y0;
          x++;
          if (x >= width_)
          {
            point_color = colortemp;
            return;
          }
          break;
        }
      }
    }

    FillRGBRect(x0, y0, (uint8_t*)&write, size == 12 ? 6 : 8, size);
    point_color = colortemp;
  }

  /**
   * @brief 写入一块像素数据，每像素 2 字节，高字节在前；矩形超出屏幕时忽略该调用。
   *        Write a block of pixels, 2 bytes per pixel with the high byte first; the
   *        call is ignored when the rectangle exceeds the screen.
   *
   * @param Xpos 矩形左上角的列。
   *             Column of the top-left corner.
   * @param Ypos 矩形左上角的行。
   *             Row of the top-left corner.
   * @param pData 像素数据，长度为 Width * Height * 2 字节。
   *              Pixel data of Width * Height * 2 bytes.
   * @param Width 矩形宽度，单位像素。
   *              Rectangle width in pixels.
   * @param Height 矩形高度，单位像素。
   *               Rectangle height in pixels.
   */
  void FillRGBRect(uint32_t Xpos, uint32_t Ypos, uint8_t* pData, uint32_t Width,
                   uint32_t Height)
  {
    if (((Xpos + Width) > width_) || ((Ypos + Height) > height_))
    {
      return;
    }

    SetWindow(Xpos, Ypos, Xpos + Width - 1, Ypos + Height - 1);
    // 一次性写入全部像素 / Write all pixels at once
    SendData({pData, Width * Height * 2});
  }

  /**
   * @brief 设置背光占空比。
   *        Set the backlight duty cycle.
   *
   * @param brightness 占空比，范围 0.0 到 1.0。
   *                   Duty cycle in the range 0.0 to 1.0.
   */
  void SetBrightness(float brightness) { st7735_pwm_->SetDutyCycle(brightness); }

  /**
   * @brief 获取当前方向下的屏幕宽度。
   *        Get the screen width in the current orientation.
   *
   * @return 屏幕宽度，单位像素。
   *         Screen width in pixels.
   */
  uint16_t GetWidth() { return width_; }

  /**
   * @brief 获取当前方向下的屏幕高度。
   *        Get the screen height in the current orientation.
   *
   * @return 屏幕高度，单位像素。
   *         Screen height in pixels.
   */
  uint16_t GetHeight() { return height_; }

 private:
  PanelType panel_ = PanelType::HANNSTAR_PANEL;
  ScreenType type_ = ScreenType::SCREEN_0_9;
  Orientation orientation_ = Orientation::LANDSCAPE_ROT180;
  PixelFormat color_coding_ = PixelFormat::FORMAT_RGB565;

  LibXR::GPIO *st7735_spi_cs_, *st7735_spi_rs_;
  LibXR::PWM* st7735_pwm_;
  LibXR::SPI* st7735_spi_;

  uint32_t width_ = 0, height_ = 0;

  LibXR::Semaphore spi_sem_;
  LibXR::SPI::OperationRW spi_op_ = LibXR::SPI::OperationRW(spi_sem_);
};
