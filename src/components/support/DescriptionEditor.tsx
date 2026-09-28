"use client";

import {
  Bold,
  Italic,
  List,
  Link2,
  Underline,
  AlignLeft,
} from "lucide-react";

export default function DescriptionEditor({
  value,
  onChange,
}: {
  value: string;
  onChange: (v: string) => void;
}) {
  return (
    <div>
      <label className="font-jakarta text-sm font-black">
        Description *
      </label>

      <div className="mt-3 overflow-hidden rounded-2xl border border-[#202020]/15 bg-white">

        {/* Toolbar */}

        <div className="flex h-12 items-center gap-2 border-b border-[#202020]/10 px-4">

          <ToolbarButton icon={<Bold size={16} />} />

          <ToolbarButton icon={<Italic size={16} />} />

          <ToolbarButton icon={<Underline size={16} />} />

          <div className="mx-2 h-5 w-px bg-[#202020]/10" />

          <ToolbarButton icon={<List size={16} />} />

          <ToolbarButton icon={<AlignLeft size={16} />} />

          <div className="mx-2 h-5 w-px bg-[#202020]/10" />

          <ToolbarButton icon={<Link2 size={16} />} />

        </div>

        {/* Text Area */}

        <textarea
          value={value}
          onChange={(e) => onChange(e.target.value)}
          placeholder="Provide as much detail as possible, including steps to reproduce the issue..."
          className="h-[210px] w-full resize-none bg-transparent p-5 font-jakarta text-sm outline-none"
        />
      </div>
    </div>
  );
}

function ToolbarButton({
  icon,
}: {
  icon: React.ReactNode;
}) {
  return (
    <button
      type="button"
      className="flex h-8 w-8 items-center justify-center rounded-md transition hover:bg-[#ECECEC]"
    >
      {icon}
    </button>
  );
}