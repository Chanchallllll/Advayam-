"use client";

import { HelpCircle } from "lucide-react";
import { useState } from "react";
import { apiFetch } from "@/lib/api";
import { useToast } from "@/context/ToastContext";

const isSubdomainConfigured = !!process.env.NEXT_PUBLIC_COMMUNITY_URL;
const communityUrl = process.env.NEXT_PUBLIC_COMMUNITY_URL || "https://community.advayamtech.in";
const extensionsUrl = process.env.NEXT_PUBLIC_EXTENSIONS_URL || "https://extensions.advayamtech.in";

const footerSections = [
  {
    title: "Solutions",
    links: [
      { label: "Professional", href: "/learningpath/professionals" },
      { label: "Education", href: "/learningpath/students" },
      { label: "Makers", href: "/learningpath/makers" },
    ],
  },
  {
    title: "Developers",
    links: [
      { label: "Documentation", href: "/documentation" },
      { label: "Downloads", href: "/downloads" },
      { label: "SDKs & APIs", href: "/documentation#sdks" },
      {
        label: "Examples & Tutorials",
        href: isSubdomainConfigured ? `${communityUrl}/showcase` : "/community/demo-programs"
      },
    ],
  },
  {
    title: "Community",
    links: isSubdomainConfigured
      ? [
        { label: "Community Home", href: communityUrl },
        { label: "Discussions", href: `${communityUrl}/discussions` },
        { label: "Showcase", href: `${communityUrl}/showcase` },
        { label: "Extensions", href: extensionsUrl },
        { label: "Leaderboard", href: `${communityUrl}/leaderboard` },
      ]
      : [
        { label: "Community Home", href: "/community" },
        { label: "Demo Programs", href: "/community/demo-programs" },
        {
          label: "Contribution Guidelines",
          href: "/community/contribution-guidelines",
        },
        { label: "Mentors", href: "/community/mentors" },
        { label: "Contributors", href: "/community/contributors" },
      ],
  },
  {
    title: "Support",
    links: [
      { label: "Help Center", href: "/support" },
      { label: "Raise Ticket", href: "/support/ticket" },
      { label: "Contact Us", href: "/contact" },
      { label: "Installation Guide", href: "/support#installation-guide" },
    ],
  },
  {
    title: "Company",
    links: [
      { label: "About Advayam", href: "/about" },
      { label: "Learning Path", href: "/learningpath" },
      { label: "Privacy and Terms", href: "/privacy" },
    ],
  },
];

import { SOCIAL_ITEMS } from "@/config/socials";

export default function Footer() {
  const { showToast } = useToast();
  const year = new Date().getFullYear();
  const [subscribeEmail, setSubscribeEmail] = useState("");
  const [submittingNewsletter, setSubmittingNewsletter] = useState(false);
  const [newsletterMessage, setNewsletterMessage] = useState("");
  const [newsletterError, setNewsletterError] = useState(false);

  const handleSubscribe = async (e: React.FormEvent) => {
    e.preventDefault();
    setNewsletterMessage("");
    setNewsletterError(false);

    if (!subscribeEmail) return;

    setSubmittingNewsletter(true);
    try {
      await apiFetch("/newsletter/subscribe", {
        method: "POST",
        body: JSON.stringify({ email: subscribeEmail }),
      });
      showToast("Thank you for subscribing to our newsletter!", "success");
      setNewsletterMessage("Thank you for subscribing to our newsletter!");
      setSubscribeEmail("");
    } catch (err: any) {
      setNewsletterError(true);
      const msg = err.message || "Failed to subscribe. Please try again.";
      showToast(msg, "error");
      setNewsletterMessage(msg);
    } finally {
      setSubmittingNewsletter(false);
    }
  };

  return (
    <footer className="w-full mt-auto bg-[#050505] text-white border-t border-white/10 font-jakarta">
      {/* Newsletter Subscription */}
      <div className="border-b border-white/10 px-6 py-6 bg-white/[0.02]">
        <div className="mx-auto max-w-7xl flex flex-col md:flex-row md:items-center justify-between gap-4">
          <div className="max-w-md">
            <h3 className="text-base font-bold text-white">
              Subscribe to our Newsletter
            </h3>
            <p className="mt-1 text-xs text-slate-400">
              Stay up to date with the latest hardware releases, software updates, and ecosystem news from Advayam.
            </p>
          </div>
          <form onSubmit={handleSubscribe} className="flex w-full md:w-auto max-w-md gap-2 flex-1 justify-end">
            <input
              type="email"
              placeholder="Enter your email"
              value={subscribeEmail}
              onChange={(e) => setSubscribeEmail(e.target.value)}
              className="h-10 w-full md:w-[260px] rounded-lg border border-white/20 bg-white/[0.06] px-3.5 text-xs text-white placeholder:text-slate-500 outline-none focus:border-[#BE5209] transition-colors"
              required
            />
            <button
              type="submit"
              disabled={submittingNewsletter}
              className="h-10 rounded-lg bg-[#BE5209] px-5 text-xs font-bold text-white shadow-md hover:bg-orange-600 transition disabled:opacity-50 shrink-0 cursor-pointer"
            >
              {submittingNewsletter ? "Subscribing..." : "Subscribe"}
            </button>
          </form>
        </div>
        {newsletterMessage && (
          <div className="mx-auto max-w-7xl mt-2">
            <p className={`text-xs font-bold ${newsletterError ? "text-red-400" : "text-green-400"}`}>
              {newsletterMessage}
            </p>
          </div>
        )}
      </div>

      {/* Links Grid */}
      <div className="px-6 py-8">
        <div className="mx-auto grid max-w-7xl gap-8 grid-cols-2 md:grid-cols-3 lg:grid-cols-5">
          {footerSections.map((section) => (
            <div key={section.title}>
              <h3 className="text-xs font-bold uppercase tracking-wider text-white">
                {section.title}
              </h3>

              <div className="mt-3 flex flex-col gap-2">
                {section.links.map((link) => (
                  <a
                    key={`${section.title}-${link.label}-${link.href}`}
                    href={link.href}
                    className="text-xs font-medium text-slate-400 hover:text-white transition-colors"
                  >
                    {link.label} ↗
                  </a>
                ))}
              </div>

              {section.title === "Company" && (
                <div className="mt-4 border-t border-white/10 pt-4">
                  <a
                    href="https://webyfy.com"
                    target="_blank"
                    rel="noopener noreferrer"
                    className="inline-block transition-transform hover:scale-105"
                  >
                    <img
                      src="/logos/webyfy-logo.svg"
                      alt="Webyfy"
                      className="h-10 w-8 object-contain"
                    />
                  </a>

                  <p className="mt-3 text-[10px] font-semibold uppercase tracking-[0.18em] text-slate-400">
                    Follow us
                  </p>

                  <div className="mt-2 flex items-center gap-2">
                    {SOCIAL_ITEMS.map((item, index) => {
                      const Icon = item.icon;

                      return (
                        <a
                          key={`social-${index}`}
                          href={item.href}
                          target="_blank"
                          rel="noopener noreferrer"
                          className="flex h-7 w-7 items-center justify-center rounded-full border border-white/20 bg-white/[0.04] text-slate-400 transition-colors hover:bg-white/15 hover:text-white"
                          aria-label={item.name}
                          title={item.name}
                        >
                          <Icon className="h-3.5 w-3.5" />
                        </a>
                      );
                    })}
                  </div>
                </div>
              )}
            </div>
          ))}
        </div>
      </div>

      {/* Bottom Bar: Brand Logo, Legal Links, Help, Copyright */}
      <div className="border-t border-white/10 px-6 py-4">
        <div className="mx-auto flex max-w-7xl flex-col gap-4 md:flex-row md:items-center md:justify-between">
          <div className="flex flex-wrap items-center gap-6">
            <a href="/">
              <img
                src="/logos/advayam-logo.png"
                alt="Advayam"
                className="h-9 w-auto object-contain transition-transform hover:scale-105"
              />
            </a>

            <a
              className="text-xs font-medium text-slate-400 hover:text-white transition-colors"
              href="/privacy"
            >
              Privacy and Terms
            </a>

            <a
              className="text-xs font-medium text-slate-400 hover:text-white transition-colors"
              href="/about"
            >
              About Advayam
            </a>

            <a
              className="text-xs font-medium text-slate-400 hover:text-white transition-colors"
              href="/community"
            >
              Community
            </a>
          </div>

          <div className="flex items-center gap-5">
            <a
              href="/support"
              className="flex items-center gap-1.5 text-xs font-medium text-slate-400 hover:text-white transition-colors"
            >
              <HelpCircle className="h-4 w-4" />
              Help
            </a>
            <span className="text-white/20">|</span>
            <p className="text-xs font-medium text-slate-500">
              Advayam © {year} All rights reserved.
            </p>
          </div>
        </div>
      </div>
    </footer>
  );
}