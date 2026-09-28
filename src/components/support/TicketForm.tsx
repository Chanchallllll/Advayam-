"use client";

import Link from "next/link";
import {
  Bold,
  CheckCircle2,
  Italic,
  List,
  Paperclip,
  Underline,
  Loader,
} from "lucide-react";
import { useRef, useState } from "react";
import UploadBox from "./UploadBox";
import { useAuth } from "@/context/AuthContext";
import { apiFetch } from "@/lib/api";
import { useToast } from "@/context/ToastContext";

const categories = [
  "Hardware Issue",
  "Software Issue",
  "VS Code Extension",
  "Account / Login",
  "Orders / Shop",
  "Documentation",
  "Other",
];

export default function TicketForm() {
  const { user, isLoading: authLoading } = useAuth();
  const { showToast } = useToast();
  const [isSubmitting, setIsSubmitting] = useState(false);
  const editorRef = useRef<HTMLDivElement | null>(null);
  const hiddenFileRef = useRef<HTMLInputElement | null>(null);

  const [submitted, setSubmitted] = useState(false);
  const [files, setFiles] = useState<File[]>([]);
  const [error, setError] = useState("");

  const [activeTools, setActiveTools] = useState({
    bold: false,
    italic: false,
    underline: false,
    bullet: false,
  });

  const [form, setForm] = useState({
    category: "",
    title: "",
    description: "",
  });

  function updateField(field: keyof typeof form, value: string) {
    setForm((prev) => ({ ...prev, [field]: value }));
  }

  function syncDescription() {
    updateField("description", editorRef.current?.innerHTML || "");
  }

  function runCommand(command: string) {
    editorRef.current?.focus();
    document.execCommand(command, false);
    syncDescription();
  }

  function toggleTool(tool: keyof typeof activeTools, command: string) {
    runCommand(command);
    setActiveTools((prev) => ({
      ...prev,
      [tool]: !prev[tool],
    }));
  }

  function handleInlineFileUpload(selectedFiles: FileList | null) {
    if (!selectedFiles) return;
    setFiles([...files, ...Array.from(selectedFiles)]);
  }

  function resetForm() {
    setSubmitted(false);
    setFiles([]);
    setError("");
    setForm({
      category: "",
      title: "",
      description: "",
    });
    setActiveTools({
      bold: false,
      italic: false,
      underline: false,
      bullet: false,
    });

    if (editorRef.current) {
      editorRef.current.innerHTML = "";
    }
  }

  const fileToBase64 = (file: File): Promise<string> => {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.readAsDataURL(file);
      reader.onload = () => resolve(reader.result as string);
      reader.onerror = (error) => reject(error);
    });
  };

  async function submitTicket() {
    setError("");

    const plainDescription = editorRef.current?.innerText.trim() || "";

    if (!form.category || !form.title || !plainDescription) {
      setError("Please fill all required fields.");
      showToast("Please fill all required fields.", "error");
      return;
    }

    setIsSubmitting(true);
    try {
      const uploadedAttachments = [];
      for (const file of files) {
        const base64Data = await fileToBase64(file);
        const uploadResult = await apiFetch<any>("/tickets/upload", {
          method: "POST",
          body: JSON.stringify({
            name: file.name,
            size: file.size,
            type: file.type,
            data: base64Data,
          }),
        });

        uploadedAttachments.push({
          name: uploadResult.name,
          size: uploadResult.size,
          type: uploadResult.type,
          data: uploadResult.data, // Secure URL from Cloudinary
        });
      }

      const ticketPayload = {
        category: form.category,
        title: form.title,
        description: editorRef.current?.innerHTML || "",
        attachments: uploadedAttachments,
      };

      await apiFetch("/tickets", {
        method: "POST",
        body: JSON.stringify(ticketPayload),
      });

      showToast("Support ticket raised successfully!", "success");
      setSubmitted(true);
    } catch (err: any) {
      console.error(err);
      const msg = err.message || "Failed to submit support request. Please try again.";
      setError(msg);
      showToast(msg, "error");
    } finally {
      setIsSubmitting(false);
    }
  }

  if (authLoading) {
    return (
      <div className="mx-auto w-full max-w-[620px] rounded-3xl border border-[#202020]/10 bg-white p-12 text-center shadow-sm flex flex-col items-center justify-center">
        <Loader className="h-8 w-8 animate-spin text-[#BE5209]" />
        <span className="mt-4 text-sm font-bold text-slate-400">Verifying session...</span>
      </div>
    );
  }

  if (!user) {
    return (
      <div className="mx-auto w-full max-w-[620px] rounded-3xl border border-[#202020]/10 bg-white p-8 text-center shadow-sm">
        <h1 className="font-jakarta text-3xl font-black text-[#202020]">Sign In Required</h1>
        <p className="mt-3 font-jakarta text-sm font-medium leading-6 text-[#202020]/60">
          You must be logged in to raise a support ticket so we can track and respond to your request.
        </p>
        <div className="mt-7 flex gap-4 max-sm:flex-col">
          <Link
            href="/auth/login"
            className="flex h-12 flex-1 items-center justify-center rounded-xl bg-[#BE5209] font-jakarta text-sm font-black text-white hover:bg-black transition"
          >
            Go to Login
          </Link>
          <Link
            href="/support"
            className="flex h-12 flex-1 items-center justify-center rounded-xl border border-[#202020]/15 bg-white font-jakarta text-sm font-black text-[#202020] hover:bg-slate-50 transition"
          >
            Cancel
          </Link>
        </div>
      </div>
    );
  }

  if (submitted) {
    return (
      <div className="mx-auto w-full max-w-[620px] rounded-3xl border border-[#202020]/10 bg-white p-8 text-center shadow-sm">
        <CheckCircle2 className="mx-auto h-14 w-14 text-[#d95b00]" />

        <h1 className="mt-5 font-jakarta text-3xl font-black text-[#202020]">
          Ticket Submitted
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium leading-6 text-[#202020]/60">
          Your support request has been recorded. The Advayam support team will
          review it and contact you soon.
        </p>

        <div className="mt-7 flex gap-4 max-sm:flex-col">
          <button
            type="button"
            onClick={resetForm}
            className="flex h-12 flex-1 items-center justify-center rounded-xl border border-[#202020]/15 bg-white font-jakarta text-sm font-black text-[#202020]"
          >
            Raise Another Ticket
          </button>

          <Link
            href="/support"
            className="flex h-12 flex-1 items-center justify-center rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white"
          >
            Go to Support
          </Link>
        </div>
      </div>
    );
  }

  return (
    <div className="mx-auto w-full max-w-[1320px]">
      <div>
        <h1 className="font-jakarta text-[34px] font-black leading-none text-[#202020]">
          Raise a Ticket
        </h1>

        <p className="mt-2 font-jakarta text-sm font-medium text-[#202020]/60">
          Facing an issue? Let us know and we’ll help you fix it.
        </p>
      </div>

      <div className="mt-6 grid h-[430px] grid-cols-[1.05fr_0.95fr] gap-10 max-lg:h-auto max-lg:grid-cols-1">
        <div className="flex h-full flex-col">
          <SelectField
            label="Category"
            value={form.category}
            options={categories}
            onChange={(value) => updateField("category", value)}
          />

          <div className="mt-3">
            <Field
              label="Subject"
              placeholder="Brief summary of the issue"
              value={form.title}
              onChange={(value) => updateField("title", value)}
            />
          </div>

          <label className="mt-3 flex min-h-0 flex-1 flex-col">
            <span className="font-jakarta text-sm font-black text-[#202020]">
              Description <span className="text-[#d95b00]">*</span>
            </span>

            <div className="mt-2 flex min-h-0 flex-1 flex-col overflow-hidden rounded-xl border border-[#202020]/15 bg-white">
              <div className="flex h-10 shrink-0 items-center gap-6 border-b border-[#202020]/10 px-5">
                <ToolbarButton
                  active={activeTools.bold}
                  onClick={() => toggleTool("bold", "bold")}
                  label="Bold"
                >
                  <Bold className="h-4 w-4" />
                </ToolbarButton>

                <ToolbarButton
                  active={activeTools.italic}
                  onClick={() => toggleTool("italic", "italic")}
                  label="Italicise"
                >
                  <Italic className="h-4 w-4" />
                </ToolbarButton>

                <ToolbarButton
                  active={activeTools.underline}
                  onClick={() => toggleTool("underline", "underline")}
                  label="Underline"
                >
                  <Underline className="h-4 w-4" />
                </ToolbarButton>

                <ToolbarButton
                  active={activeTools.bullet}
                  onClick={() =>
                    toggleTool("bullet", "insertUnorderedList")
                  }
                  label="Bullet list"
                >
                  <List className="h-4 w-4" />
                </ToolbarButton>

                <ToolbarButton
                  active={false}
                  onClick={() => hiddenFileRef.current?.click()}
                  label="Add file"
                >
                  <Paperclip className="h-4 w-4" />
                </ToolbarButton>

                <input
                  ref={hiddenFileRef}
                  type="file"
                  multiple
                  accept=".png,.jpg,.jpeg,.pdf"
                  onChange={(e) => handleInlineFileUpload(e.target.files)}
                  className="hidden"
                />
              </div>

              <div
                ref={editorRef}
                contentEditable
                suppressContentEditableWarning
                onInput={syncDescription}
                data-placeholder="Provide as much detail as possible, including steps to reproduce the issue."
                className="support-editor min-h-0 flex-1 overflow-y-auto px-5 py-4 font-jakarta text-sm leading-6 outline-none"
              />
            </div>
          </label>
        </div>

        <div className="h-full max-lg:h-[390px]">
          <UploadBox files={files} setFiles={setFiles} />
        </div>
      </div>

      {error && (
        <p className="mt-3 rounded-xl border border-red-200 bg-red-50 px-4 py-3 font-jakarta text-sm font-bold text-red-600">
          {error}
        </p>
      )}

      <div className="mt-4 flex justify-end">
        <button
          type="button"
          onClick={submitTicket}
          disabled={isSubmitting}
          className="h-11 rounded-xl bg-[#d95b00] px-9 font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50 transition inline-flex items-center gap-2 cursor-pointer shadow-sm"
        >
          {isSubmitting ? (
            <>
              <Loader className="h-4 w-4 animate-spin text-white" />
              <span>Submitting...</span>
            </>
          ) : (
            <span>Submit Ticket</span>
          )}
        </button>
      </div>
    </div>
  );
}

function ToolbarButton({
  children,
  active,
  onClick,
  label,
}: {
  children: React.ReactNode;
  active: boolean;
  onClick: () => void;
  label: string;
}) {
  return (
    <button
      type="button"
      aria-label={label}
      onMouseDown={(e) => e.preventDefault()}
      onClick={onClick}
      className={`transition ${
        active ? "text-[#d95b00]" : "text-[#202020]/70 hover:text-[#d95b00]"
      }`}
    >
      {children}
    </button>
  );
}

function Field({
  label,
  placeholder,
  value,
  onChange,
}: {
  label: string;
  placeholder: string;
  value: string;
  onChange: (value: string) => void;
}) {
  return (
    <label className="block">
      <span className="font-jakarta text-sm font-black text-[#202020]">
        {label} <span className="text-[#d95b00]">*</span>
      </span>

      <input
        type="text"
        placeholder={placeholder}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        className="mt-2 h-10 w-full rounded-xl border border-[#202020]/15 bg-white px-4 font-jakarta text-sm outline-none focus:border-[#d95b00]"
      />
    </label>
  );
}

function SelectField({
  label,
  value,
  options,
  onChange,
}: {
  label: string;
  value: string;
  options: string[];
  onChange: (value: string) => void;
}) {
  return (
    <label className="block">
      <span className="font-jakarta text-sm font-black text-[#202020]">
        {label} <span className="text-[#d95b00]">*</span>
      </span>

      <select
        value={value}
        onChange={(e) => onChange(e.target.value)}
        className="mt-2 h-10 w-full rounded-xl border border-[#202020]/15 bg-white px-4 font-jakarta text-sm outline-none focus:border-[#d95b00]"
      >
        <option value="">Select a category</option>

        {options.map((option) => (
          <option key={option} value={option}>
            {option}
          </option>
        ))}
      </select>
    </label>
  );
}