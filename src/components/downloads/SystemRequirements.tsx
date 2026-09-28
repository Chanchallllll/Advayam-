import { Cpu, HardDrive, Monitor, Wifi, Usb, ShieldCheck } from "lucide-react";
import { systemRequirements } from "@/data/downloads";

export default function SystemRequirements() {
  return (
    <section className="mt-20">
      <div className="mb-8">
        <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
          Compatibility
        </span>

        <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-white">
          System Requirements
        </h2>
        <p className="mt-2 max-w-2xl font-jakarta text-sm font-medium text-slate-400">
          Ensure your system meets the hardware and software specifications for optimal performance.
        </p>
      </div>

      <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
        {/* OS Requirement Card featuring Authentic Blue Windows Logo */}
        <div className="rounded-2xl border border-black/10 bg-white p-6 shadow-sm flex items-start gap-4 transition hover:shadow-md">
          <div className="flex h-12 w-12 shrink-0 items-center justify-center rounded-xl bg-[#0078D4]/10 p-2.5">
            <img src="/icons/windows-blue.svg" alt="Windows OS" className="h-6 w-6 object-contain" />
          </div>
          <div>
            <h3 className="font-jakarta text-xs font-bold uppercase tracking-wider text-slate-400">Operating System</h3>
            <p className="mt-1 font-jakarta text-base font-bold text-[#202020]">Windows 10 / 11 (64-bit)</p>
          </div>
        </div>

        <RequirementCard
          icon={<Cpu className="h-6 w-6 text-[#BE5209]" />}
          title="Processor"
          value={systemRequirements.processor}
        />

        <RequirementCard
          icon={<Cpu className="h-6 w-6 text-[#BE5209]" />}
          title="System Memory (RAM)"
          value={systemRequirements.ram}
        />

        <RequirementCard
          icon={<HardDrive className="h-6 w-6 text-[#BE5209]" />}
          title="Available Storage"
          value={systemRequirements.storage}
        />

        <RequirementCard
          icon={<Monitor className="h-6 w-6 text-[#BE5209]" />}
          title="Display Resolution"
          value={systemRequirements.display}
        />

        <RequirementCard
          icon={<Usb className="h-6 w-6 text-[#BE5209]" />}
          title="Hardware Interface"
          value={systemRequirements.usb}
        />
      </div>
    </section>
  );
}

function RequirementCard({
  icon,
  title,
  value,
}: {
  icon: React.ReactNode;
  title: string;
  value: string;
}) {
  return (
    <div className="rounded-2xl border border-black/10 bg-white p-6 shadow-sm flex items-start gap-4 transition hover:shadow-md">
      <div className="flex h-12 w-12 shrink-0 items-center justify-center rounded-xl bg-slate-100">
        {icon}
      </div>
      <div>
        <h3 className="font-jakarta text-xs font-bold uppercase tracking-wider text-slate-400">{title}</h3>
        <p className="mt-1 font-jakarta text-base font-bold text-[#202020]">{value}</p>
      </div>
    </div>
  );
}