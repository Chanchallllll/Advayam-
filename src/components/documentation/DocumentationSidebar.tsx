"use client";

import { ArrowLeft, PanelLeftClose, PanelLeftOpen } from "lucide-react";
import { useState } from "react";
import { documentationGroups } from "@/data/documentation";

import DocumentationTree from "./DocumentationTree";

export default function DocumentationSidebar({
  sidebarOpen,
  setSidebarOpen,
}: {
  sidebarOpen: boolean;
  setSidebarOpen: (value: boolean) => void;
}) {
  const initialOpen = documentationGroups.reduce<Record<string, boolean>>(
    (acc, group) => {
      acc[group.id] = Boolean(group.defaultOpen);
      return acc;
    },
    {},
  );
  const [activeHref, setActiveHref] = useState("#microcontrollers");
  const [openGroups, setOpenGroups] = useState(initialOpen);

  return (
    <aside
      className={`fixed bottom-2 left-2 top-2 z-[9998] flex flex-col overflow-hidden rounded-2xl border border-white/10 bg-[#111111] px-4 py-4 text-white shadow-2xl shadow-black/40 transition-all duration-300 sm:bottom-3 sm:left-3 sm:top-3 sm:rounded-3xl sm:px-5 sm:py-5 md:bottom-4 md:left-4 md:top-4 md:rounded-[2rem] md:px-6 md:py-6 ${
        sidebarOpen ? "w-[310px]" : "w-[72px]"
      }`}
    >
      {/* Toggle button top-right */}
      <button
        onClick={() => setSidebarOpen(!sidebarOpen)}
        className="absolute right-5 top-5 flex h-9 w-9 items-center justify-center rounded-full border-2 border-[#202020] hover:bg-[#FFFFFF]"
        aria-label="Toggle sidebar"
      >
        {sidebarOpen ? (
          <PanelLeftClose className="h-4 w-4" />
        ) : (
          <PanelLeftOpen className="h-4 w-4" />
        )}
      </button>

      {sidebarOpen ? (
        <div className="doc-scrollbar mt-14 flex min-h-0 flex-1 flex-col gap-8 overflow-y-auto pr-2">
          {documentationGroups.map((group) => (
            <DocumentationTree
              key={group.id}
              group={group}
              isOpen={openGroups[group.id]}
              activeHref={activeHref}
              setActiveHref={setActiveHref}
              onToggle={() =>
                setOpenGroups((prev) => ({
                  ...prev,
                  [group.id]: !prev[group.id],
                }))
              }
            />
          ))}
        </div>
      ) : (
        <div className="mt-14 flex min-h-0 flex-1 flex-col items-center overflow-y-auto"></div>
      )}

      <button
        onClick={() => window.history.back()}
        className="mt-5 flex h-10 shrink-0 items-center justify-center gap-3 rounded-full border-2 border-[#202020] font-jakarta text-base font-semibold transition hover:bg-[#FFFFFF]"
      >
        <img src="/icons/back-icon.svg" className="h-4 w-4" />
        {sidebarOpen && "Back"}
      </button>
    </aside>
  );
}
