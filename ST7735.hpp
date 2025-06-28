#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: ST7735 显示屏驱动 / ST7735 display driver
constructor_args: 
  - panel: ST7735::PanelType::HANNSTAR_PANEL
  - type: ST7735::ScreenType::SCREEN_0_9
  - orientation: ST7735::Orientation::LANDSCAPE
  - format: ST7735::PixelFormat::FORMAT_RGB565
template_args: []
required_hardware:
  - st7735_spi
  - st7735_spi_cs
  - st7735_spi_rs
  - st7735_pwm
depends: []
=== END MANIFEST === */
// clang-format on

#include "app_framework.hpp"
#include "font.h"
#include "gpio.hpp"
#include "libxr_type.hpp"
#include "pwm.hpp"
#include "semaphore.hpp"
#include "spi.hpp"
#include <cstdint>

class ST7735 : public LibXR::Application {
public:
  enum Command : uint8_t {
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

  /// 屏幕尺寸 / Screen size
  enum ScreenType : uint8_t {
    SCREEN_1_8 = 0x00,
    SCREEN_0_9 = 0x01,
    SCREEN_1_8A = 0x02
  };

  /// 面板类型 / Panel type
  enum PanelType : uint8_t { HANNSTAR_PANEL = 0x00, BOE_PANEL = 0x01 };

  /// 方向 / Orientation
  enum Orientation : uint8_t {
    PORTRAIT = 0x00,
    PORTRAIT_ROT180 = 0x01,
    LANDSCAPE = 0x02,
    LANDSCAPE_ROT180 = 0x03
  };

  /// 像素格式 / Pixel format
  enum PixelFormat : uint8_t {
    FORMAT_RGB444 = 0x03,
    FORMAT_RGB565 = 0x05,
    FORMAT_RGB666 = 0x06,
    FORMAT_DEFAULT = FORMAT_RGB565
  };

  /// 屏幕物理宽高 / Screen physical size
  static constexpr uint16_t WIDTH_1_8 = 128;
  static constexpr uint16_t HEIGHT_1_8 = 160;
  static constexpr uint16_t WIDTH_0_9 = 80;
  static constexpr uint16_t HEIGHT_0_9 = 160;

  /// 方向->MADCTL设置表（不变） / Orientation->MADCTL table
  static constexpr uint32_t OrientationTab[4][2] = {
      {0x40U, 0xC0U}, {0x80U, 0x00U}, {0x20U, 0x60U}, {0xE0U, 0xA0U}};

  /// RGB/BGR
  enum RGBOrder : uint8_t { LCD_RGB = 0x00, LCD_BGR = 0x08 };

  /// 颜色
  enum Color : uint16_t {
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

  ST7735(LibXR::HardwareContainer &hw, LibXR::ApplicationManager &app,
         PanelType panel, ScreenType type, Orientation orientation,
         PixelFormat format)
      : panel_(panel), type_(type), orientation_(orientation),
        color_coding_(format) {
    st7735_spi_cs_ = hw.template FindOrExit<LibXR::GPIO>({"st7735_spi_cs"});
    st7735_spi_rs_ = hw.template FindOrExit<LibXR::GPIO>({"st7735_spi_rs"});
    st7735_pwm_ = hw.template FindOrExit<LibXR::PWM>({"st7735_pwm"});
    st7735_spi_ = hw.template FindOrExit<LibXR::SPI>({"st7735_spi"});

    st7735_spi_cs_->SetConfig(
        {.direction = LibXR::GPIO::Direction::OUTPUT_PUSH_PULL,
         .pull = LibXR::GPIO::Pull::NONE});

    st7735_spi_rs_->SetConfig(
        {.direction = LibXR::GPIO::Direction::OUTPUT_PUSH_PULL,
         .pull = LibXR::GPIO::Pull::NONE});

    st7735_spi_cs_->Write(true);
    st7735_spi_rs_->Write(true);

    st7735_pwm_->SetConfig({.frequency = 10000});

    st7735_pwm_->Enable();

    SetBrightness(1.0f);

    st7735_spi_->SetConfig({.clock_polarity = LibXR::SPI::ClockPolarity::LOW,
                            .clock_phase = LibXR::SPI::ClockPhase::EDGE_1});

    Init();

    FillRect(0, 0, width_, height_, Color::BLUE);

    char text[] = "XRobot ST7735 Driver";

    ShowString(Color::RED, Color::BLACK, 0, 58, width_, 16, 16, text);

    app.Register(*this);
  }

  void WriteReg(uint8_t reg, LibXR::RawData data) {
    st7735_spi_cs_->Write(false);
    st7735_spi_rs_->Write(false);
    st7735_spi_->Write(reg, spi_op_);
    st7735_spi_rs_->Write(true);
    if (data.size_ > 0) {
      st7735_spi_->Write(data, spi_op_);
    }
    st7735_spi_cs_->Write(true);
  }

  void SendData(LibXR::RawData data) {
    st7735_spi_cs_->Write(false);
    st7735_spi_->Write(data, spi_op_);
    st7735_spi_cs_->Write(true);
  }

  void Init() {
    uint8_t tmp;

    // Out of sleep mode, 0 args, delay 120ms
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
    if (panel_ == PanelType::HANNSTAR_PANEL) {
      WriteReg(Command::DISPLAY_INVERSION_ON, {&tmp, 0});
    } else {
      WriteReg(Command::DISPLAY_INVERSION_OFF, {&tmp, 0});
    }
    // Set color mode, 1 arg, no delay
    WriteReg(Command::COLOR_MODE, {&color_coding_, 1});

    // Magical unicorn dust, 16 args, no delay
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

    // Sparkles and rainbows, 16 args, no delay
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

    // Main screen turn on, no delay
    WriteReg(Command::DISPLAY_ON, {&tmp, 1});

    // Set the display Orientation and the default display window
    SetOrientation();
  }

  void SetOrientation() {
    uint8_t tmp;

    if ((orientation_ == Orientation::PORTRAIT) ||
        (orientation_ == Orientation::PORTRAIT_ROT180)) {
      if (type_ == ScreenType::SCREEN_0_9) {
        width_ = WIDTH_0_9;
        height_ = HEIGHT_0_9;
      } else if (type_ == ScreenType::SCREEN_1_8 ||
                 type_ == ScreenType::SCREEN_1_8A) {
        width_ = WIDTH_1_8;
        height_ = HEIGHT_1_8;
      }
    } else {
      if (type_ == ScreenType::SCREEN_0_9) {
        width_ = HEIGHT_0_9;
        height_ = WIDTH_0_9;
      } else if (type_ == ScreenType::SCREEN_1_8 ||
                 type_ == ScreenType::SCREEN_1_8A) {
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

  void SetDisplayWindow(uint32_t Xpos, uint32_t Ypos) {
    uint8_t tmp;

    // Cursor calibration
    if (orientation_ <= Orientation::PORTRAIT_ROT180) {
      if (type_ == ScreenType::SCREEN_0_9) { // 0.96 ST7735
        if (panel_ == PanelType::HANNSTAR_PANEL) {
          Xpos += 26;
          Ypos += 1;
        } else { // BOE Panel
          Xpos += 24;
          Ypos += 0;
        }
      } else if (type_ == ScreenType::SCREEN_1_8A) {
        if (panel_ == PanelType::BOE_PANEL) {
          Xpos += 2;
          Ypos += 1;
        }
      }
    } else {
      if (type_ == ScreenType::SCREEN_0_9) {
        if (panel_ == PanelType::HANNSTAR_PANEL) { // 0.96 ST7735
          Xpos += 1;
          Ypos += 26;
        } else { // BOE Panel
          Xpos += 1;
          Ypos += 24;
        }
      } else if (type_ == ScreenType::SCREEN_1_8A) {
        if (panel_ == PanelType::BOE_PANEL) {
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

  void FillRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height,
                uint32_t Color) {
    if (((Xpos + Width) > width_) || ((Ypos + Height) > height_))
      return;

    SetWindow(Xpos, Ypos, Xpos + Width - 1, Ypos + Height - 1);

    // 分配整块颜色数据 / Allocate a block of color data
    uint32_t pixelCount = Width * Height;
    static uint8_t buf[2048]; // 2KB
    uint32_t remain = pixelCount;
    uint8_t hi = Color >> 8, lo = Color & 0xFF;

    // 填充缓冲区 / Fill the buffer
    for (uint32_t i = 0; i < sizeof(buf) / 2; ++i) {
      buf[2 * i] = hi;
      buf[2 * i + 1] = lo;
    }

    // 分批发送 / Send in batches
    while (remain > 0) {
      uint32_t chunk = remain > (sizeof(buf) / 2) ? (sizeof(buf) / 2) : remain;
      SendData({buf, chunk * 2});
      remain -= chunk;
    }
  }

  void SetWindow(uint32_t Xpos0, uint32_t Ypos0, uint32_t Xpos1,
                 uint32_t Ypos1) {
    uint8_t tmp;

    if (orientation_ <= Orientation::PORTRAIT_ROT180) {
      if (type_ == ScreenType::SCREEN_0_9) { // 0.96寸
        if (panel_ == PanelType::HANNSTAR_PANEL) {
          Xpos0 += 26;
          Xpos1 += 26;
          Ypos0 += 1;
          Ypos1 += 1;
        } else { // BOE Panel
          Xpos0 += 24;
          Xpos1 += 24;
        }
      } else if (type_ == ScreenType::SCREEN_1_8A) {
        if (panel_ == PanelType::BOE_PANEL) {
          Xpos0 += 2;
          Xpos1 += 2;
          Ypos0 += 1;
          Ypos1 += 1;
        }
      }
    } else {
      if (type_ == ScreenType::SCREEN_0_9) {
        if (panel_ == PanelType::HANNSTAR_PANEL) {
          Xpos0 += 1;
          Xpos1 += 1;
          Ypos0 += 26;
          Ypos1 += 26;
        } else { // BOE Panel
          Ypos0 += 24;
          Ypos1 += 24;
        }
      } else if (type_ == ScreenType::SCREEN_1_8A) {
        if (panel_ == PanelType::BOE_PANEL) {
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

  void ShowString(uint16_t point_color, uint16_t back_color, uint16_t x,
                  uint16_t y, uint16_t width, uint16_t height, uint8_t size,
                  const char *data) {
    uint8_t x0 = x;
    width += x;
    height += y;
    while ((*data <= '~') && (*data >= ' ')) {
      if (x >= width) {
        x = x0;
        y += size;
      }
      if (y >= height)
        break;
      ShowChar(point_color, back_color, x, y, *data, size);
      x += size / 2;
      data++;
    }
  }

  void ShowChar(uint16_t point_color, uint16_t back_color, uint16_t x,
                uint16_t y, uint8_t num, uint8_t size) {
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint16_t x0 = x;
    uint16_t colortemp = point_color;

    uint16_t length = size == 12 ? 6 : 8;
    static uint16_t write[16 * 8] = {0};
    uint16_t count;

    num = num - ' ';
    count = 0;

    for (t = 0; t < size; t++) {
      if (size == 12)
        temp = asc2_1206[num][t];
      else
        temp = asc2_1608[num][t];

      for (t1 = 0; t1 < 8; t1++) {
        if (temp & 0x80)
          point_color = (colortemp & 0xFF) << 8 | colortemp >> 8;
        else
          point_color = (back_color & 0xFF) << 8 | back_color >> 8;

        write[count * length + t / 2] = point_color;
        count++;
        if (count >= size)
          count = 0;

        temp <<= 1;
        y++;
        if (y >= height_) {
          point_color = colortemp;
          return;
        }
        if ((y - y0) == size) {
          y = y0;
          x++;
          if (x >= width_) {
            point_color = colortemp;
            return;
          }
          break;
        }
      }
    }

    FillRGBRect(x0, y0, (uint8_t *)&write, size == 12 ? 6 : 8, size);
    point_color = colortemp;
  }

  void FillRGBRect(uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width,
                   uint32_t Height) {
    if (((Xpos + Width) > width_) || ((Ypos + Height) > height_)) {
      return;
    }

    SetWindow(Xpos, Ypos, Xpos + Width - 1, Ypos + Height - 1);
    // 一次性写入全部像素 / Write all pixels at once
    SendData({pData, Width * Height * 2});
  }

  void SetBrightness(float brightness) {
    st7735_pwm_->SetDutyCycle(brightness);
  }

  uint16_t GetWidth() { return width_; }
  uint16_t GetHeight() { return height_; }

  void OnMonitor() override {}

private:
  PanelType panel_ = PanelType::HANNSTAR_PANEL;
  ScreenType type_ = ScreenType::SCREEN_0_9;
  Orientation orientation_ = Orientation::LANDSCAPE_ROT180;
  PixelFormat color_coding_ = PixelFormat::FORMAT_RGB565;

  LibXR::GPIO *st7735_spi_cs_, *st7735_spi_rs_;
  LibXR::PWM *st7735_pwm_;
  LibXR::SPI *st7735_spi_;

  uint32_t width_ = 0, height_ = 0;

  LibXR::Semaphore spi_sem_;
  LibXR::SPI::OperationRW spi_op_ = LibXR::SPI::OperationRW(spi_sem_);
};
