"use client";

import { useState, useMemo, useEffect } from "react";
import CommunityHeader from "@/components/community/CommunityHeader";
import Footer from "@/components/Footer";
import { useAuth } from "@/context/AuthContext";
import {
  ALL_DEMO_PROGRAMS,
  DemoProgramItem,
  ProgramLanguage,
} from "@/data/allDemoPrograms";
import {
  Search,
  Code2,
  Copy,
  Check,
  Download,
  X,
  ExternalLink,
  Terminal,
  FileCode,
  ListFilter,
  ChevronRight,
  Sparkles,
} from "lucide-react";

const LANGUAGES: { label: string; value: ProgramLanguage | "All"; count: number }[] = [
  { label: "All Languages", value: "All", count: ALL_DEMO_PROGRAMS.length },
  {
    label: "Arduino",
    value: "Arduino",
    count: ALL_DEMO_PROGRAMS.filter((p) => p.language === "Arduino").length,
  },
  {
    label: "ESP-IDF",
    value: "ESP-IDF",
    count: ALL_DEMO_PROGRAMS.filter((p) => p.language === "ESP-IDF").length,
  },
  {
    label: "FreeRTOS",
    value: "FreeRTOS",
    count: ALL_DEMO_PROGRAMS.filter((p) => p.language === "FreeRTOS").length,
  },
];

const GUEST_PREVIEW_LINES = 12;

export default function DemoProgramsPage() {
  const { user, isLoading: authLoading } = useAuth();
  const isAuthenticated = !authLoading && Boolean(user);
  const [selectedLanguage, setSelectedLanguage] = useState<ProgramLanguage | "All">("All");
  const [searchQuery, setSearchQuery] = useState("");
  const [selectedCategory, setSelectedCategory] = useState<string>("All");
  const [activeModalProgram, setActiveModalProgram] = useState<DemoProgramItem | null>(null);
  const [modalCode, setModalCode] = useState<string>("");
  const [loadingCode, setLoadingCode] = useState<boolean>(false);
  const [copiedCode, setCopiedCode] = useState<boolean>(false);
  const [activeHighlightId, setActiveHighlightId] = useState<string | null>(null);
  const [mobileIndexOpen, setMobileIndexOpen] = useState(false);
  const [showDownloadPrompt, setShowDownloadPrompt] = useState(false);

  // Filter programs based on selected language, category, and search query
  const filteredPrograms = useMemo(() => {
    return ALL_DEMO_PROGRAMS.filter((program) => {
      // 1. Language filter
      if (selectedLanguage !== "All" && program.language !== selectedLanguage) {
        return false;
      }
      // 2. Category filter
      if (selectedCategory !== "All" && program.category !== selectedCategory) {
        return false;
      }
      // 3. Search query
      if (searchQuery.trim()) {
        const query = searchQuery.toLowerCase();
        const matchesTitle = program.title.toLowerCase().includes(query);
        const matchesDesc = program.description.toLowerCase().includes(query);
        const matchesCategory = program.category.toLowerCase().includes(query);
        const matchesFilename = program.filename.toLowerCase().includes(query);
        if (!matchesTitle && !matchesDesc && !matchesCategory && !matchesFilename) {
          return false;
        }
      }
      return true;
    });
  }, [selectedLanguage, selectedCategory, searchQuery]);

  // Categories available for currently selected language
  const availableCategories = useMemo(() => {
    const list = ALL_DEMO_PROGRAMS.filter((p) =>
      selectedLanguage === "All" ? true : p.language === selectedLanguage
    ).map((p) => p.category);
    return ["All", ...Array.from(new Set(list))];
  }, [selectedLanguage]);

  // Handle opening code modal & fetching file
  const handleOpenCodeModal = (program: DemoProgramItem) => {
    setActiveModalProgram(program);
    setLoadingCode(true);
    setModalCode("");
    setCopiedCode(false);

    // If existing href is blink-led or special, fetch or load directly
    if (program.id === "cpp-0") {
      fetch("/Programs/Program1.cpp")
        .then((res) => (res.ok ? res.text() : "// Blink LED sample"))
        .then((text) => {
          setModalCode(isAuthenticated ? text : text.split("\n").slice(0, GUEST_PREVIEW_LINES).join("\n"));
          setLoadingCode(false);
        })
        .catch(() => {
          setModalCode("// Blink LED program for Advayam Core Board");
          setLoadingCode(false);
        });
      return;
    }

    fetch(program.codePath)
      .then((res) => {
        if (!res.ok) throw new Error("Failed to load program file");
        return res.text();
      })
      .then((text) => {
        setModalCode(isAuthenticated ? text : text.split("\n").slice(0, GUEST_PREVIEW_LINES).join("\n"));
        setLoadingCode(false);
      })
      .catch((err) => {
        setModalCode(`// Could not load file: ${program.filename}\n// Path: ${program.codePath}\n\n// Error: ${err.message}`);
        setLoadingCode(false);
      });
  };

  // Close modal on Escape key
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        setActiveModalProgram(null);
      }
    };
    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, []);

  // Jump to program card from the right-side index
  const jumpToProgram = (programId: string) => {
    setActiveHighlightId(programId);
    setMobileIndexOpen(false);

    const el = document.getElementById(`program-${programId}`);
    if (el) {
      el.scrollIntoView({ behavior: "smooth", block: "center" });
    }

    setTimeout(() => {
      setActiveHighlightId(null);
    }, 2500);
  };

  const copyToClipboard = async () => {
    if (!modalCode) return;
    await navigator.clipboard.writeText(modalCode);
    setCopiedCode(true);
    setTimeout(() => setCopiedCode(false), 2000);
  };

  const downloadFile = (program: DemoProgramItem) => {
    if (!isAuthenticated) {
      setShowDownloadPrompt(true);
      return;
    }

    const content = modalCode || `// ${program.title} (${program.language})\n// Downloaded from Advayam Demo Programs`;
    const blob = new Blob([content], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = program.filename;
    link.click();
    URL.revokeObjectURL(url);
  };

  const getLanguageColor = (lang: ProgramLanguage) => {
    switch (lang) {
      case "Arduino":
        return "bg-emerald-500/15 text-emerald-400 border-emerald-500/30";
      case "ESP-IDF":
        return "bg-orange-500/15 text-orange-400 border-orange-500/30";
      case "FreeRTOS":
        return "bg-cyan-500/15 text-cyan-400 border-cyan-500/30";
      default:
        return "bg-white/10 text-white/80 border-white/20";
    }
  };

  return (
    <main className="relative flex min-h-screen flex-col overflow-x-hidden bg-black text-white font-jakarta">
      <CommunityHeader />

      {/* Background ambient accents */}
      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none fixed right-0 top-[92px] z-0 h-[380px] w-[470px] object-contain object-right opacity-40 blur-sm"
      />

      <div className="relative z-10 mx-auto w-full max-w-[1560px] px-4 sm:px-6 lg:px-8 pb-24 pt-[130px]">
        {/* Header Hero */}
        <div className="flex flex-col items-start max-w-4xl">
          <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/10 px-3.5 py-1 text-xs font-bold uppercase tracking-wider text-orange-400">
            <Sparkles className="h-3.5 w-3.5" />
            Official & Community Code Repository
          </div>

          <h1 className="mt-4 text-4xl sm:text-5xl lg:text-6xl font-black tracking-tight text-white">
            Demo Programs
          </h1>

          <p className="mt-4 text-base sm:text-lg font-medium text-slate-400 leading-relaxed max-w-3xl">
            Browse through <span className="text-white font-bold">{ALL_DEMO_PROGRAMS.length} verified programs</span> across{" "}
            <span className="text-emerald-400 font-bold">Arduino</span>,{" "}
            <span className="text-orange-400 font-bold">ESP-IDF</span>,{" "}
            <span className="text-cyan-400 font-bold">FreeRTOS</span>. Select your framework to filter the codes and use the quick index on the right.
          </p>
        </div>

        {/* Language Selection Filter Tabs */}
        <div className="mt-8 flex flex-wrap items-center gap-2.5 border-b border-white/10 pb-5">
          <span className="text-xs font-bold uppercase tracking-wider text-slate-400 mr-2 flex items-center gap-1.5">
            <Terminal className="h-4 w-4 text-orange-400" />
            Language:
          </span>

          {LANGUAGES.map((lang) => {
            const isSelected = selectedLanguage === lang.value;
            return (
              <button
                key={lang.value}
                onClick={() => {
                  setSelectedLanguage(lang.value);
                  setSelectedCategory("All");
                }}
                className={`flex items-center gap-2 rounded-full px-4 py-2 text-xs sm:text-sm font-bold transition-all cursor-pointer ${
                  isSelected
                    ? "bg-gradient-to-r from-[#EA580C] to-[#F97316] text-white shadow-lg shadow-orange-500/25 scale-[1.02]"
                    : "border border-white/15 bg-white/[0.04] text-slate-300 hover:bg-white/10 hover:text-white"
                }`}
              >
                <span>{lang.label}</span>
                <span
                  className={`rounded-full px-2 py-0.5 text-[11px] font-extrabold ${
                    isSelected ? "bg-black/25 text-white" : "bg-white/10 text-slate-400"
                  }`}
                >
                  {lang.count}
                </span>
              </button>
            );
          })}
        </div>

        {/* Search & Category Filter Toolbar */}
        <div className="mt-6 flex flex-col md:flex-row items-stretch md:items-center justify-between gap-4">
          {/* Search Bar */}
          <div className="relative flex-1 max-w-md">
            <input
              type="text"
              placeholder={`Search ${selectedLanguage === "All" ? "all" : selectedLanguage} programs (e.g. Matrix, IMU, Game)...`}
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="w-full h-11 rounded-2xl border border-white/15 bg-white/[0.05] pl-10 pr-4 text-sm text-white placeholder:text-slate-500 outline-none focus:border-orange-500 focus:bg-white/[0.08] transition-colors"
            />
            <Search className="absolute left-3.5 top-3 h-4 w-4 text-slate-500" />
            {searchQuery && (
              <button
                onClick={() => setSearchQuery("")}
                className="absolute right-3 top-3 text-slate-400 hover:text-white"
              >
                <X className="h-4 w-4" />
              </button>
            )}
          </div>

          {/* Category Pills dropdown or scroll */}
          {availableCategories.length > 2 && (
            <div className="category-scrollbar flex items-center gap-2 overflow-x-auto pb-1 max-w-full">
              <span className="text-xs font-semibold text-slate-400 shrink-0 flex items-center gap-1">
                <ListFilter className="h-3.5 w-3.5" />
                Category:
              </span>
              <div className="flex items-center gap-1.5">
                {availableCategories.map((cat) => (
                  <button
                    key={cat}
                    onClick={() => setSelectedCategory(cat)}
                    className={`whitespace-nowrap rounded-lg px-3 py-1.5 text-xs font-bold transition-all cursor-pointer ${
                      selectedCategory === cat
                        ? "bg-white text-black"
                        : "bg-white/[0.06] text-slate-400 hover:bg-white/10 hover:text-white"
                    }`}
                  >
                    {cat}
                  </button>
                ))}
              </div>
            </div>
          )}
        </div>

        {/* Results Info Counter */}
        <div className="mt-4 flex items-center justify-between text-xs font-medium text-slate-400">
          <p>
            Showing <span className="font-bold text-white">{filteredPrograms.length}</span> programs
            {selectedLanguage !== "All" && (
              <span> in <span className="text-orange-400 font-bold">{selectedLanguage}</span></span>
            )}
            {selectedCategory !== "All" && (
              <span> • Category: <span className="text-white font-bold">{selectedCategory}</span></span>
            )}
          </p>

          {/* Mobile Index Trigger Button */}
          <button
            onClick={() => setMobileIndexOpen(true)}
            className="flex lg:hidden items-center gap-1.5 rounded-lg border border-white/20 bg-white/10 px-3 py-1.5 text-xs font-bold text-white hover:bg-white/15"
          >
            <ListFilter className="h-3.5 w-3.5 text-orange-400" />
            Quick Index ({filteredPrograms.length})
          </button>
        </div>

        {/* Main 2-Column Section: Programs Grid (Left) + Filtered Sticky Index (Right) */}
        <div className="mt-6 flex flex-col lg:flex-row items-start gap-8">
          {/* LEFT: Programs Grid */}
          <div className="w-full lg:flex-1">
            {filteredPrograms.length === 0 ? (
              <div className="flex flex-col items-center justify-center rounded-3xl border border-white/10 bg-white/[0.02] p-16 text-center">
                <Code2 className="h-12 w-12 text-slate-600 mb-4" />
                <h3 className="text-lg font-bold text-white">No programs found</h3>
                <p className="mt-1 text-sm text-slate-400 max-w-sm">
                  No programs match your current filter criteria. Try clearing the search query or selecting &quot;All Languages&quot;.
                </p>
                <button
                  onClick={() => {
                    setSelectedLanguage("All");
                    setSelectedCategory("All");
                    setSearchQuery("");
                  }}
                  className="mt-5 rounded-xl bg-orange-600 px-4 py-2 text-xs font-bold text-white hover:bg-orange-500 transition"
                >
                  Reset Filters
                </button>
              </div>
            ) : (
              <div className="grid grid-cols-1 md:grid-cols-2 gap-5">
                {filteredPrograms.map((program) => {
                  const isHighlighted = activeHighlightId === program.id;
                  return (
                    <div
                      key={program.id}
                      id={`program-${program.id}`}
                      className={`group relative flex flex-col justify-between rounded-3xl border p-6 transition-all duration-300 ${
                        isHighlighted
                          ? "border-orange-500 bg-orange-500/10 shadow-2xl shadow-orange-500/20 scale-[1.02]"
                          : "border-white/10 bg-white/[0.04] hover:border-white/25 hover:bg-white/[0.07] hover:-translate-y-1 shadow-sm"
                      }`}
                    >
                      <div>
                        {/* Top Badges */}
                        <div className="flex flex-wrap items-center justify-between gap-2 mb-3.5">
                          <div className="flex items-center gap-2">
                            <span
                              className={`rounded-full border px-2.5 py-0.5 text-[11px] font-black uppercase tracking-wider ${getLanguageColor(
                                program.language
                              )}`}
                            >
                              {program.language}
                            </span>
                            <span className="rounded-full bg-white/[0.08] px-2.5 py-0.5 text-[11px] font-semibold text-slate-300">
                              {program.category}
                            </span>
                          </div>

                          <span className="text-[11px] font-bold text-slate-400">
                            {program.level}
                          </span>
                        </div>

                        {/* Title */}
                        <h3 className="text-xl font-black text-white group-hover:text-orange-400 transition-colors leading-snug">
                          {program.title}
                        </h3>

                        {/* Description */}
                        <p className="mt-2 text-xs sm:text-sm font-medium leading-relaxed text-slate-400 line-clamp-3">
                          {program.description}
                        </p>
                      </div>

                      {/* Footer Actions */}
                      <div className="mt-5 pt-4 border-t border-white/10 flex items-center justify-between gap-2">
                        <div className="flex items-center gap-2">
                          <button
                            onClick={() => handleOpenCodeModal(program)}
                            className="flex items-center gap-1.5 rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] px-3.5 py-2 text-xs font-bold text-white shadow-md shadow-orange-500/20 hover:scale-[1.02] active:scale-95 transition-all cursor-pointer"
                          >
                            <Code2 className="h-3.5 w-3.5" />
                            View Code
                          </button>

                          {isAuthenticated && (
                            <button
                              onClick={() => downloadFile(program)}
                              title="Download Source Code"
                              className="flex h-8 w-8 items-center justify-center rounded-xl border border-white/15 bg-white/[0.04] text-slate-300 hover:bg-white/15 hover:text-white transition-all cursor-pointer"
                            >
                              <Download className="h-3.5 w-3.5" />
                            </button>
                          )}
                        </div>

                        {program.existingHref && (
                          <a
                            href={program.existingHref}
                            className="inline-flex items-center gap-1 text-xs font-bold text-slate-400 hover:text-white transition-colors"
                          >
                            <span>Dedicated Page</span>
                            <ExternalLink className="h-3 w-3" />
                          </a>
                        )}
                      </div>
                    </div>
                  );
                })}
              </div>
            )}
          </div>

          {/* RIGHT: Filtered Sticky Index Panel */}
            <aside className="hidden lg:flex w-72 xl:w-80 shrink-0 sticky top-[100px] max-h-[calc(100vh-120px)] flex-col rounded-3xl border border-white/15 bg-[#0e0e0e]/95 p-5 shadow-2xl backdrop-blur-xl">            <div className="flex items-center justify-between border-b border-white/10 pb-3 mb-3">
              <div className="flex items-center gap-2">
                <ListFilter className="h-4 w-4 text-orange-400" />
                <h3 className="text-sm font-black uppercase tracking-wider text-white">
                  Program Index
                </h3>
              </div>
              <span className="rounded-full bg-white/10 px-2 py-0.5 text-[11px] font-bold text-slate-300">
                {filteredPrograms.length}
              </span>
            </div>

            <p className="text-[11px] text-slate-400 mb-2">
              Showing programs for <span className="font-bold text-orange-400">{selectedLanguage}</span>. Click any item to jump directly to it:
            </p>

            {/* Scrollable Index List */}
              <div className="index-scrollbar min-h-0 flex-1 overflow-y-auto pr-1 space-y-1">              {filteredPrograms.map((prog, idx) => {
                const isSelected = activeHighlightId === prog.id;
                return (
                  <button
                    key={`index-${prog.id}`}
                    onClick={() => jumpToProgram(prog.id)}
                    className={`w-full text-left flex items-start gap-2.5 rounded-xl px-2.5 py-1.5 text-xs transition-all cursor-pointer ${
                      isSelected
                        ? "bg-orange-500 text-white font-bold pl-3"
                        : "text-slate-300 hover:bg-white/10 hover:text-white"
                    }`}
                  >
                    <span className="text-[10px] font-bold text-slate-500 mt-0.5 shrink-0">
                      {(idx + 1).toString().padStart(2, "0")}
                    </span>
                    <span className="truncate flex-1 font-medium">{prog.title}</span>
                    <ChevronRight className="h-3 w-3 opacity-40 shrink-0 mt-0.5" />
                  </button>
                );
              })}
            </div>
          </aside>
        </div>
      </div>

      {/* MOBILE QUICK INDEX MODAL / DRAWER */}
      {mobileIndexOpen && (
        <div className="fixed inset-0 z-50 flex items-end sm:items-center justify-center bg-black/80 backdrop-blur-md p-4 lg:hidden">
          <div className="w-full max-w-md max-h-[80vh] flex flex-col rounded-3xl border border-white/20 bg-[#121212] p-6 shadow-2xl">
            <div className="flex items-center justify-between border-b border-white/10 pb-4 mb-4">
              <div className="flex items-center gap-2">
                <ListFilter className="h-5 w-5 text-orange-400" />
                <h3 className="text-base font-black text-white">
                  Program Index ({filteredPrograms.length})
                </h3>
              </div>
              <button
                onClick={() => setMobileIndexOpen(false)}
                className="rounded-full p-1 text-slate-400 hover:text-white"
              >
                <X className="h-5 w-5" />
              </button>
            </div>

            <div className="flex-1 overflow-y-auto space-y-1.5 pr-1">
              {filteredPrograms.map((prog, idx) => (
                <button
                  key={`mob-index-${prog.id}`}
                  onClick={() => jumpToProgram(prog.id)}
                  className="w-full text-left flex items-center justify-between rounded-xl p-2.5 text-xs text-slate-200 hover:bg-white/10 active:bg-orange-500 active:text-white"
                >
                  <span className="flex items-center gap-2 truncate">
                    <span className="text-slate-500 font-mono text-[10px]">
                      {(idx + 1).toString().padStart(2, "0")}
                    </span>
                    <span className="truncate font-semibold">{prog.title}</span>
                  </span>
                  <span className="text-[10px] px-2 py-0.5 rounded bg-white/10 text-slate-400 shrink-0 ml-2">
                    {prog.language}
                  </span>
                </button>
              ))}
            </div>
          </div>
        </div>
      )}

      {/* CODE VIEWER MODAL */}
      {activeModalProgram && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/85 backdrop-blur-md p-4 sm:p-6 md:p-8 animate-in fade-in duration-200">
          <div className="relative w-full max-w-5xl max-h-[90vh] flex flex-col rounded-3xl border border-white/20 bg-[#0d0d0d] shadow-2xl overflow-hidden">
            {/* Modal Header */}
            <div className="flex items-center justify-between border-b border-white/15 px-6 py-4 bg-white/[0.03]">
              <div className="flex items-center gap-3">
                <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-orange-500/20 text-orange-400">
                  <FileCode className="h-5 w-5" />
                </div>
                <div>
                  <div className="flex items-center gap-2">
                    <h2 className="text-lg font-black text-white">
                      {activeModalProgram.title}
                    </h2>
                    <span
                      className={`rounded-full border px-2.5 py-0.5 text-[11px] font-black ${getLanguageColor(
                        activeModalProgram.language
                      )}`}
                    >
                      {activeModalProgram.language}
                    </span>
                  </div>
                  <p className="text-xs text-slate-400 font-mono mt-0.5">
                    {activeModalProgram.filename}
                  </p>
                </div>
              </div>

              {/* Close Button */}
              <button
                onClick={() => setActiveModalProgram(null)}
                className="flex h-9 w-9 items-center justify-center rounded-full border border-white/20 bg-white/[0.05] text-slate-400 hover:bg-white/15 hover:text-white transition cursor-pointer"
              >
                <X className="h-4 w-4" />
              </button>
            </div>

            {/* Modal Subheader & Actions */}
            <div className="flex flex-wrap items-center justify-between gap-3 px-6 py-3 border-b border-white/10 bg-[#111111]/60 text-xs">
              <p className="text-slate-400 text-xs line-clamp-1 flex-1">
                {activeModalProgram.description}
              </p>

              <div className="flex items-center gap-2">
                {isAuthenticated && (
                  <>
                    <button
                      type="button"
                      onClick={copyToClipboard}
                      disabled={loadingCode || !modalCode}
                      className="flex items-center gap-1.5 rounded-xl border border-white/15 bg-white/[0.06] px-3.5 py-1.5 text-xs font-bold text-white hover:bg-white/15 transition cursor-pointer disabled:opacity-50"
                    >
                      {copiedCode ? (
                        <Check className="h-3.5 w-3.5 text-emerald-400" />
                      ) : (
                        <Copy className="h-3.5 w-3.5 text-slate-300" />
                      )}
                      <span>{copiedCode ? "Copied!" : "Copy Code"}</span>
                    </button>

                    <button
                      type="button"
                      onClick={() => downloadFile(activeModalProgram)}
                      disabled={loadingCode}
                      className="flex items-center gap-1.5 rounded-xl bg-orange-600 px-3.5 py-1.5 text-xs font-bold text-white hover:bg-orange-500 transition cursor-pointer disabled:opacity-50"
                    >
                      <Download className="h-3.5 w-3.5" />
                      <span>Download</span>
                    </button>
                  </>
                )}
              </div>
            </div>

            {/* Code Body */}
            <div className="flex-1 overflow-auto p-6 bg-[#080808]">
              {loadingCode ? (
                <div className="flex flex-col items-center justify-center py-20 text-slate-400">
                  <div className="h-8 w-8 animate-spin rounded-full border-2 border-orange-500 border-t-transparent mb-3" />
                  <p className="text-xs font-medium">Loading source code...</p>
                </div>
              ) : (
                isAuthenticated ? (
                  <pre className="font-mono text-xs sm:text-sm leading-6 text-slate-200 selection:bg-orange-500 selection:text-white">
                    <code>{modalCode}</code>
                  </pre>
                ) : (
                  <div className="relative">
                    <pre className="font-mono text-xs sm:text-sm leading-6 text-slate-200 selection:bg-orange-500 selection:text-white">
                      <code>{modalCode.split("\n").slice(0, GUEST_PREVIEW_LINES).join("\n")}</code>
                    </pre>
                    <div className="relative mt-2 overflow-hidden rounded-2xl border border-orange-500/25 bg-orange-500/10 p-6 text-center">
                      <div className="pointer-events-none absolute inset-x-0 -top-12 h-24 bg-[#080808] blur-xl" />
                      <p className="relative text-sm font-bold text-white">Log in or register to view the full code</p>
                      <p className="relative mt-1 text-xs text-slate-300">The rest of this source is available to registered users.</p>
                      <a href="/auth/login" className="relative mt-4 inline-flex rounded-xl bg-orange-600 px-4 py-2 text-xs font-bold text-white hover:bg-orange-500">Log in / Register</a>
                    </div>
                  </div>
                )
              )}
            </div>
          </div>
        </div>
      )}

      {showDownloadPrompt && (
        <div className="fixed inset-0 z-[60] flex items-center justify-center bg-black/80 p-4 backdrop-blur-md">
          <div className="w-full max-w-md rounded-3xl border border-white/20 bg-[#121212] p-6 text-center shadow-2xl">
            <h2 className="text-lg font-black text-white">Login required</h2>
            <p className="mt-2 text-sm text-slate-300">You need to log in/ register to download the code.</p>
            <div className="mt-5 flex justify-center gap-3">
              <button onClick={() => setShowDownloadPrompt(false)} className="rounded-xl border border-white/15 px-4 py-2 text-xs font-bold text-slate-300 hover:bg-white/10">Close</button>
              <a href="/auth/login" className="rounded-xl bg-orange-600 px-4 py-2 text-xs font-bold text-white hover:bg-orange-500">Log in / Register</a>
            </div>
          </div>
        </div>
      )}

      <Footer />
    </main>
  );
}