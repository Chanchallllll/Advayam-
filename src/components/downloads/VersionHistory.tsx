import { versionHistory, latestDownload } from "@/data/downloads";
import { Download, CheckCircle2 } from "lucide-react";

export default function VersionHistory() {
  return (
    <section className="mt-20">
      <div className="mb-8">
        <span className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
          Release History
        </span>

        <h2 className="mt-2 font-jakarta text-3xl md:text-4xl font-extrabold text-white">
          Version Changelog & Past Releases
        </h2>

        <p className="mt-2 max-w-3xl font-jakarta text-sm font-medium leading-relaxed text-slate-400">
          Comprehensive release notes and historical builds for Advayam IDE.
        </p>
      </div>

      <div className="overflow-hidden rounded-3xl border border-black/10 bg-white shadow-sm">
        <div className="overflow-x-auto">
          <table className="w-full border-collapse text-left font-jakarta">
            <thead className="bg-slate-100 border-b border-black/10">
              <tr>
                <th className="px-6 py-4 text-xs font-extrabold text-slate-700 uppercase tracking-wider">
                  Version
                </th>
                <th className="px-6 py-4 text-xs font-extrabold text-slate-700 uppercase tracking-wider">
                  Release Date
                </th>
                <th className="px-6 py-4 text-xs font-extrabold text-slate-700 uppercase tracking-wider">
                  Highlights & Changes
                </th>
                <th className="px-6 py-4 text-xs font-extrabold text-slate-700 uppercase tracking-wider text-right">
                  Download
                </th>
              </tr>
            </thead>

            <tbody className="divide-y divide-slate-100">
              {versionHistory.map((release, index) => (
                <tr key={release.version} className="hover:bg-slate-50/80 transition-colors">
                  <td className="px-6 py-5 whitespace-nowrap">
                    <div className="flex items-center gap-2.5">
                      <span className="font-bold text-[#202020] text-sm">{release.version}</span>
                      {index === 0 && (
                        <span className="inline-flex items-center gap-1 rounded-full bg-green-100 px-2.5 py-0.5 text-[11px] font-bold text-green-700">
                          <CheckCircle2 className="h-3 w-3" /> Latest
                        </span>
                      )}
                    </div>
                  </td>

                  <td className="px-6 py-5 whitespace-nowrap text-sm font-medium text-slate-600">
                    {release.date}
                  </td>

                  <td className="px-6 py-5 text-sm font-medium text-slate-600 leading-relaxed max-w-lg">
                    {release.notes}
                  </td>

                  <td className="px-6 py-5 whitespace-nowrap text-right">
                    <a
                      href={latestDownload.downloadUrl}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="inline-flex items-center gap-1.5 rounded-lg bg-slate-100 hover:bg-[#BE5209] hover:text-white px-3.5 py-1.5 text-xs font-bold text-slate-700 transition-all"
                    >
                      <Download className="h-3.5 w-3.5" /> Get Build
                    </a>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </section>
  );
}