import { GraduationCap, Briefcase, Wrench, CheckCircle2, ArrowRight, Sparkles } from "lucide-react";

const paths = [
  {
    id: "students",
    title: "Students & Education",
    subtitle: "For School (K-12), Undergraduates & Postgraduate Researchers.",
    icon: GraduationCap,
    badgeBg: "bg-orange-500/10 border-orange-500/20 text-[#EA580C]",
    gradient: "from-orange-500/15 via-amber-500/5 to-transparent",
    href: "/learningpath/students",
    items: [
      "K-12 STEM & Electronics Foundations",
      "Undergraduate Microcontroller Labs",
      "Postgraduate & Research Projects",
      "Hands-on Hardware Kits & Curriculum",
    ],
  },
  {
    id: "professionals",
    title: "Professionals",
    subtitle: "For Industry Engineers, Firmware Developers & IoT Specialists.",
    icon: Briefcase,
    badgeBg: "bg-purple-500/10 border-purple-500/20 text-purple-600",
    gradient: "from-purple-500/15 via-indigo-500/5 to-transparent",
    href: "/learningpath/professionals",
    items: [
      "Industrial Automation & Control",
      "IoT Product Development & APIs",
      "RTOS Firmware & Low-Latency Tasks",
      "Edge AI & On-Device Inference",
      "Hardware Testing & Debugging Workflows",
    ],
  },
  {
    id: "makers",
    title: "Makers & Innovators",
    subtitle: "For Hobbyists, Robotics Builders & DIY Hardware Creators.",
    icon: Wrench,
    badgeBg: "bg-emerald-500/10 border-emerald-500/20 text-emerald-600",
    gradient: "from-emerald-500/15 via-teal-500/5 to-transparent",
    href: "/learningpath/makers",
    items: [
      "DIY Smart Home & Automation",
      "Robotics & Motor Control Builds",
      "Sensor Dashboards & Telemetry",
      "Wireless Bluetooth & Wi-Fi Nodes",
      "Custom PCB & Prototyping Projects",
    ],
  },
];

export default function LearningPathShowcase() {
  return (
    <section className="relative w-full overflow-hidden bg-transparent py-20 sm:py-28 text-white font-jakarta">
      {/* Background Ambient Blur */}
      <div className="pointer-events-none absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 h-[600px] w-[900px] rounded-full bg-gradient-to-r from-orange-500/8 via-amber-500/5 to-transparent blur-[160px]" />

      <div className="relative z-10 mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
        
        {/* Section Header */}
        <div className="text-center max-w-3xl mx-auto">
          <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/15 px-4 py-1.5 text-xs font-semibold text-orange-400">
            <Sparkles className="h-3.5 w-3.5" />
            <span>Tailored Learning Journeys</span>
          </div>

          <h2 className="mt-4 font-extrabold text-3xl sm:text-5xl tracking-tight text-white">
            Choose Your Embedded Journey
          </h2>

          <p className="mt-3.5 text-base sm:text-lg text-slate-300 font-medium leading-relaxed">
            Structured learning pathways engineered for students, industry engineers, and hardware makers.
          </p>
        </div>

        {/* 3-Column Learning Path Cards */}
        <div className="mt-14 grid grid-cols-1 md:grid-cols-3 gap-8">
          {paths.map((path) => {
            const Icon = path.icon;

            return (
              <div
                key={path.id}
                className="group relative flex flex-col justify-between rounded-3xl border border-white/15 bg-white/[0.04] backdrop-blur-xl p-7 sm:p-8 shadow-2xl transition-all duration-300 hover:shadow-orange-500/10 hover:border-white/30 hover:bg-white/[0.07] hover:-translate-y-2"
              >
                <div>
                  {/* Card Header Top Row */}
                  <div className="flex items-center justify-between">
                    <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-orange-500/15 text-orange-400 group-hover:scale-110 transition-transform duration-300">
                      <Icon className="h-6 w-6" />
                    </div>

                  </div>

                  {/* Title & Subtitle */}
                  <h3 className="mt-6 text-2xl font-extrabold text-white group-hover:text-orange-400 transition-colors">
                    {path.title}
                  </h3>

                  <p className="mt-2.5 text-xs sm:text-sm text-slate-400 font-semibold leading-relaxed">
                    {path.subtitle}
                  </p>

                  {/* Checklist Items */}
                  <div className="mt-7 pt-6 border-t border-white/10 space-y-3">
                    {path.items.map((item, i) => (
                      <div key={i} className="flex items-start gap-2.5">
                        <CheckCircle2 className="h-4 w-4 shrink-0 text-[#EA580C] mt-0.5" />
                        <span className="text-xs font-semibold text-slate-200">
                          {item}
                        </span>
                      </div>
                    ))}
                  </div>
                </div>

                {/* Integrated Action CTA Button */}
                <div className="mt-8 pt-6 border-t border-white/10">
                  <a
                    href={path.href}
                    className="inline-flex w-full items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] px-6 py-3.5 text-xs sm:text-sm font-bold text-white shadow-md shadow-orange-500/20 transition-all duration-200 hover:shadow-lg hover:shadow-orange-500/30 hover:scale-[1.01] active:scale-[0.99]"
                  >
                    <span>Explore Learning Path</span>
                    <ArrowRight className="h-4 w-4 transition-transform duration-200 group-hover:translate-x-1" />
                  </a>
                </div>

              </div>
            );
          })}
        </div>

      </div>
    </section>
  );
}
