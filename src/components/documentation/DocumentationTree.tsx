"use client";

import {
  ChevronDown,
  ChevronRight,
} from "lucide-react";

import DocumentationBranch from "./DocumentationBranch";
import type { DocGroup } from "@/data/documentation";

export default function DocumentationTree({
  group,
  isOpen,
  onToggle,
  activeHref,
  setActiveHref,
}: {
  group: DocGroup;
  isOpen: boolean;
  onToggle: () => void;
  activeHref: string;
  setActiveHref: (href: string) => void;
}) {
  return (
    <div>
      {/* GROUP HEADER */}
      <button
        type="button"
        onClick={onToggle}
        className="flex w-full items-center gap-3 rounded-xl px-2 py-2 font-jakarta text-sm font-extrabold text-white transition hover:bg-white/[0.06]"
      >
        <img
          src="/icons/cpu-icon.svg"
          className="h-4 w-4 brightness-0 invert"
          alt=""
        />

        <span>{group.title}</span>

        {isOpen ? (
          <ChevronDown className="ml-auto h-4 w-4 text-white/60" />
        ) : (
          <ChevronRight className="ml-auto h-4 w-4 text-white/60" />
        )}
      </button>

      {/* CHILDREN */}
      {isOpen && group.children && (
        <div className="relative ml-2 mt-3 h-[128px]">
          <DocumentationBranch itemCount={group.children.length} />

          <div className="absolute left-10 top-[8px] flex h-[104px] flex-col justify-between">
            {group.children.map((child) => (
              <a
                key={child.href}
                href={child.href}
                onClick={() =>
                  setActiveHref(child.href)
                }
                className={`w-fit rounded-full px-4 py-1.5 font-jakarta text-sm font-medium transition-all ${
                  activeHref === child.href
                    ? "bg-[#EA580C] text-white shadow-md shadow-orange-500/20"
                    : "text-white/65 hover:bg-white/10 hover:text-white"
                }`}
              >
                {child.title}
              </a>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}