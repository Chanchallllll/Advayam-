"use client";

import Link from "next/link";
import { Menu, Search, X } from "lucide-react";
import { useState } from "react";
import { runSiteSearch } from "@/lib/siteSearch";

export default function AuthHeader() {
  const [open, setOpen] = useState(false);
  const [query, setQuery] = useState("");

  const closeMenu = () => setOpen(false);

  return (
    <header className="fixed left-3 right-3 top-3 z-50 rounded-full border border-white/15 bg-[#0A0A0A]/95 shadow-2xl shadow-black/50 backdrop-blur-md sm:left-4 sm:right-4 sm:top-4">
      <div className="mx-auto flex h-[64px] w-full max-w-[1500px] items-center justify-between gap-4 px-4 sm:h-[68px] sm:px-6 lg:px-8 xl:px-10">

        {/* Logo */}
        <Link
          href="/"
          className="flex shrink-0 items-center"
          onClick={closeMenu}
        >
          <img
            src="/logos/advayam-logo.png"
            alt="Advayam"
            className="h-12 w-auto object-contain transition-transform hover:scale-105 sm:h-14 md:h-16 lg:h-18"
          />
        </Link>

        {/* Desktop Navigation */}
        <nav className="hidden items-center justify-center gap-6 font-jakarta text-sm font-bold text-white lg:flex xl:gap-10">
          <Link
            href="/community"
            className="whitespace-nowrap transition-colors hover:text-[#d95b00]"
          >
            Community
          </Link>

          <Link
            href="/downloads"
            className="whitespace-nowrap transition-colors hover:text-[#d95b00]"
          >
            Downloads
          </Link>

          <Link
            href="/support"
            className="whitespace-nowrap transition-colors hover:text-[#d95b00]"
          >
            Help
          </Link>
        </nav>

        {/* Desktop Search + Login + Register */}
        <div className="hidden shrink-0 items-center gap-3 lg:flex xl:gap-4">
          {/* Search */}
          <form
            onSubmit={(e) => {
              e.preventDefault();
              runSiteSearch(query);
            }}
            className="flex h-10 w-[190px] items-center rounded-full border border-white/20 bg-[#151515] pl-4 pr-1.5 text-white transition-colors focus-within:border-[#d95b00] xl:h-11 xl:w-[230px]"
          >
            <input
              type="search"
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              placeholder="Search"
              className="w-full bg-transparent pr-2 font-jakarta text-sm text-white outline-none placeholder:text-white/50"
            />

            <button
              type="submit"
              aria-label="Search"
              className="flex h-8 w-8 shrink-0 cursor-pointer items-center justify-center rounded-full text-white/70 transition-all hover:bg-white/10 hover:text-[#d95b00] active:scale-95"
            >
              <Search className="h-4 w-4" />
            </button>
          </form>

          {/* Login */}
          <Link
            href="/auth/login"
            className="whitespace-nowrap rounded-lg border border-white/20 bg-transparent px-4 py-2.5 font-jakarta text-sm font-bold text-white transition-colors hover:border-white/40 xl:px-5"
          >
            Login
          </Link>

          {/* Register */}
          <Link
            href="/auth/register"
            className="whitespace-nowrap rounded-lg bg-[#d95b00] px-4 py-2.5 font-jakarta text-sm font-bold text-white transition-colors hover:bg-[#bd4f00] xl:px-5"
          >
            Register
          </Link>
        </div>

        {/* Tablet + Mobile Menu Button */}
        <button
          type="button"
          onClick={() => setOpen((value) => !value)}
          aria-label={open ? "Close menu" : "Open menu"}
          aria-expanded={open}
          className="flex h-10 w-10 shrink-0 items-center justify-center rounded-full border border-white/20 text-white transition-colors hover:border-white/40 lg:hidden sm:h-11 sm:w-11"
        >
          {open ? (
            <X className="h-5 w-5" />
          ) : (
            <Menu className="h-5 w-5" />
          )}
        </button>
      </div>

      {/* Responsive Menu */}
      {open && (
        <div className="absolute left-0 right-0 top-[72px] rounded-2xl border border-white/15 bg-[#0A0A0A] p-4 text-white shadow-2xl sm:top-[76px] sm:p-5 lg:hidden">
          
          {/* Search */}
          <form
            onSubmit={(e) => {
              e.preventDefault();
              runSiteSearch(query);
              closeMenu();
            }}
            className="flex h-11 w-full items-center rounded-full border border-white/20 bg-[#151515] pl-4 pr-1.5 text-white transition-colors focus-within:border-[#d95b00]"
          >
            <input
              type="search"
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              placeholder="Search"
              className="w-full bg-transparent pr-2 font-jakarta text-sm text-white outline-none placeholder:text-white/50"
            />

            <button
              type="submit"
              aria-label="Search"
              className="flex h-8 w-8 shrink-0 cursor-pointer items-center justify-center rounded-full text-white/70 transition-all hover:bg-white/10 hover:text-[#d95b00] active:scale-95"
            >
              <Search className="h-4 w-4" />
            </button>
          </form>

          {/* Navigation */}
          <nav className="mt-5 grid gap-1 font-jakarta text-sm font-bold">
            <Link
              href="/community"
              onClick={closeMenu}
              className="rounded-lg px-3 py-3 transition-colors hover:bg-white/5 hover:text-[#d95b00]"
            >
              Community
            </Link>

            <Link
              href="/downloads"
              onClick={closeMenu}
              className="rounded-lg px-3 py-3 transition-colors hover:bg-white/5 hover:text-[#d95b00]"
            >
              Downloads
            </Link>

            <Link
              href="/support"
              onClick={closeMenu}
              className="rounded-lg px-3 py-3 transition-colors hover:bg-white/5 hover:text-[#d95b00]"
            >
              Help
            </Link>

            <Link
              href="/auth/login"
              onClick={closeMenu}
              className="rounded-lg px-3 py-3 transition-colors hover:bg-white/5 hover:text-[#d95b00]"
            >
              Login
            </Link>

            <Link
              href="/auth/register"
              onClick={closeMenu}
              className="mt-2 rounded-lg bg-[#d95b00] px-5 py-3 text-center text-white transition-colors hover:bg-[#bd4f00]"
            >
              Register
            </Link>
          </nav>
        </div>
      )}
    </header>
  );
}