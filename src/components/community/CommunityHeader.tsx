"use client";

import Link from "next/link";
import { ChevronDown, Menu, Search, X } from "lucide-react";
import { useState } from "react";
import { runSiteSearch } from "@/lib/siteSearch";

type CommunityHeaderProps = {
  query?: string;
  setQuery?: (value: string) => void;
};

const communityLinks = [
  { label: "Community Home", href: "/community" },
  { label: "Demo Programs", href: "/community/demo-programs" },
  {
    label: "Contribution Guidelines",
    href: "/community/contribution-guidelines",
  },
  { label: "Mentors", href: "/community/mentors" },
  { label: "Contributors", href: "/community/contributors" },
];

export default function CommunityHeader({
  query = "",
  setQuery,
}: CommunityHeaderProps) {
  const [open, setOpen] = useState(false);
  const [localQuery, setLocalQuery] = useState("");

  const activeQuery = setQuery ? query : localQuery;

  function handleSearch() {
    if (setQuery) {
      runSiteSearch(query);
    } else {
      runSiteSearch(localQuery);
    }
  }

  return (
    <header className="fixed left-0 right-0 top-0 z-50 border-b border-white/10 bg-black/90 backdrop-blur-md">
      <div className="mx-auto grid h-[84px] max-w-[1500px] grid-cols-[250px_1fr_auto] items-center gap-8 px-10 max-lg:grid-cols-[180px_1fr_48px] max-sm:px-5">
       
        <nav className="flex items-center justify-center gap-10 font-jakarta text-sm font-bold text-white max-lg:hidden">
          <div className="group relative">
            <Link
              href="/community"
              className="flex items-center gap-2 border-b-2 border-[#d95b00] pb-2 text-[#d95b00]"
            >
              Community
              <ChevronDown className="h-4 w-4" />
            </Link>

            <div className="invisible absolute left-0 top-full z-[999] pt-4 opacity-0 transition group-hover:visible group-hover:opacity-100">
              <div className="min-w-72 rounded-2xl border border-white/15 bg-[#111] p-3 shadow-xl">
                <div className="flex flex-col gap-1">
                  {communityLinks.map((link) => (
                    <Link
                      key={link.href}
                      href={link.href}
                      className="rounded-xl px-4 py-3 text-white hover:bg-white/10"
                    >
                      {link.label}
                    </Link>
                  ))}
                </div>
              </div>
            </div>
          </div>

          <Link href="/downloads" className="hover:text-orange-400">
            Downloads
          </Link>

          <Link href="/support" className="hover:text-orange-400">
            Help
          </Link>
        </nav>

        <div className="flex items-center gap-4 max-lg:hidden">
          <form
            onSubmit={(e) => {
              e.preventDefault();
              handleSearch();
            }}
            className="flex h-11 w-[260px] items-center rounded-full border border-white/20 bg-white/[0.06] pl-4 pr-1.5 focus-within:border-[#d95b00] transition-colors"
          >
            <input
              type="search"
              placeholder="Search members, roles..."
              value={activeQuery}
              onChange={(e) =>
                setQuery ? setQuery(e.target.value) : setLocalQuery(e.target.value)
              }
              className="w-full bg-transparent font-jakarta text-sm text-white outline-none placeholder:text-white/40 pr-2"
            />
            <button
              type="submit"
              aria-label="Search"
              className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full text-white/60 hover:text-[#d95b00] hover:bg-white/10 active:scale-95 transition-all cursor-pointer"
            >
              <Search className="h-4 w-4 shrink-0" />
            </button>
          </form>

          <Link
            href="/auth/login"
            className="rounded-lg border border-white/15 bg-white/[0.06] px-5 py-2.5 font-jakarta text-sm font-bold text-white hover:bg-white/10"
          >
            Login
          </Link>

          <Link
            href="/auth/register"
            className="rounded-lg bg-[#d95b00] px-5 py-2.5 font-jakarta text-sm font-bold text-white"
          >
            Register
          </Link>
        </div>

        <button
          onClick={() => setOpen(!open)}
          className="hidden h-11 w-11 items-center justify-center rounded-full border border-white/30 text-white max-lg:flex"
        >
          {open ? <X className="h-5 w-5" /> : <Menu className="h-5 w-5" />}
        </button>
      </div>

      {open && (
        <div className="absolute left-5 right-5 top-[86px] rounded-2xl border border-white/15 bg-black/95 p-5 shadow-xl lg:hidden">
          <form
            onSubmit={(e) => {
              e.preventDefault();
              handleSearch();
            }}
            className="flex h-11 items-center rounded-full border border-white/20 bg-white/[0.06] pl-4 pr-1.5 focus-within:border-[#d95b00] transition-colors"
          >
            <input
              type="search"
              placeholder="Search"
              value={activeQuery}
              onChange={(e) =>
                setQuery ? setQuery(e.target.value) : setLocalQuery(e.target.value)
              }
              className="w-full bg-transparent text-sm outline-none placeholder:text-white/40 pr-2 text-white"
            />
            <button
              type="submit"
              aria-label="Search"
              className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full text-white/60 hover:text-[#d95b00] hover:bg-white/10 active:scale-95 transition-all cursor-pointer"
            >
              <Search className="h-4 w-4 shrink-0" />
            </button>
          </form>

          <div className="mt-5 grid gap-4 font-jakarta text-sm font-bold text-white">
            {communityLinks.map((link) => (
              <Link key={link.href} href={link.href}>
                {link.label}
              </Link>
            ))}
            <Link href="/downloads">Downloads</Link>
            <Link href="/support">Help</Link>
            <Link href="/auth/login">Login</Link>
            <Link
              href="/auth/register"
              className="rounded-lg bg-[#d95b00] px-5 py-3 text-center text-white"
            >
              Register
            </Link>
          </div>
        </div>
      )}
    </header>
  );
}