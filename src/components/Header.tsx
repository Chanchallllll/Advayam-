"use client";

import { useState } from "react";
import { ChevronDown, Menu, Search, X } from "lucide-react";
import { runSiteSearch } from "@/lib/siteSearch";
import { useAuth } from "@/context/AuthContext";


const isSubdomainConfigured =
  !!process.env.NEXT_PUBLIC_COMMUNITY_URL;

const communityUrl =
  process.env.NEXT_PUBLIC_COMMUNITY_URL ||
  "https://community.advayamtech.in";

const extensionsUrl =
  process.env.NEXT_PUBLIC_EXTENSIONS_URL ||
  "https://extensions.advayamtech.in";

const baseCommunityUrl = isSubdomainConfigured
  ? communityUrl
  : "/community";

const communityLinks = isSubdomainConfigured
  ? [
      {
        label: "Community Home",
        href: communityUrl,
      },
      {
        label: "Discussions",
        href: `${communityUrl}/discussions`,
      },
      {
        label: "Showcase",
        href: `${communityUrl}/showcase`,
      },
      {
        label: "Extensions",
        href: extensionsUrl,
      },
      {
        label: "Leaderboard",
        href: `${communityUrl}/leaderboard`,
      },
    ]
  : [
      {
        label: "Community Home",
        href: "/community",
      },
      {
        label: "Demo Programs",
        href: "/community/demo-programs",
      },
      {
        label: "Contribution Guidelines",
        href: "/community/contribution-guidelines",
      },
      {
        label: "Mentors",
        href: "/community/mentors",
      },
      {
        label: "Contributors",
        href: "/community/contributors",
      },
    ];

export default function Header() {
  const [mobileOpen, setMobileOpen] = useState(false);
  const [query, setQuery] = useState("");

  const { user, logout } = useAuth();

  const toggleMobileMenu = () => {
    setMobileOpen((value) => !value);
  };

  const closeMobileMenu = () => {
    setMobileOpen(false);
  };

  const handleSearch = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (!query.trim()) {
      return;
    }

    runSiteSearch(query);
    closeMobileMenu();
  };

  return (
    <header className="fixed inset-x-0 top-0 z-[9999] w-full px-2 py-2 sm:px-3 sm:py-3 md:px-4 md:py-4">
      <div className="relative mx-auto w-full max-w-[1800px] rounded-2xl border border-white/15 bg-[#0c0c0c]/95 text-white shadow-2xl shadow-black/50 backdrop-blur-xl sm:rounded-3xl md:rounded-full">

        {/* =========================================================
            MAIN HEADER
        ========================================================== */}
        <div className="flex min-h-16 w-full items-center px-3 sm:min-h-[68px] sm:px-5 md:px-6 lg:px-8">

          {/* =======================================================
              LOGO
          ======================================================== */}
          <a
            href="/"
            className="flex shrink-0 items-center"
          >
            <img
              src="/logos/advayam-logo.png"
             alt="Advayam ll"
              className="h-10 w-auto object-contain sm:h-12 md:h-14 lg:h-16"
            />
          </a>

          {/* =======================================================
              DESKTOP NAVIGATION
              Visible at lg and above
          ======================================================== */}
          <nav className="ml-4 hidden min-w-0 flex-1 items-center gap-2 lg:flex xl:ml-6 xl:gap-3 2xl:ml-10 2xl:gap-4">

            {/* SEARCH */}
            <form
              onSubmit={handleSearch}
              className="flex h-11 w-[190px] shrink-0 items-center rounded-full border border-white/20 bg-white/[0.06] pl-4 pr-1.5 transition-colors focus-within:border-[#BE5209] xl:w-[220px] 2xl:w-[300px]"
            >
              <input
                type="search"
                placeholder="Search"
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                className="w-full min-w-0 bg-transparent pr-2 text-sm text-white outline-none placeholder:text-white/50"
              />

              <button
                type="submit"
                aria-label="Search"
                className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full text-white/60 transition-all hover:bg-white/10 hover:text-[#EA580C] active:scale-95"
              >
                <Search className="h-4 w-4" />
              </button>
            </form>

            <div className="flex-1" />

            {/* COMMUNITY */}
            <Dropdown
              label="Community"
              href={baseCommunityUrl}
            >
              {communityLinks.map((link) => (
                <a
                  key={link.href}
                  href={link.href}
                >
                  {link.label}
                </a>
              ))}
            </Dropdown>

            {/* DOCUMENTATION */}
            <Dropdown
              label="Documentation"
              href="/documentation"
            >
              <a href="/documentation">
                Docs Home
              </a>

              <a href="/downloads">
                Downloads
              </a>

              <a href="/documentation#projects">
                Projects
              </a>
            </Dropdown>

            {/* TARGET AUDIENCE */}
            <Dropdown
              label="Target Audience"
              href="/learningpath"
            >
              <a href="/learningpath/students">
                Students
              </a>

              <a href="/learningpath/professionals">
                Professionals
              </a>

              <a href="/learningpath/makers">
                Makers
              </a>
            </Dropdown>
          </nav>

          {/* =======================================================
              DESKTOP ACCOUNT
              Visible at lg and above
          ======================================================== */}
          <div className="ml-3 hidden shrink-0 lg:flex xl:ml-4 2xl:ml-6">
            {user ? (
              <Dropdown
                label={user.name || "Account"}
                href="/dashboard"
              >
                <a href="/dashboard">
                  Dashboard
                </a>

                {user.role === "admin" && (
                  <a href="/admin">
                    Admin Panel
                  </a>
                )}

                <button
                  type="button"
                  onClick={logout}
                  className="w-full text-left font-jakarta text-sm font-medium text-red-400 hover:text-red-300"
                >
                  Logout
                </button>
              </Dropdown>
            ) : (
              <div className="flex items-center">
                <a
                  href="/auth/login"
                  className="flex h-10 items-center justify-center whitespace-nowrap rounded-full bg-gradient-to-r from-[#EA580C] to-[#F97316] px-5 text-xs font-extrabold text-white shadow-md shadow-orange-500/25 transition-all hover:scale-[1.03] active:scale-[0.98]"
                >
                  Login
                </a>
              </div>
            )}
          </div>

          {/* =======================================================
              MOBILE / TABLET MENU BUTTON
              Visible below lg
          ======================================================== */}
          <div className="ml-auto flex shrink-0 lg:hidden">
            <button
              type="button"
              onClick={toggleMobileMenu}
              aria-label={
                mobileOpen
                  ? "Close navigation menu"
                  : "Open navigation menu"
              }
              aria-expanded={mobileOpen}
              aria-controls="mobile-navigation"
              className="relative z-[99999] flex h-11 w-11 cursor-pointer touch-manipulation items-center justify-center rounded-full border border-white/20 bg-white/[0.08] text-white transition-all duration-200 hover:bg-white/15 active:scale-95"
            >
              {mobileOpen ? (
                <X
                  className="h-6 w-6"
                  strokeWidth={2}
                />
              ) : (
                <Menu
                  className="h-6 w-6"
                  strokeWidth={2}
                />
              )}
            </button>
          </div>
        </div>

        {/* =========================================================
            MOBILE MENU
        ========================================================== */}
        {mobileOpen && (
          <div
            id="mobile-navigation"
            className="absolute left-0 right-0 top-[calc(100%+8px)] z-[99999] max-h-[calc(100vh-100px)] overflow-y-auto rounded-2xl border border-white/15 bg-[#111111] p-4 text-white shadow-2xl shadow-black/60 sm:p-5 md:rounded-3xl lg:hidden"
          >
            {/* MOBILE SEARCH */}
            <form
              onSubmit={handleSearch}
              className="flex h-11 w-full items-center rounded-full border border-white/20 bg-white/[0.06] pl-4 pr-1.5 focus-within:border-[#BE5209]"
            >
              <input
                type="search"
                placeholder="Search"
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                className="w-full min-w-0 bg-transparent pr-2 text-sm text-white outline-none placeholder:text-white/50"
              />

              <button
                type="submit"
                aria-label="Search"
                className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full text-white/70 transition-colors hover:bg-white/10 hover:text-[#EA580C]"
              >
                <Search className="h-4 w-4" />
              </button>
            </form>

            {/* MOBILE LINKS */}
            <div className="mt-4 grid gap-1 text-sm font-semibold">

              {/* COMMUNITY */}
              <a
                href="/community"
                onClick={closeMobileMenu}
                className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
              >
                Community
              </a>

              {/* DOCUMENTATION */}
              <a
                href="/documentation"
                onClick={closeMobileMenu}
                className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
              >
                Documentation
              </a>

              {/* DOWNLOADS */}
              <a
                href="/downloads"
                onClick={closeMobileMenu}
                className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
              >
                Downloads
              </a>

              <a
                href="/documentation#projects"
                onClick={closeMobileMenu}
                className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
              >
                Projects
              </a>

              {/* TARGET AUDIENCE */}
              <a
                href="/learningpath"
                onClick={closeMobileMenu}
                className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
              >
                Target Audience
              </a>

              {/* USER */}
              {user ? (
                <>
                  <a
                    href="/dashboard"
                    onClick={closeMobileMenu}
                    className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
                  >
                    Dashboard
                  </a>

                  {user.role === "admin" && (
                    <a
                      href="/admin"
                      onClick={closeMobileMenu}
                      className="rounded-xl px-3 py-3 transition-colors hover:bg-white/10"
                    >
                      Admin Panel
                    </a>
                  )}

                  <button
                    type="button"
                    onClick={() => {
                      logout();
                      closeMobileMenu();
                    }}
                    className="rounded-xl px-3 py-3 text-left text-red-400 transition-colors hover:bg-white/10"
                  >
                    Logout
                  </button>
                </>
              ) : (
                <div className="mt-2 grid gap-2 border-t border-white/10 pt-3">
                  <a
                    href="/auth/login"
                    onClick={closeMobileMenu}
                    className="flex h-11 items-center justify-center rounded-full bg-gradient-to-r from-[#EA580C] to-[#F97316] text-sm font-bold text-white transition-all hover:scale-[1.01] active:scale-[0.98]"
                  >
                    Login
                  </a>
                </div>
              )}

              {/* WEBYFY */}
              <div className="mt-3 flex flex-col items-center gap-2 border-t border-white/10 pt-4 text-xs font-bold text-slate-400">
                <span>
                  designed &amp; managed by
                </span>

                <a
                  href="https://www.webyfy.com"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="transition-opacity hover:opacity-80"
                >
                  <img
                    src="/logos/webyfy-logo.svg"
                    alt="Webyfy IoT"
                    className="h-9 w-auto object-contain"
                  />
                </a>
              </div>
            </div>
          </div>
        )}
      </div>
    </header>
  );
}

/* ===============================================================
   DROPDOWN
================================================================ */

function Dropdown({
  label,
  href,
  children,
}: {
  label: string;
  href?: string;
  children: React.ReactNode;
}) {
  return (
    <div className="group relative">
      <a
        href={href}
        className="flex items-center gap-2 whitespace-nowrap rounded-full border border-white/20 bg-white/[0.04] px-4 py-2.5 text-sm font-medium text-white/90 transition-colors hover:bg-white/10 hover:text-white 2xl:px-5 2xl:py-3"
      >
        {label}

        <ChevronDown
          className="h-4 w-4 opacity-70"
        />
      </a>

      <div className="invisible absolute left-0 top-full z-[999] pt-2 opacity-0 transition-all duration-150 group-hover:visible group-hover:opacity-100">
        <div className="min-w-64 rounded-2xl border border-white/15 bg-[#111111]/95 p-3 shadow-2xl backdrop-blur-xl">
          <div className="flex flex-col gap-1 text-sm font-medium text-slate-200 [&_a]:rounded-xl [&_a]:px-4 [&_a]:py-3 [&_a]:transition-colors [&_a:hover]:bg-white/10 [&_a:hover]:text-white">
            {children}
          </div>
        </div>
      </div>
    </div>
  );
}