"use client";

import { FileText, X } from "lucide-react";

export default function AttachmentCard({
  file,
  onRemove,
}: {
  file: File;
  onRemove: () => void;
}) {
  return (
    <div className="flex items-center justify-between rounded-xl border border-[#202020]/15 bg-white px-4 py-3">
      <div className="flex min-w-0 items-center gap-3">
        <FileText className="h-5 w-5 shrink-0 text-[#d95b00]" />
        <div className="min-w-0">
          <p className="truncate font-jakarta text-sm font-black text-[#202020]">
            {file.name}
          </p>
          <p className="font-jakarta text-xs font-medium text-[#202020]/50">
            {(file.size / 1024).toFixed(1)} KB
          </p>
        </div>
      </div>

      <button
        type="button"
        onClick={onRemove}
        className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full hover:bg-[#f2f2f2]"
      >
        <X className="h-4 w-4" />
      </button>
    </div>
  );
}