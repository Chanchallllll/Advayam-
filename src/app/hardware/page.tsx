"use client";

import { useState } from "react";
import Header from "@/components/Header";
import Footer from "@/components/Footer";
import {
  Cpu,
  Zap,
  Wifi,
  ShieldCheck,
  Layers,
  Download,
  ArrowRight,
  Terminal,
  FileCode,
  CheckCircle2,
  Radio,
  Sparkles,
} from "lucide-react";
import { Metadata } from "next";

const hardwareProducts = [
  {
    id: "core-board-v1",
    name: "Advayam Core Board v1.0",
    badge: "Flagship Dev Board",
    category: "Microcontroller Board",
    description:
      "Dual-core RISC-V / ARM microcontroller board with onboard Wi-Fi 4, Bluetooth 5.0 (BLE), USB-C programming interface, ultra-low power PMIC, RGB LED, and QWIIC/I2C expansion port.",
    specs: [
      { label: "Processor", value: "Dual-Core RISC-V @ 240MHz" },
      { label: "Memory", value: "8 MB PSRAM + 16 MB Flash" },
      { label: "Wireless", value: "Wi-Fi 802.11 b/g/n + BLE 5.0" },
      { label: "I/O Headers", value: "32 GPIOs, 4x SPI, 3x UART, 2x I2C" },
      { label: "Power Input", value: "5V USB-C / 3.3V LiPo Battery" },
    ],
    image: "/products/board.svg",
    schematicUrl: "/products/board.svg",
  },
  {
    id: "explorer-base",
    name: "Advayam Explorer Base Board",
    badge: "Prototyping Shield",
    category: "Expansion Base",
    description:
      "Full-featured docking baseboard with 0.96\" OLED screen header, onboard USB-to-UART bridge, breadboard expansion zone, motor driver sockets, and Grove sensor connectors.",
    specs: [
      { label: "Display Port", value: "0.96\" I2C OLED (128x64)" },
      { label: "Motor Driver", value: "Dual H-Bridge Socket (L298N/TB6612)" },
      { label: "Sensors", value: "6x Qwiic / 4x Analog Channels" },
      { label: "Debug Interface", value: "SWD / JTAG Dedicated Pins" },
      { label: "Form Factor", value: "Standard 100mm x 80mm PCB" },
    ],
    image: "/products/board.svg",
    schematicUrl: "/products/board.svg",
  },
  {
    id: "relay-automation",
    name: "Advayam Industrial Relay & Modbus Shield",
    badge: "Industrial Automation",
    category: "Automation Shield",
    description:
      "4-channel optically isolated 250V AC / 10A relay board featuring RS485 Modbus RTU communication interface for factory automation, smart grid, and IoT control.",
    specs: [
      { label: "Relay Outputs", value: "4x 250V AC / 10A SPDT" },
      { label: "Isolation", value: "3.75kV Optocoupler Protection" },
      { label: "Bus Protocol", value: "RS485 Modbus RTU / UART" },
      { label: "Power Supply", value: "12V - 24V DC Industrial DIN" },
      { label: "Status LEDs", value: "Individual Channel Relay LEDs" },
    ],
    image: "/products/board.svg",
    schematicUrl: "/products/board.svg",
  },
  {
    id: "edge-ai-vision",
    name: "Advayam Edge AI & Vision Module",
    badge: "Machine Learning",
    category: "AI Expansion",
    description:
      "High-speed Vision & Audio AI addon board with OV2640 camera interface, I2S digital microphone, microSD storage socket, and neural network inference hardware accelerator.",
    specs: [
      { label: "Camera Port", value: "DVP 24-Pin (OV2640 / OV5640)" },
      { label: "Audio Input", value: "I2S Digital MEMS Microphone" },
      { label: "AI Accelerator", value: "Local TinyML Neural Hardware" },
      { label: "Storage", value: "MicroSD Card Slot (up to 64GB)" },
      { label: "Frameworks", value: "TensorFlow Lite Micro, Edge Impulse" },
    ],
    image: "/products/board.svg",
    schematicUrl: "/products/board.svg",
  },
];

export default function HardwarePage() {
  const [selectedProduct, setSelectedProduct] = useState(hardwareProducts[0]);

  return (
    <main className="flex min-h-screen flex-col bg-black text-white">
      <Header />

      {/* Hero Section */}
      <section className="relative overflow-hidden bg-white px-6 pb-20 pt-36">
        <div className="mx-auto max-w-7xl">
          <div className="inline-flex items-center gap-2 rounded-full bg-[#BE5209]/10 px-4 py-1.5 font-jakarta text-xs font-bold uppercase tracking-wider text-[#BE5209]">
            <Cpu className="h-4 w-4" /> Open Hardware Ecosystem
          </div>

          <h1 className="mt-4 font-jakarta text-5xl md:text-6xl font-black leading-tight text-[#202020]">
            Advayam Hardware Ecosystem
          </h1>

          <p className="mt-5 max-w-3xl font-jakarta text-lg font-medium leading-relaxed text-slate-600">
            Indigenous, high-performance embedded development boards, modular prototyping shields, and open hardware schematics designed and manufactured in India for learning, Edge AI, IoT, and industrial automation.
          </p>

          {/* Quick Specs Badges */}
          <div className="mt-8 flex flex-wrap gap-3 font-jakarta">
            <span className="inline-flex items-center gap-2 rounded-xl bg-slate-100 border border-slate-200 px-4 py-2 text-xs font-bold text-slate-700">
              <Zap className="h-4 w-4 text-[#BE5209]" /> RISC-V & ARM Architectures
            </span>
            <span className="inline-flex items-center gap-2 rounded-xl bg-slate-100 border border-slate-200 px-4 py-2 text-xs font-bold text-slate-700">
              <Wifi className="h-4 w-4 text-[#BE5209]" /> Wi-Fi 4 + BLE 5.0 Wireless
            </span>
            <span className="inline-flex items-center gap-2 rounded-xl bg-slate-100 border border-slate-200 px-4 py-2 text-xs font-bold text-slate-700">
              <ShieldCheck className="h-4 w-4 text-[#BE5209]" /> Industrial 3.75kV Isolation
            </span>
            <span className="inline-flex items-center gap-2 rounded-xl bg-slate-100 border border-slate-200 px-4 py-2 text-xs font-bold text-slate-700">
              <FileCode className="h-4 w-4 text-[#BE5209]" /> Open Gerber & KiCAD Files
            </span>
          </div>
        </div>
      </section>

      {/* Main Hardware Interactive Showcase */}
      <section className="px-6 py-16">
        <div className="mx-auto max-w-7xl">
          <div className="mb-10 text-left">
            <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
              Board Specifications
            </span>
            <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-[#202020]">
              Explore Hardware Boards & Shields
            </h2>
          </div>

          {/* Navigation Tabs */}
          <div className="flex overflow-x-auto gap-3 pb-4 scrollbar-none border-b border-black/10">
            {hardwareProducts.map((prod) => (
              <button
                key={prod.id}
                onClick={() => setSelectedProduct(prod)}
                className={`whitespace-nowrap rounded-2xl px-6 py-3 font-jakarta text-sm font-bold transition-all ${
                  selectedProduct.id === prod.id
                    ? "bg-[#BE5209] text-white shadow-md"
                    : "bg-white text-slate-700 hover:bg-slate-200 border border-black/10"
                }`}
              >
                {prod.name}
              </button>
            ))}
          </div>

          {/* Featured Selected Board Showcase */}
          <div className="mt-8 rounded-3xl border-2 border-black/10 bg-white p-8 md:p-12 shadow-lg">
            <div className="grid gap-12 lg:grid-cols-[1fr_1.1fr] items-center">
              <div>
                <span className="rounded-full bg-[#BE5209]/10 px-3.5 py-1 font-jakarta text-xs font-bold text-[#BE5209]">
                  {selectedProduct.badge}
                </span>

                <h3 className="mt-4 font-jakarta text-3xl font-extrabold text-[#202020]">
                  {selectedProduct.name}
                </h3>

                <p className="mt-4 font-jakarta text-slate-600 text-base leading-relaxed">
                  {selectedProduct.description}
                </p>

                {/* Specs Table */}
                <div className="mt-8 space-y-3 font-jakarta">
                  {selectedProduct.specs.map((spec) => (
                    <div
                      key={spec.label}
                      className="flex items-center justify-between rounded-xl bg-slate-50 border border-slate-200/80 px-4 py-3 text-sm"
                    >
                      <span className="font-bold text-slate-500">{spec.label}</span>
                      <span className="font-extrabold text-[#202020] text-right">{spec.value}</span>
                    </div>
                  ))}
                </div>

                <div className="mt-8 flex flex-wrap gap-4">
                  <a
                    href={selectedProduct.schematicUrl}
                    target="_blank"
                    rel="noopener noreferrer"
                    download
                    className="inline-flex items-center gap-2 rounded-2xl bg-[#BE5209] px-6 py-3.5 font-jakarta text-sm font-bold text-white shadow-md hover:bg-black transition-colors"
                  >
                    <Download className="h-4 w-4" /> Download Hardware Schematics (PDF)
                  </a>
                  <a
                    href="/downloads"
                    className="inline-flex items-center gap-2 rounded-2xl border-2 border-[#202020] bg-white px-6 py-3.5 font-jakarta text-sm font-bold text-[#202020] hover:bg-[#202020] hover:text-white transition-colors"
                  >
                    Get Board Support Package <ArrowRight className="h-4 w-4" />
                  </a>
                </div>
              </div>

              {/* Hardware PCB Image Presentation Card */}
              <div className="relative group">
                <div className="absolute -inset-3 rounded-3xl bg-gradient-to-r from-[#BE5209]/20 to-orange-400/20 blur-2xl opacity-70 group-hover:opacity-100 transition duration-500" />
                <div className="relative rounded-3xl border border-black/10 bg-gradient-to-b from-slate-50 to-white p-8 shadow-xl">
                  <img
                    src={selectedProduct.image}
                    alt={selectedProduct.name}
                    className="w-full h-auto object-contain transition-transform duration-500 group-hover:scale-105"
                  />
                  <div className="mt-6 flex items-center justify-between border-t border-slate-200 pt-4 font-jakarta text-xs font-bold text-slate-500">
                    <span>Designed by Advayam Tech</span>
                    <span className="text-[#BE5209]">Made in India 🇮🇳</span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* Open Hardware & Gerber Files Download Section */}
      <section className="bg-white px-6 py-16">
        <div className="mx-auto max-w-7xl">
          <div className="text-left">
            <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
              Hardware Design Files
            </span>
            <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-[#202020]">
              Open-Hardware Design & Manufacturing Files
            </h2>
            <p className="mt-2 max-w-2xl font-jakarta text-sm font-medium text-slate-600">
              Download complete production-ready KiCAD project files, Gerber packages, bill of materials (BOM), and mechanical STEP models.
            </p>
          </div>

          <div className="mt-8 grid gap-6 md:grid-cols-3">
            <div className="rounded-3xl border-2 border-black/10 bg-slate-50 p-7 flex flex-col justify-between hover:border-[#BE5209] transition-colors">
              <div>
                <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-[#BE5209]/10 text-[#BE5209]">
                  <FileCode className="h-6 w-6" />
                </div>
                <h3 className="mt-5 font-jakarta text-xl font-bold text-[#202020]">
                  KiCAD Schematic & Layout
                </h3>
                <p className="mt-2 font-jakarta text-xs font-medium text-slate-600 leading-relaxed">
                  Complete PCB layout files, component symbol libraries, trace routing, and KiCAD project sources for Advayam Core v1.0.
                </p>
              </div>
              <a
                href="/products/board.svg"
                download
                className="mt-6 inline-flex items-center gap-2 text-xs font-bold text-[#BE5209] hover:underline font-jakarta"
              >
                Download KiCAD Package (.zip) <ArrowRight className="h-4 w-4" />
              </a>
            </div>

            <div className="rounded-3xl border-2 border-black/10 bg-slate-50 p-7 flex flex-col justify-between hover:border-[#BE5209] transition-colors">
              <div>
                <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-[#BE5209]/10 text-[#BE5209]">
                  <Layers className="h-6 w-6" />
                </div>
                <h3 className="mt-5 font-jakarta text-xl font-bold text-[#202020]">
                  Gerber Production Files
                </h3>
                <p className="mt-2 font-jakarta text-xs font-medium text-slate-600 leading-relaxed">
                  2-layer & 4-layer industrial Gerber files, drill files, silkscreen layers, and pick-and-place CPL files ready for SMT manufacturing.
                </p>
              </div>
              <a
                href="/products/board.svg"
                download
                className="mt-6 inline-flex items-center gap-2 text-xs font-bold text-[#BE5209] hover:underline font-jakarta"
              >
                Download Gerber Files (.zip) <ArrowRight className="h-4 w-4" />
              </a>
            </div>

            <div className="rounded-3xl border-2 border-black/10 bg-slate-50 p-7 flex flex-col justify-between hover:border-[#BE5209] transition-colors">
              <div>
                <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-[#BE5209]/10 text-[#BE5209]">
                  <Terminal className="h-6 w-6" />
                </div>
                <h3 className="mt-5 font-jakarta text-xl font-bold text-[#202020]">
                  Datasheets & Pinout Diagrams
                </h3>
                <p className="mt-2 font-jakarta text-xs font-medium text-slate-600 leading-relaxed">
                  High-resolution pinout posters, GPIO mapping charts, electrical power specs, and microcontroller datasheet references.
                </p>
              </div>
              <a
                href="/products/board.svg"
                download
                className="mt-6 inline-flex items-center gap-2 text-xs font-bold text-[#BE5209] hover:underline font-jakarta"
              >
                Download Pinout Vector PDF <ArrowRight className="h-4 w-4" />
              </a>
            </div>
          </div>
        </div>
      </section>

      <Footer />
    </main>
  );
}
