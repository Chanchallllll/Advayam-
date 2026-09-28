"use client";

import CommunityHeader from "@/components/community/CommunityHeader";
import { Check, Copy, Download } from "lucide-react";
import { useState, useEffect, use } from "react";
import { demoPrograms } from "@/data/community";

export default function DynamicProgramPage({
  params: paramsPromise,
}: {
  params: Promise<{ id: string }>;
}) {
  // Unwrap params using React.use() as required in Next.js 15/16
  const params = use(paramsPromise);
  const idStr = params.id;
  const id = parseInt(idStr, 10);

  const [code, setCode] = useState<string>("// Loading code...");
  const [copied, setCopied] = useState(false);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(false);

  const programInfo = demoPrograms.find((p) => p.id === id);

  useEffect(() => {
    if (!id) return;

    setLoading(true);
    fetch(`/Programs/Program${id}.cpp`)
      .then((res) => {
        if (!res.ok) throw new Error("Failed to load program");
        return res.text();
      })
      .then((text) => {
        setCode(text);
        setLoading(false);
      })
      .catch(() => {
        setError(true);
        setLoading(false);
      });
  }, [id]);

  async function copyCode() {
    await navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 1500);
  }

  function downloadCode() {
    const blob = new Blob([code], { type: "text/plain" });
    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = url;
    link.download = `Program${id}.cpp`;
    link.click();

    URL.revokeObjectURL(url);
  }

  if (loading) {
    return (
      <main className="relative min-h-screen bg-white text-[#202020]">
        <CommunityHeader />
        <section className="relative z-10 mx-auto max-w-[1100px] px-8 pb-20 pt-[145px] text-center">
          <p className="font-jakarta text-lg font-medium text-[#202020]/70">
            Loading program code...
          </p>
        </section>
      </main>
    );
  }

  if (error || !programInfo) {
    return (
      <main className="relative min-h-screen bg-white text-[#202020]">
        <CommunityHeader />
        <section className="relative z-10 mx-auto max-w-[1100px] px-8 pb-20 pt-[145px] text-center">
          <h1 className="font-jakarta text-3xl font-black text-red-600">
            Program Not Found
          </h1>
          <p className="mt-4 font-jakarta text-base font-medium text-[#202020]/70">
            The requested demo program could not be loaded.
          </p>
        </section>
      </main>
    );
  }

  return (
    <main className="relative min-h-screen overflow-hidden bg-white text-[#202020]">
      <CommunityHeader />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none fixed right-0 top-[92px] z-0 h-[430px] w-[520px] object-contain object-right opacity-120"
      />

      <section className="relative z-10 mx-auto max-w-[1100px] px-8 pb-20 pt-[145px]">
        <p className="font-jakarta text-sm font-black uppercase tracking-[0.18em] text-[#d95b00]">
          Demo Program {id} — {programInfo.level}
        </p>

        <h1 className="mt-4 font-jakarta text-5xl font-black">
          {programInfo.title}
        </h1>

        <p className="mt-5 max-w-3xl font-jakarta text-base font-medium leading-7 text-[#202020]/70">
          {programInfo.description}
        </p>

        <div className="mt-8 flex-1 overflow-hidden rounded-[28px] border border-[#202020]/15 bg-[#202020] shadow-sm">
          <div className="flex items-center justify-between border-b border-white/10 px-5 py-4">
            <span className="font-jakarta text-sm font-black text-white">
              Program{id}.cpp
            </span>

            <div className="flex items-center gap-3">
              <button
                type="button"
                onClick={copyCode}
                className="flex h-10 items-center gap-2 rounded-xl bg-white px-4 font-jakarta text-xs font-black text-[#202020] cursor-pointer hover:bg-neutral-100 transition"
              >
                {copied ? (
                  <Check className="h-4 w-4" />
                ) : (
                  <Copy className="h-4 w-4" />
                )}
                {copied ? "Copied" : "Copy Code"}
              </button>

              <button
                type="button"
                onClick={downloadCode}
                className="flex h-10 items-center gap-2 rounded-xl bg-[#d95b00] px-4 font-jakarta text-xs font-black text-white cursor-pointer hover:bg-[#c85a0a] transition"
              >
                <Download className="h-4 w-4" />
                Download
              </button>
            </div>
          </div>

          <pre className="h-full overflow-auto p-6 text-sm leading-7 text-[#ECECEC]">
            <code>{code}</code>
          </pre>
        </div>
      </section>
    </main>
  );
}
