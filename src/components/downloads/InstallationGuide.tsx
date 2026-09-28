import Link from "next/link";
import {
  CheckCircle2,
  FileText,
  HelpCircle,
  PlugZap,
  Settings,
  Terminal,
  ArrowRight,
} from "lucide-react";

const installSteps = [
  {
    step: "01",
    title: "Download installer or source code",
    description:
      "Download the Windows installer executable (v1.0.0) or clone the open-source repository from GitHub.",
    icon: FileText,
  },
  {
    step: "02",
    title: "Run guided setup wizard",
    description:
      "Launch the installer setup and follow the step-by-step instructions to configure Advayam IDE.",
    icon: Settings,
  },
  {
    step: "03",
    title: "Install board drivers & BSP",
    description:
      "Install the USB serial driver package so Advayam development boards are automatically detected.",
    icon: PlugZap,
  },
  {
    step: "04",
    title: "Build and flash your first project",
    description:
      "Open Advayam IDE, select your board model, compile firmware, and upload to the hardware.",
    icon: Terminal,
  },
];

const troubleshooting = [
  {
    problem: "Board not detected over USB",
    solution: "Reinstall the USB serial driver package and check physical USB cable connection.",
  },
  {
    problem: "Firmware upload timeout",
    solution: "Verify selected COM port in Board Manager and close any open external serial monitors.",
  },
  {
    problem: "Compilation error",
    solution: "Check toolchain packages in Advayam IDE settings and verify target architecture.",
  },
];

export default function InstallationGuide() {
  return (
    <section className="mt-20">
      <div className="mb-8">
        <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
          Quick Start
        </span>

        <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-white">
          Installation & Setup Guide
        </h2>

        <p className="mt-2 max-w-3xl font-jakarta text-sm font-medium text-slate-400">
          Follow these 4 simple steps to set up Advayam IDE and begin building embedded projects.
        </p>
      </div>

      {/* Step Cards Grid */}
      <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-4">
        {installSteps.map((item) => {
          const Icon = item.icon;
          return (
            <div
              key={item.step}
              className="relative flex flex-col justify-between rounded-3xl border border-black/10 bg-white p-7 shadow-sm transition hover:shadow-lg"
            >
              <div>
                <div className="flex items-center justify-between">
                  <span className="font-jakarta text-2xl font-black text-[#BE5209]">
                    {item.step}
                  </span>
                  <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-slate-100 text-[#202020]">
                    <Icon className="h-5 w-5" />
                  </div>
                </div>

                <h3 className="mt-5 font-jakarta text-lg font-bold text-[#202020]">
                  {item.title}
                </h3>

                <p className="mt-2 font-jakarta text-xs font-medium leading-relaxed text-slate-600">
                  {item.description}
                </p>
              </div>
            </div>
          );
        })}
      </div>

      {/* Troubleshooting Section */}
      <div className="mt-12 rounded-3xl border border-black/10 bg-slate-50 p-8">
        <div className="flex items-center gap-3">
          <HelpCircle className="h-6 w-6 text-[#BE5209]" />
          <h3 className="font-jakarta text-xl font-bold text-[#202020]">Common Troubleshooting Steps</h3>
        </div>

        <div className="mt-6 grid gap-4 md:grid-cols-3 font-jakarta">
          {troubleshooting.map((item, idx) => (
            <div key={idx} className="rounded-2xl bg-white p-5 border border-slate-200 shadow-sm">
              <p className="text-xs font-bold uppercase tracking-wider text-[#BE5209]">{item.problem}</p>
              <p className="mt-2 text-xs font-medium text-slate-700 leading-relaxed">{item.solution}</p>
            </div>
          ))}
        </div>

        <div className="mt-8 flex flex-wrap items-center justify-between gap-4 border-t border-slate-200 pt-6">
          <span className="font-jakarta text-xs font-bold text-slate-600">Need additional installation help or technical support?</span>
          <Link
            href="/support"
            className="inline-flex items-center gap-2 font-jakarta text-xs font-bold text-[#BE5209] hover:underline"
          >
            Visit Support & Installation Docs <ArrowRight className="h-4 w-4" />
          </Link>
        </div>
      </div>
    </section>
  );
}