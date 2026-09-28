"use client";

import { UploadCloud } from "lucide-react";
import AttachmentCard from "./AttachmentCard";

export default function UploadBox({
  files,
  setFiles,
}: {
  files: File[];
  setFiles: (files: File[]) => void;
}) {
  function handleFiles(selectedFiles: FileList | null) {
    if (!selectedFiles) return;
    setFiles([...files, ...Array.from(selectedFiles)]);
  }

  return (
    <div className="flex h-full flex-col">
      <label className="mb-2 block font-jakarta text-sm font-black text-[#202020]">
        Attach Screenshot / Image
      </label>

      <label className="flex min-h-0 flex-1 cursor-pointer flex-col items-center justify-center rounded-2xl border-2 border-dashed border-[#d95b00]/40 bg-white px-6 text-center transition hover:border-[#d95b00]">
        <UploadCloud className="h-11 w-11 text-[#d95b00]" />

        <p className="mt-3 font-jakarta text-sm font-black text-[#202020]">
          Click to upload
        </p>

        <p className="mt-1 font-jakarta text-xs font-medium text-[#202020]/60">
          or drag and drop
        </p>

        <p className="mt-1 font-jakarta text-xs font-medium text-[#202020]/50">
          PNG, JPG or PDF max. 10MB
        </p>

        <input
          type="file"
          multiple
          accept=".png,.jpg,.jpeg,.pdf"
          onChange={(e) => handleFiles(e.target.files)}
          className="hidden"
        />
      </label>

      {files.length > 0 && (
        <div className="mt-3 grid max-h-[74px] gap-2 overflow-y-auto pr-1">
          {files.map((file, index) => (
            <AttachmentCard
              key={`${file.name}-${index}`}
              file={file}
              onRemove={() =>
                setFiles(files.filter((_, fileIndex) => fileIndex !== index))
              }
            />
          ))}
        </div>
      )}
    </div>
  );
}