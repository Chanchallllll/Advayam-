import { FaGithub } from "react-icons/fa";
import { Download, FileCode, Cpu, Layers } from "lucide-react";

type DownloadCardProps = {
  title: string;
  description: string;
  version: string;
  fileSize: string;
  href: string;
};

export default function DownloadCard({
  title,
  description,
  version,
  fileSize,
  href,
}: DownloadCardProps) {
  const isGitHub = href.includes("github.com");
  const isHardware = href.includes("board.svg") || title.toLowerCase().includes("pcb");

  return (
    <article className="group flex flex-col justify-between rounded-[2rem] border border-black/10 bg-white p-7 shadow-sm transition-all duration-300 hover:-translate-y-1 hover:shadow-xl hover:border-[#BE5209]/30">
      <div>
        <div className="flex items-start justify-between gap-4">
          {/* Icon Badge */}
          <div className="flex h-13 w-13 shrink-0 items-center justify-center rounded-2xl bg-slate-100 p-3 group-hover:bg-[#BE5209]/10 transition-colors">
            {isGitHub ? (
              <FaGithub className="h-6 w-6 text-[#202020]" />
            ) : isHardware ? (
              <Cpu className="h-6 w-6 text-[#BE5209]" />
            ) : (
              <img
                src="/icons/windows-blue.svg"
                alt="Windows"
                className="h-6 w-6 object-contain"
              />
            )}
          </div>

          <span className="rounded-full bg-[#BE5209]/10 px-3.5 py-1 font-jakarta text-xs font-bold text-[#BE5209]">
            {version}
          </span>
        </div>

        <h3 className="mt-5 font-jakarta text-xl font-bold text-[#202020] leading-snug group-hover:text-[#BE5209] transition-colors">
          {title}
        </h3>

        <p className="mt-3 font-jakarta text-sm font-medium leading-relaxed text-slate-600">
          {description}
        </p>
      </div>

      <div className="mt-8 flex items-center justify-between border-t border-slate-100 pt-5">
        <span className="font-jakarta text-xs font-bold text-slate-500">
          {fileSize}
        </span>

        <a
          href={href}
          target="_blank"
          rel="noopener noreferrer"
          download={!isGitHub}
          className="inline-flex h-11 items-center gap-2 rounded-xl bg-[#202020] px-5 font-jakarta text-xs font-bold text-white transition-all duration-300 hover:bg-[#BE5209] hover:shadow-md"
        >
          <Download className="h-4 w-4" />
          <span>{isGitHub ? "View Source" : "Download"}</span>
        </a>
      </div>
    </article>
  );
}