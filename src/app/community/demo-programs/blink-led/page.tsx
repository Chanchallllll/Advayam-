"use client";

import CommunityHeader from "@/components/community/CommunityHeader";
import Footer from "@/components/Footer";
import { Check, Copy, Download } from "lucide-react";
import { useState } from "react";

const blinkCode = `/*
 * =====================================================================================
 * BOARD PIN DEFINITION & FUNCTION DOCUMENTATION
 * =====================================================================================
 * Pin Type & Protocol Definitions:
 *   - UART    : Universal Asynchronous Receiver-Transmitter (TX/RX serial lines for communication).
 *   - SPI     : Serial Peripheral Interface (SCK, MOSI, MISO, CS for high-speed sensor/display buses).
 *   - I²C     : Inter-Integrated Circuit (SDA, SCL lines for multi-device peripheral buses).
 *   - Digital : General Purpose Input/Output (GPIO) for binary state reading/control (HIGH/LOW).
 *   - PWM     : Pulse Width Modulation for precise analog-like signal output (brightness, tone, speed).
 *   - ADC     : Analog-to-Digital Converter for reading continuous analog sensor voltages.
 *   - Touch   : Capacitive touch input sensing capability.
 * 
 * Supported Development Board Pin Mapping / Reference (ESP32 / Advayam Core Board):
 * -------------------------------------------------------------------------------------
 * Pin        UART    SPI     I²C     Digital PWM     ADC/Touch  Function / Hardware Mapping
 * -------------------------------------------------------------------------------------
 * GPIO1      TX      —       —       ✓       ✓       ADC        Analog Temp Sensor / Serial TX
 * GPIO3      RX      —       —       ✓       ✓       —          Serial RX
 * GPIO7      —       —       —       ✓       ✓       —          5x5 RGB Matrix Data (WS2812B)
 * GPIO8      —       —       —       ✓       —       Touch      Capacitive Touch Sensor Input
 * GPIO10     —       CS      —       ✓       ✓       —          SPI Chip Select (LSM6DSOX IMU CS)
 * GPIO11     —       MOSI    —       ✓       ✓       —          SPI MOSI (LSM6DSOX IMU Data In)
 * GPIO12     —       SCK     —       ✓       ✓       —          SPI Clock (LSM6DSOX IMU Clock)
 * GPIO13     —       MISO    —       ✓       ✓       —          SPI MISO (LSM6DSOX IMU Data Out)
 * GPIO16     —       —       —       ✓       ✓       —          Piezo Buzzer Output
 * GPIO17     —       —       —       ✓       —       —          User Push Button Input (Pullup)
 * GPIO18     —       SCK     —       ✓       ✓       —          SPI SCK Default Pin
 * GPIO21     —       —       SDA     ✓       —       —          I²C Data Pin (SDA)
 * GPIO22     —       —       SCL     ✓       —       —          I²C Clock Pin (SCL)
 * GPIO23     —       MOSI    —       ✓       ✓       —          SPI MOSI Default Pin
 * GPIO38     —       —       —       ✓       —       —          LSM6DSOX IMU Interrupt Pin
 * GPIO45     —       —       —       ✓       ✓       —          User Onboard LED Indicator
 * -------------------------------------------------------------------------------------
 * 
 * DEMO PROGRAM: Blink LED
 * Purpose: Turns an LED connected to GPIO pin 2 ON for 1 second and OFF for 1 second continuously.
 * Active Pins Used In This Code:
 *   - GPIO2 (LED_PIN) : Digital Output / PWM -> Onboard / External LED
 * =====================================================================================
 */

#include <Arduino.h>

#define LED_PIN 2

void setup()
{
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(1000);

    digitalWrite(LED_PIN, LOW);
    delay(1000);
}`;

export default function BlinkLedPage() {
  const [copied, setCopied] = useState(false);

  async function copyCode() {
    await navigator.clipboard.writeText(blinkCode);
    setCopied(true);
    setTimeout(() => setCopied(false), 1500);
  }

  function downloadCode() {
    const blob = new Blob([blinkCode], { type: "text/plain" });
    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = url;
    link.download = "blink-led.cpp";
    link.click();

    URL.revokeObjectURL(url);
  }

  return (
    <main className="relative flex min-h-screen flex-col overflow-x-hidden bg-white text-[#202020]">
      <CommunityHeader />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none fixed right-0 top-[92px] z-0 h-[430px] w-[520px] object-contain object-right opacity-120"
      />

      <section className="relative z-10 mx-auto max-w-[1100px] px-8 pb-20 pt-[145px]">
        <p className="font-jakarta text-sm font-black uppercase tracking-[0.18em] text-[#d95b00]">
          Demo Program
        </p>

        <h1 className="mt-4 font-jakarta text-5xl font-black">
          Blink LED
        </h1>

        <p className="mt-5 max-w-3xl font-jakarta text-base font-medium leading-7 text-[#202020]/70">
          This basic program turns an LED connected to GPIO pin 2 ON for
          one second and then OFF for one second. The `setup()` function
          configures the LED pin as an output, while the `loop()` function keeps
          repeating the blink sequence forever.
        </p>

       <div className="mt-8 flex-1 overflow-hidden rounded-[28px] border border-[#202020]/15 bg-[#202020] shadow-sm">
          <div className="flex items-center justify-between border-b border-white/10 px-5 py-4">
            <span className="font-jakarta text-sm font-black text-white">
              blink-led.cpp
            </span>

            <div className="flex items-center gap-3">
              <button
                type="button"
                onClick={copyCode}
                className="flex h-10 items-center gap-2 rounded-xl bg-white px-4 font-jakarta text-xs font-black text-[#202020]"
              >
                {copied ? (
                  <Check className="h-4 w-4" />
                ) : (
                  <Copy className="h-4 w-4" />
                )}
                {copied ? "Copied" : ""}
              </button>

              <button
                type="button"
                onClick={downloadCode}
                className="flex h-10 items-center gap-2 rounded-xl bg-[#d95b00] px-4 font-jakarta text-xs font-black text-white"
              >
                <Download className="h-4 w-4" />
                
              </button>
            </div>
          </div>

        <pre className="h-full overflow-auto p-6 text-sm leading-7 text-[#ECECEC]">
            <code>{blinkCode}</code>
          </pre>
        </div>
      </section>
      <Footer />
    </main>
  );
}