"use client";

import { useState } from "react";
import {
  MapPin,
  Cpu,
  Layers,
  Brain,
  Code2,
  GraduationCap,
  ChevronRight,
  CheckCircle2,
  Sparkles,
} from "lucide-react";

const reasons = [
  {
    id: "made-in-india",
    title: "Made in India",
    subtitle: "Indigenous Hardware & Software",
    icon: MapPin,
    badge: "100% Indigenous",
    content:
      "Advayam is designed and developed in India as an indigenous embedded ecosystem for students, educators, makers, researchers, startups, and industries. It supports local innovation while providing a modern alternative to traditional prototyping platforms.",
    highlights: [
      "Engineered & manufactured locally in India",
      "Tailored for Indian universities & industry standards",
      "Reduces dependency on imported hardware platforms",
    ],
  },
  {
    id: "arduino-compatible",
    title: "Arduino Compatible",
    subtitle: "Seamless Developer Transition",
    icon: Cpu,
    badge: "Plug & Play",
    content:
      "Advayam supports Arduino-compatible workflows so learners and developers can reuse familiar programming models while moving toward a more scalable embedded development platform.",
    highlights: [
      "Compatible with standard Arduino C/C++ libraries",
      "Instant board setup with zero learning curve",
      "Easy migration path to production RTOS architectures",
    ],
  },
  {
    id: "rtos-ready",
    title: "RTOS Ready",
    subtitle: "Real-Time Operating Systems",
    icon: Layers,
    badge: "FreeRTOS & Zephyr",
    content:
      "Advayam is built for real-time operating system workflows, helping users understand task scheduling, multitasking, timing constraints, embedded control, and industrial firmware design.",
    highlights: [
      "Native support for FreeRTOS & Zephyr RTOS",
      "Deterministic task scheduling & low latency",
      "Essential for robotics & industrial automation builds",
    ],
  },
  {
    id: "edge-ai-enabled",
    title: "Edge AI Enabled",
    subtitle: "On-Device Intelligence",
    icon: Brain,
    badge: "Machine Learning",
    content:
      "Advayam is prepared for Edge AI applications involving intelligent sensing, robotics, local inference, automation, and future embedded machine learning systems.",
    highlights: [
      "Accelerated local neural network inference",
      "Optimized for micro-ML models & sensor fusion",
      "Real-time data processing without cloud dependency",
    ],
  },
  {
    id: "multi-language",
    title: "Multi-Language Support",
    subtitle: "C, C++, MicroPython & Rust",
    icon: Code2,
    badge: "Polyglot SDK",
    content:
      "Advayam supports C, C++, Arduino, MicroPython, FreeRTOS, and Zephyr-based workflows, allowing beginners and advanced developers to work in the environment best suited to their goals.",
    highlights: [
      "Multi-language toolchains for all skill levels",
      "Native MicroPython REPL & script execution",
      "Comprehensive C/C++ hardware drivers",
    ],
  },
  {
    id: "classroom-to-industry",
    title: "Classroom to Industry",
    subtitle: "Scalable Career Pipeline",
    icon: GraduationCap,
    badge: "Academic & Industry",
    content:
      "Advayam scales from classroom learning to undergraduate projects, postgraduate research, maker builds, IoT products, robotics, industrial automation, and embedded product development.",
    highlights: [
      "Curriculum modules for engineering institutions",
      "Direct pathway to industrial IoT & firmware careers",
      "Production-ready reference designs & SDKs",
    ],
  },
];

export default function WhyAdvayam() {
  const [activeIndex, setActiveIndex] = useState<number>(0);
  const activeReason = reasons[activeIndex];

  return (
    <section
      id="why-advayam"
      className="relative bg-transparent py-24 sm:py-32 text-white font-jakarta overflow-hidden"
    >
      {/* Ambient background glows */}
      <div className="pointer-events-none absolute top-1/2 left-0 -translate-y-1/2 h-[500px] w-[500px] rounded-full bg-orange-500/10 blur-[140px]" />
      <div className="pointer-events-none absolute bottom-0 right-0 h-[450px] w-[450px] rounded-full bg-amber-500/10 blur-[150px]" />

      <div className="relative z-10 mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
        
        {/* Section Header */}
        <div className="text-center max-w-3xl mx-auto">
          <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/15 px-4 py-1.5 text-xs font-semibold text-orange-400">
            <Sparkles className="h-3.5 w-3.5" />
            <span>Why Choose Advayam Platform</span>
          </div>
          
          <h2 className="mt-4 font-extrabold text-3xl sm:text-5xl tracking-tight text-white">
            Why Advayam?
          </h2>
          
          <p className="mt-3.5 text-base sm:text-lg text-slate-300 font-medium leading-relaxed">
            An indigenous embedded ecosystem engineered to bridge academic learning, maker prototyping, and industrial innovation.
          </p>
        </div>

        {/* 2-Column Split Interactive Layout */}
        <div className="mt-14 grid grid-cols-1 lg:grid-cols-12 gap-8 items-start">
          
          {/* Left Column: Interactive Selector List */}
          <div className="lg:col-span-5 flex flex-col gap-3">
            {reasons.map((reason, index) => {
              const isActive = activeIndex === index;
              const Icon = reason.icon;

              return (
                <button
                  key={reason.id}
                  onClick={() => setActiveIndex(index)}
                  className={`group relative flex items-center justify-between w-full p-4 text-left rounded-2xl transition-all duration-200 border ${
                    isActive
                      ? "bg-white/[0.1] border-orange-500 shadow-lg shadow-orange-500/15 ring-1 ring-orange-500/40"
                      : "bg-white/[0.04] border-white/10 hover:bg-white/[0.08] hover:border-white/20 shadow-sm"
                  }`}
                >
                  <div className="flex items-center gap-3.5">
                    {/* Icon Container */}
                    <div
                      className={`flex h-11 w-11 shrink-0 items-center justify-center rounded-xl transition-all duration-200 ${
                        isActive
                          ? "bg-gradient-to-br from-[#EA580C] to-[#F97316] text-white shadow-md shadow-orange-500/30"
                          : "bg-white/10 text-slate-300 group-hover:bg-orange-500/20 group-hover:text-[#EA580C]"
                      }`}
                    >
                      <Icon className="h-5 w-5" />
                    </div>

                    {/* Text Details */}
                    <div>
                      <h3
                        className={`text-base font-bold transition-colors ${
                          isActive ? "text-white" : "text-slate-200 group-hover:text-white"
                        }`}
                      >
                        {reason.title}
                      </h3>
                      <p className={`text-xs font-medium ${isActive ? "text-orange-200/80" : "text-slate-400"}`}>
                        {reason.subtitle}
                      </p>
                    </div>
                  </div>

                  {/* Arrow Indicator */}
                  <div
                    className={`flex h-8 w-8 shrink-0 items-center justify-center rounded-full transition-all duration-200 ${
                      isActive
                        ? "bg-orange-500/20 text-orange-400 translate-x-0.5"
                        : "text-slate-500 group-hover:text-slate-300 group-hover:translate-x-0.5"
                    }`}
                  >
                    <ChevronRight className="h-4 w-4" />
                  </div>
                </button>
              );
            })}
          </div>

          {/* Right Column: Dynamic Feature Detail Card */}
          <div className="lg:col-span-7 sticky top-28">
            <div
              key={activeReason.id}
              className="relative rounded-3xl border border-white/15 bg-white/[0.04] backdrop-blur-2xl p-7 sm:p-10 shadow-2xl transition-all duration-300 animate-in fade-in slide-in-from-right-3"
            >
              {/* Badge Tag */}
              <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/15 px-3.5 py-1 text-xs font-bold text-orange-400">
                <span>{activeReason.badge}</span>
              </div>

              {/* Title */}
              <h3 className="mt-4 text-2xl sm:text-3xl font-extrabold text-white">
                {activeReason.title}
              </h3>

              {/* Content Description */}
              <p className="mt-4 text-base sm:text-lg text-slate-300 leading-relaxed font-medium">
                {activeReason.content}
              </p>

              {/* Key Bullet Highlights */}
              <div className="mt-7 pt-7 border-t border-white/10 space-y-3.5">
                <h4 className="text-xs font-bold uppercase tracking-wider text-slate-400">
                  Key Capabilities
                </h4>
                
                {activeReason.highlights.map((highlight, i) => (
                  <div key={i} className="flex items-start gap-3">
                    <CheckCircle2 className="h-5 w-5 shrink-0 text-[#EA580C] mt-0.5" />
                    <span className="text-sm font-semibold text-slate-200">
                      {highlight}
                    </span>
                  </div>
                ))}
              </div>

              {/* Bottom Action Footer */}
              <div className="mt-8 pt-6 border-t border-white/10 flex items-center justify-between">
                <span className="text-xs font-semibold text-slate-400">
                  Advayam Platform Architecture
                </span>
                <a
                  href="/learningpath"
                  className="inline-flex items-center gap-1.5 text-xs font-bold text-orange-400 hover:text-orange-300 transition-colors"
                >
                  <span>Explore Learning Paths</span>
                  <ChevronRight className="h-3.5 w-3.5" />
                </a>
              </div>

            </div>
          </div>

        </div>

      </div>
    </section>
  );
}
