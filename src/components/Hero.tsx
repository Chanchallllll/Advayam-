"use client";

import { useEffect, useState } from "react";
import { Cpu, Zap, Wifi } from "lucide-react";
import { FaWindows } from "react-icons/fa";
import { AnimatePresence, motion } from "framer-motion";
import WireframeMesh from "@/components/WireframeMesh";
import thejas32Kicad2Image from "../../Board_Images/AdvayamThejas32_kicad2.png";
import thejas32Kicad1Image from "../../Board_Images/AdvayamThejas32_kicad1.png";
import esp32S3Image from "../../Board_Images/Advayam_ESP32_S3.png";
import starterKitImage from "../../Board_Images/starterkit.png";
import esp32S3SKImage from "../../Board_Images/advayam_ESP32S3_SK.png";
import ra4m1SKImage from "../../Board_Images/Advayam_RA4M1_SK.png";
import rp2354BSKImage from "../../Board_Images/Advayam_RP2354B_sk.png";
import rp2354BSK2Image from "../../Board_Images/RP2354B_sk2 (1).png";
import wifiShieldImage from "../../Shield images/Wifi_Ethernet_RS_new (1).png";
import plantSenseShieldImage from "../../Shield images/plantSenseControl_Shield (1).png";
import motorServoShieldImage from "../../Shield images/Motor_servo_shield (1).png";

const slides = [
  {
    id: "thejas32-sk-kicad2",
    eyebrow: "Thejas32 Expansion",
    title: "Advayam Thejas32 SK",
    description:
      "A multifunctional Arduino shield compatible board, made with an Indian processor, featuring an LED matrix, touch sensor, temperature and humidity sensor, pressure sensor, inertial sensor, and buzzer. It also provides a USB interface, GPIO headers, onboard controls, and an integrated wireless module for embedded development.",
    image: thejas32Kicad2Image.src,
    alt: "Advayam Thejas32 SK expansion board",
    label: "Advayam Thejas32 SK",
    note: "Sensors & Interfaces",
    tags: [
      { icon: Cpu, label: "Integrated Sensors" },
      { icon: Zap, label: "LED Matrix" },
      { icon: Wifi, label: "Wireless Communication" },
      { icon: Cpu, label: "Arduino Compatible Board" },
    ],
  },
  {
    id: "thejas32-sk-kicad1",
    eyebrow: "Thejas32 Expansion",
    title: "Advayam Thejas32 SK",
    description:
      "A multifunctional Arduino shield compatible board, made with an Indian processor, featuring an LED matrix, touch sensor, temperature and humidity sensor, pressure sensor, inertial sensor, and buzzer. It also provides a USB interface, GPIO headers, onboard controls, and an integrated wireless module for embedded development.",
    image: thejas32Kicad1Image.src,
    alt: "Advayam Thejas32 SK alternate board view",
    label: "Advayam Thejas32 SK",
    note: "Sensors & Interfaces",
    tags: [
      { icon: Cpu, label: "Integrated Sensors" },
      { icon: Zap, label: "LED Matrix" },
      { icon: Wifi, label: "Wireless Communication" },
      { icon: Cpu, label: "Arduino Compatible Board" },
    ],
  },
  {
    id: "esp32-s3",
    eyebrow: "Wireless Development",
    title: "Advayam Baseboard",
    description:
      "An ESP32 Baseboard for connected embedded applications, with wireless capability, USB connectivity, and accessible expansion interfaces, which is the plug-in board for the Advayam Explorer Starter Kit.",
    image: esp32S3Image.src,
    alt: "Advayam Baseboard",
    label: "Advayam Baseboard",
    note: "Connected Prototyping",
    tags: [
      { icon: Wifi, label: "Wireless Connectivity" },
      { icon: Cpu, label: "GPIO Expansion" },
      { icon: Zap, label: "USB Interface" },
    ],
  },
  {
    id: "starter-kit",
    eyebrow: "Explorer Starter Kit",
    title: "Advayam Explorer Starter Kit",
    description:
      "A feature-rich learning kit integrating an LED matrix, microphone, touch interface, sensors, buttons, buzzer, and USB connectivity.",
    image: starterKitImage.src,
    alt: "Advayam Explorer Starter Kit",
    label: "Advayam Explorer Starter Kit",
    note: "All-in-One Learning",
    tags: [
      { icon: Cpu, label: "Multiple Sensors" },
      { icon: Zap, label: "USB Connectivity" },
      { icon: Wifi, label: "Hands-on Learning" },
    ],
  },
  {
    id: "esp32-s3-sk",
    eyebrow: "Wireless Development",
    title: "Advayam Starter Kit",
    description:
      "An Advayam Starter Kit board for prototyping, with an LED matrix, wireless connectivity, USB interface, GPIO expansion, onboard sensors, and user controls.",
    image: esp32S3SKImage.src,
    alt: "Advayam Starter Kit",
    label: "Advayam Starter Kit",
    note: "Connected Prototyping",
    tags: [
      { icon: Wifi, label: "Wireless Connectivity" },
      { icon: Cpu, label: "GPIO Expansion" },
      { icon: Zap, label: "Onboard Controls" },
    ],
  },
  {
    id: "ra4m1-sk",
    eyebrow: "Microcontroller Board",
    title: "Advayam RA4M1 SK",
    description:
      "A microcontroller development board which is Arduino Shield Compatible with GPIO, analog, digital, and expansion interfaces for embedded applications.",
    image: ra4m1SKImage.src,
    alt: "Advayam RA4M1 SK development board",
    label: "Advayam RA4M1 SK",
    note: "Embedded Applications",
    tags: [
      { icon: Cpu, label: "RA4M1 MCU" },
      { icon: Zap, label: "Analog & Digital I/O" },
      { icon: Wifi, label: "Expansion Interfaces" },
      { icon: Cpu, label: "Arduino Shield Compatible" },
    ],
  },
  {
    id: "rp2354b-sk-1",
    eyebrow: "RP2354B Development",
    title: "Advayam RP2354B SK",
    description:
      "An RP2354B development board which is Arduino shield compatible with USB connectivity, GPIO headers, power input, onboard debugging interfaces, an LED matrix, touch sensor, temperature and humidity sensor, pressure sensor, inertial sensor, and buzzer.",
    image: rp2354BSKImage.src,
    alt: "Advayam RP2354B SK development board",
    label: "Advayam RP2354B SK",
    note: "USB & Debug Ready",
    tags: [
      { icon: Cpu, label: "RP2354B MCU" },
      { icon: Zap, label: "GPIO Headers" },
      { icon: Wifi, label: "Debug Interfaces" },
      { icon: Cpu, label: "Arduino Shield Compatible" },
    ],
  },
  {
    id: "rp2354b-sk-2",
    eyebrow: "RP2354B Development",
    title: "Advayam RP2354B SK",
    description:
      "An RP2354B development board which is Arduino shield compatible with USB connectivity, GPIO headers, power input, onboard debugging interfaces, an LED matrix, touch sensor, temperature and humidity sensor, pressure sensor, inertial sensor, and buzzer.",
    image: rp2354BSK2Image.src,
    alt: "Advayam RP2354B SK alternate board view",
    label: "Advayam RP2354B SK",
    note: "USB & Debug Ready",
    tags: [
      { icon: Cpu, label: "RP2354B MCU" },
      { icon: Zap, label: "GPIO Headers" },
      { icon: Wifi, label: "Debug Interfaces" },
      { icon: Cpu, label: "Arduino Shield Compatible" },
    ],
  },
  {
    id: "wifi-shield",
    eyebrow: "Communication Shield",
    title: "Wi‑Fi / Ethernet RS Communication Shield",
    description:
      "A communication-focused expansion shield designed for network and serial connectivity, supporting Wi-Fi, Ethernet, and RS interfaces for connected embedded applications.",
    image: wifiShieldImage.src,
    alt: "Wi‑Fi Ethernet RS Communication Shield",
    label: "Wireless Communication Shield",
    note: "Industrial Connectivity",
    tags: [
      { icon: Wifi, label: "Wi‑Fi & Ethernet" },
      { icon: Cpu, label: "RS Serial Support" },
      { icon: Zap, label: "Connected Systems" },
    ],
  },
  {
    id: "plantsense-shield",
    eyebrow: "PlantSense Shield",
    title: "PlantSense Control Shield",
    description:
      "A control and monitoring shield designed for PlantSense applications, featuring an onboard display and multiple interface connections for sensor-based systems. An alternate view of the shield is used in automated plant-monitoring and irrigation systems.",
    image: plantSenseShieldImage.src,
    alt: "PlantSense Control Shield",
    label: "PlantSense Control Shield",
    note: "Sensor & Display Ready",
    tags: [
      { icon: Cpu, label: "Onboard Display" },
      { icon: Wifi, label: "Sensor Interfaces" },
      { icon: Zap, label: "Monitoring Ready" },
    ],
  },
  {
    id: "motor-shield",
    eyebrow: "Motion Control Shield",
    title: "Motor & Servo Shield",
    description:
      "A dedicated motor and servo control shield providing multiple driver channels and interface connections for robotics, motion control, and automation projects.",
    image: motorServoShieldImage.src,
    alt: "Motor & Servo Shield",
    label: "Motor & Servo Shield",
    note: "Automation & Robotics",
    tags: [
      { icon: Zap, label: "Multiple Drivers" },
      { icon: Cpu, label: "Servo Control" },
      { icon: Wifi, label: "Automation Interfaces" },
    ],
  },
];

export default function Hero() {
  const [activeSlide, setActiveSlide] = useState(0);
  const [isAutoPlaying, setIsAutoPlaying] = useState(true);

  useEffect(() => {
    if (!isAutoPlaying) return;

    const timer = setInterval(() => {
      setActiveSlide((current) => (current + 1) % slides.length);
    }, 5000);

    return () => clearInterval(timer);
  }, [isAutoPlaying]);

  const currentSlide = slides[activeSlide];

  return (
    <section className="relative flex min-h-screen flex-col items-center justify-center overflow-hidden bg-transparent px-6 pt-32 pb-20 text-center">
      <WireframeMesh />

      <div className="relative z-10 mx-auto max-w-6xl translate-y-6">
        {/* Webyfy IoT Management Badge */}
        <div className="-translate-y-3">
          <a
            href="https://www.webyfy.com"
            target="_blank"
            rel="noopener noreferrer"
            className="inline-flex items-center gap-3 rounded-full bg-[#07053d] border border-white/20 px-6 py-2.5 backdrop-blur-md transition-all hover:bg-white/15 hover:border-white/30 shadow-lg"
          >
            <span className="text-xs font-bold uppercase tracking-wider text-slate-300">
              Designed and Managed By
            </span>

            <img
              src="/logos/webyfy-logo.svg"
              alt="Webyfy IoT"
              className="h-7 md:h-8 w-auto object-contain"
            />
          </a>
          <p className="font-jakarta mt-4 text-lg font-extrabold text-white">
            Open source for curious minds.
          </p>
        </div>

        <h1 className="font-druk text-[5rem] leading-none tracking-[0.08em] text-white drop-shadow-[0_4px_30px_rgba(0,0,0,0.8)] md:text-[7rem] lg:text-[9rem] mt-6">
          ADVAYAM
        </h1>
        {/* Sub-headline */}
        <h2 className="mt-4 sm:mt-6 text-xl sm:text-3xl lg:text-4xl font-bold leading-tight text-slate-200 drop-shadow-[0_2px_15px_rgba(0,0,0,0.8)] max-w-2xl mx-auto">
          India’s Embedded Ecosystem for Learning, Innovation and Product Development.
        </h2>

        <div className="mt-10 flex flex-wrap items-center justify-center gap-5">
          <a
            href="/learningpath"
            className="rounded-full bg-[#BE5209] px-5 py-2 font-jakarta text-lg font-semibold text-white transition hover:bg-orange-600 shadow-xl shadow-orange-500/30 hover:scale-[1.02]"
          >
            Get Started
          </a>

          <a
            href="/downloads"
            className="inline-flex items-center gap-3 rounded-full border-2 border-white/30 bg-black/40 backdrop-blur-md px-4 py-3 font-jakarta text-lg font-semibold text-white transition hover:bg-white/15 hover:border-white/50"
          >
            <span className="flex h-12 w-12 items-center justify-center rounded-full border-2 border-white/30 text-white">
              <FaWindows className="h-6 w-6" />
            </span>
            Get Advayam Now
          </a>
        </div>

        {/* Hardware Circuit Board PCB Showcase */}
        <div
          className="mt-14 relative mx-auto max-w-6xl overflow-hidden rounded-3xl border border-white/15 bg-black/70 backdrop-blur-2xl p-6 md:p-10 shadow-2xl text-white"
          onMouseEnter={() => setIsAutoPlaying(false)}
        >
          <AnimatePresence mode="wait" initial={false}>
            <motion.div
              key={currentSlide.id}
              initial={{ opacity: 0, x: 56 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -56 }}
              transition={{ duration: 0.55, ease: "easeInOut" }}
              className="flex flex-col lg:flex-row items-center justify-between gap-8"
            >
              {/* Left Content */}
              <div className="text-left max-w-lg">
                <div className="inline-flex items-center gap-2 rounded-full bg-[#BE5209]/20 border border-[#BE5209]/40 px-4 py-1.5 text-xs font-bold uppercase tracking-wider text-orange-400">
                  <Cpu className="h-4 w-4" /> {currentSlide.eyebrow}
                </div>
                <h3 className="mt-3 font-jakarta text-2xl md:text-3xl font-extrabold text-white leading-snug">
                  {currentSlide.title}
                </h3>
                <p className="mt-3 font-jakarta text-slate-300 text-sm md:text-base leading-relaxed">
                  {currentSlide.description}
                </p>

                <div className="mt-6 flex flex-wrap gap-2.5">
                  {currentSlide.tags.map((tag, index) => {
                    const Icon = tag.icon;

                    return (
                      <span
                        key={`${currentSlide.id}-${tag.label}-${index}`}
                        className="rounded-lg bg-white/10 border border-white/15 px-3.5 py-1.5 text-xs font-bold text-slate-200 shadow-sm flex items-center gap-1.5"
                      >
                        <Icon className="h-4 w-4 text-[#BE5209]" /> {tag.label}
                      </span>
                    );
                  })}
                </div>
              </div>

              {/* Right Hardware Image Showcase */}
              <div className="relative w-full lg:w-[600px] shrink-0 group">
                <div className="absolute inset-0 rounded-3xl bg-black/70 blur-2xl opacity-80"></div>
                <div className="relative rounded-2xl border border-white/15 bg-black/60 backdrop-blur-sm p-4 shadow-[0_0_30px_rgba(0,0,0,0.9)] hover:border-white/30 transition duration-300">
                  <img
                    src={currentSlide.image}
                    alt={currentSlide.alt}
                    className="w-full h-auto max-h-[440px] object-contain transition-transform duration-500 group-hover:scale-110"
                  />
                  <div className="mt-3 flex items-center justify-between border-t border-white/10 pt-3">
                    <span className="text-xs font-bold text-slate-400 font-jakarta">{currentSlide.label}</span>
                    <span className="text-xs font-bold text-orange-400 font-jakarta">{currentSlide.note}</span>
                  </div>
                </div>
              </div>
            </motion.div>
          </AnimatePresence>

          <div className="mt-4 flex w-full items-center justify-center gap-3">
            {slides.map((slide, index) => (
              <button
                key={`${slide.id}-indicator`}
                type="button"
                onClick={() => {
                  setIsAutoPlaying(false);
                  setActiveSlide(index);
                }}
                className={`h-2.5 rounded-full transition-all ${
                  index === activeSlide ? "w-8 bg-[#BE5209]" : "w-2.5 bg-white/30 hover:bg-white/50"
                }`}
                aria-label={`View slide ${index + 1}`}
                title={`View ${slide.title}`}
              />
            ))}
          </div>
        </div>
      </div>
    </section>
  );
}
