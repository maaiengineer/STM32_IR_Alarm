# STM32 IR Alarm 🚨

ระบบสัญญาณกันขโมยด้วย **IR sensor + Buzzer** บนบอร์ด **NUCLEO-F411RE** เขียนด้วย STM32 HAL และ build ด้วย **Keil µVision (MDK-ARM)**

A simple intrusion alarm for the NUCLEO-F411RE using an IR obstacle sensor and a buzzer module (STM32 HAL, Keil MDK-ARM).

## 🎬 Demo

<!-- ลากไฟล์วิดีโอมาวางตรงนี้ตอนแก้ README บน GitHub แล้วลบบรรทัดนี้ -->

## ✨ Features

- เปิด/ปิดระบบด้วยปุ่ม B1 บนบอร์ด พร้อมเสียงยืนยัน (ติ๊ด ติ๊ด = เปิด, ปี๊ดยาว = ปิด)
- เมื่อเปิดระบบแล้วมีวัตถุผ่าน IR → เสียงไซเรนกวาดความถี่ 800–1600 Hz และ LED กะพริบ
- ตรวจจับด้วย External Interrupt (EXTI) ไม่ต้องวนเช็กขา
- สร้างเสียงด้วย Timer PWM (TIM3) ปรับความถี่ได้
- ส่ง log ผ่าน UART (ST-Link Virtual COM, 115200 baud)

## 🧰 Hardware

| อุปกรณ์ | จำนวน |
|---|---|
| NUCLEO-F411RE | 1 |
| IR obstacle sensor (FC-51 หรือเทียบเท่า) | 1 |
| Buzzer module (low-level trigger) | 1 |
| สายจัมเปอร์ | 6 |

## 🔌 Wiring

| อุปกรณ์ | ขาโมดูล | NUCLEO-F411RE |
|---|---|---|
| IR sensor | VCC / GND / OUT | 3V3 / GND / **D2 (PA10)** |
| Buzzer | VCC / GND / I/O | 3V3 / GND / **D12 (PA6)** |

> ⚠️ ต่อ VCC ของ buzzer ที่ **3V3** เท่านั้น โมดูลแบบ low-level trigger ที่เลี้ยงด้วย 5V จะปิดเสียงไม่ได้

## ⚙️ Pin & Peripheral Config

| ขา | หน้าที่ | การตั้งค่า |
|---|---|---|
| PA10 | IR_OUT | GPIO_EXTI10, falling edge, pull-up |
| PC13 | B1 (ปุ่มสีฟ้า) | GPIO_EXTI13, falling edge |
| PA6 | Buzzer | TIM3_CH1 PWM, Prescaler 83, Period 999, polarity LOW |
| PA5 | LD2 (LED เขียว) | GPIO output |
| PA2/PA3 | USART2 | 115200 8N1 |

System clock: HSI → PLL → **84 MHz**, timer tick = 1 MHz

## 🔄 How it works

```
          กด B1                 IR จับได้
DISARMED ───────▶ ARMED ───────────────▶ ALARM
   ▲                │                      │
   └──── กด B1 ─────┴──────── กด B1 ───────┘
```

| สถานะ | LED LD2 | Buzzer |
|---|---|---|
| DISARMED | ดับ | เงียบ |
| ARMED | ติดค้าง | เงียบ (รอ IR) |
| ALARM | กะพริบ | ไซเรน |

## 🛠️ Build & Flash (Keil µVision)

1. เปิด `MDK-ARM/STM32_IR_Alarm.uvprojx`
2. ติดตั้ง pack **Keil::STM32F4xx_DFP** ถ้ายังไม่มี
3. Options for Target → Debug → **ST-Link Debugger** → Settings → Flash Download → ติ๊ก **Reset and Run**
4. Build (F7) → Download (F8)

## 📁 Project Structure

```
├── Core/
│   ├── Inc/          main.h, stm32f4xx_hal_conf.h, stm32f4xx_it.h
│   └── Src/          main.c, stm32f4xx_it.c, stm32f4xx_hal_msp.c, system_stm32f4xx.c
├── Drivers/
│   ├── CMSIS/                  CMSIS 5.9 + STM32F411 device headers
│   └── STM32F4xx_HAL_Driver/   ST HAL (เฉพาะโมดูลที่ใช้)
├── MDK-ARM/          Keil project + startup file
└── docs/             รูป/วิดีโอประกอบ
```

## 🚀 Ideas to extend

- [ ] Exit delay 5 วินาทีหลังเปิดระบบ
- [ ] Entry delay ก่อนไซเรนดัง
- [ ] Main loop แบบ non-blocking (ไม่ใช้ `HAL_Delay`)

## 📄 License

โค้ดของโปรเจคนี้ใช้ได้อย่างอิสระ ไลบรารี STM32 HAL และ CMSIS เป็นของ STMicroelectronics และ Arm ภายใต้ BSD-3-Clause / Apache-2.0 (ดูไฟล์ LICENSE ในโฟลเดอร์ `Drivers/`)
