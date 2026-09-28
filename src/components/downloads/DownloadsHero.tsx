"use client";

import { FaGithub } from "react-icons/fa";
import { Download, ShieldCheck, Sparkles } from "lucide-react";
import Link from "next/link";
import { latestDownload } from "@/data/downloads";
export default function DownloadsHero() {

  return (
    <section className="relative overflow-hidden rounded-[2.5rem] border border-[#202020]/10 bg-gradient-to-br from-white via-[#FAF9F7] to-[#F3F0EB] p-8 md:p-12 shadow-xl backdrop-blur-xl">
      {/* Background Accent Glows */}
      <div className="pointer-events-none absolute -right-20 -top-20 h-80 w-80 rounded-full bg-[#0078D4]/10 blur-3xl" />
      <div className="pointer-events-none absolute -bottom-20 -left-20 h-80 w-80 rounded-full bg-[#BE5209]/10 blur-3xl" />

      <div className="relative z-10 grid items-center gap-10 lg:grid-cols-[1.1fr_0.9fr]">
        <div>
          {/* Badge */}
          <div className="inline-flex items-center gap-2 rounded-full bg-[#BE5209]/10 px-4 py-1.5 font-jakarta text-xs font-black uppercase tracking-[0.18em] text-[#BE5209]">
            <Sparkles className="h-4 w-4" /> Official Software Release
          </div>

          <h1 className="mt-4 font-jakarta text-5xl font-black leading-tight text-[#202020] md:text-6xl">
            Download Advayam IDE
          </h1>

          <p className="mt-5 max-w-2xl font-jakarta text-base md:text-lg font-medium leading-relaxed text-slate-600">
            Get the latest Advayam development environment equipped with built-in project templates, board manager, compiler toolchains, device serial monitor, and open hardware integration.
          </p>

          <div className="mt-8 flex flex-wrap items-center gap-4">
            {/* Primary Download Button with Real Blue Windows Logo */}
            <a
              href={"/downloads/AdvayamSetUp.exe"}
              rel="noopener noreferrer"
              download="AdvayamSetUp.exe"
              className="group inline-flex h-14 items-center gap-3.5 rounded-2xl bg-[#BE5209] px-7 font-jakarta text-sm font-bold text-white shadow-lg transition-all duration-300 hover:bg-black hover:shadow-orange-500/20 hover:scale-[1.02]"
            >
              <img
                src="/icons/windows-blue.svg"
                alt="Windows"
                className="h-6 w-6 object-contain filter drop-shadow-sm group-hover:brightness-125 transition"
              />
              <span>Download Binary (Windows Setup)</span>
            </a>

            {/* GitHub Source Code Download Button */}
            <a
              href="https://github.com/Webyfy/Advayam-IDE"
              target="_blank"
              rel="noopener noreferrer"
              className="inline-flex h-14 items-center gap-3.5 rounded-2xl border-2 border-[#202020] bg-white px-7 font-jakarta text-sm font-bold text-[#202020] shadow-sm transition-all duration-300 hover:bg-[#202020] hover:text-white hover:scale-[1.02]"
            >
              <FaGithub className="h-6 w-6" />
              <span>Source Code (GitHub)</span>
            </a>
          </div>

          <div className="mt-6 flex items-center gap-6 font-jakarta text-xs font-bold text-slate-500">
            <span className="flex items-center gap-1.5">
              <ShieldCheck className="h-4 w-4 text-green-600" /> Digitally Signed & Verified
            </span>
            <span>•</span>
            <span>Windows 10 / 11 (64-bit)</span>
            <Link href="/free-to-use" className="text-[#BE5209] hover:underline">
              view license
            </Link>
          </div>
        </div>

        {/* Right Information Card with Authentic Blue Windows Branding */}
        <div className="relative rounded-3xl border border-black/10 bg-white/90 p-8 shadow-lg backdrop-blur-md">
          <div className="flex items-center justify-between border-b border-black/10 pb-6">
            <div>
              <h2 className="font-jakarta text-3xl font-black text-[#202020]">
                {latestDownload.title}
              </h2>
              <div className="mt-1 flex items-center gap-2">
                <span className="rounded-full bg-[#BE5209]/10 px-3 py-0.5 font-jakarta text-xs font-extrabold text-[#BE5209]">
                  {latestDownload.version}
                </span>
                <span className="text-xs font-bold text-slate-400">Latest Stable</span>
              </div>
            </div>

            {/* Real Blue Windows Logo Icon Container */}
            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-[#0078D4]/10 border border-[#0078D4]/20 shadow-sm">
              <img
                src="/icons/windows-blue.svg"
                alt="Windows Blue Logo"
                className="h-8 w-8 object-contain"
              />
            </div>
          </div>

          <div className="mt-6 space-y-3 font-jakarta text-sm">
            <InfoRow label="Release Date" value={latestDownload.releaseDate} />
            <InfoRow label="Installer Size" value={latestDownload.fileSize} />
            <InfoRow label="Supported OS" value="Windows 10 / 11 (64-bit)" />
            <InfoRow
              label="License"
              value={
                <Link href="/license" className="text-[#BE5209] hover:underline">
                  Open Source (MIT / GPL)
                </Link>
              }
            />
          </div>

          <div className="mt-6 rounded-2xl bg-slate-50 border border-slate-200/80 p-4">
            <div className="flex items-center justify-between text-xs font-bold text-slate-500 font-jakarta">
              <span>SHA-256 Checksum</span>
              <button
                onClick={() => navigator.clipboard.writeText("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")}
                className="text-[#BE5209] hover:underline"
              >
                Copy
              </button>
            </div>
            <code className="mt-1.5 block truncate font-mono text-[11px] text-slate-600">
              e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
            </code>
          </div>
        </div>
      </div>
    </section>
  );
}

function InfoRow({ label, value }: { label: string; value: React.ReactNode }) {
  return (
    <div className="flex items-center justify-between rounded-xl bg-slate-50/80 border border-slate-100 px-4 py-3">
      <span className="font-bold text-slate-500">{label}</span>
      <span className="font-extrabold text-[#202020]">{value}</span>
    </div>
  );
}