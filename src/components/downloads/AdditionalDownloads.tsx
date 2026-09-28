import { additionalDownloads } from "@/data/downloads";
import DownloadCard from "./DownloadCard";

export default function AdditionalDownloads() {
  return (
    <section className="mt-16">
      <div className="flex flex-col md:flex-row md:items-end justify-between gap-6">
        <div>
          <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
            Ecosystem Resources
          </span>
          <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-white">
            Additional Software & Hardware Downloads
          </h2>
        </div>

        <p className="max-w-xl font-jakarta text-sm font-medium leading-relaxed text-slate-400">
          Explore supporting developer packages, open-hardware PCB Gerber files, board support libraries, driver bundles, and VS Code extensions.
        </p>
      </div>

      <div className="mt-8 grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
        {additionalDownloads.map((item) => (
          <DownloadCard key={item.title} {...item} />
        ))}
      </div>
    </section>
  );
}