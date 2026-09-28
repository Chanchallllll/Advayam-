"use client";

import { useEffect, useState } from "react";
import { MessageSquare, Eye, Flame, ArrowRight, Activity } from "lucide-react";

interface Discussion {
  id: string | number;
  title: string;
  category: string;
  replies: number;
  views: number;
  url: string;
  author: string;
  timeAgo: string;
}

// Fallback high-fidelity mock data (as listed in the request)
const FALLBACK_DISCUSSIONS: Discussion[] = [
  {
    id: 1,
    title: "How to flash ESP32-S3 using Adwayam IDE",
    category: "Guides & Tutorials",
    replies: 24,
    views: 412,
    url: "/discussions/flash-esp32s3",
    author: "sanjay_maker",
    timeAgo: "2 hours ago",
  },
  {
    id: 2,
    title: "PlatformIO board manager released",
    category: "Announcements",
    replies: 12,
    views: 189,
    url: "/discussions/platformio-released",
    author: "adwayam_dev",
    timeAgo: "5 hours ago",
  },
  {
    id: 3,
    title: "Zephyr support roadmap",
    category: "Roadmap",
    replies: 38,
    views: 654,
    url: "/discussions/zephyr-roadmap",
    author: "harish_k",
    timeAgo: "1 day ago",
  },
  {
    id: 4,
    title: "New MicroPython extension available",
    category: "Showcase",
    replies: 9,
    views: 143,
    url: "/discussions/micropython-ext",
    author: "iot_maker",
    timeAgo: "2 days ago",
  },
];

export default function TrendingDiscussions() {
  const [discussions, setDiscussions] = useState<Discussion[]>(FALLBACK_DISCUSSIONS);
  const [loading, setLoading] = useState(true);

  const communityUrl = process.env.NEXT_PUBLIC_COMMUNITY_URL || "https://community.advayamtech.in";

  useEffect(() => {
    async function fetchDiscussions() {
      try {
        const res = await fetch(`${communityUrl}/api/discussions`);
        if (res.ok) {
          const data = await res.json();
          if (Array.isArray(data) && data.length > 0) {
            setDiscussions(data);
          }
        }
      } catch (err) {
        // Fallback silently to mock data
      } finally {
        setLoading(false);
      }
    }
    fetchDiscussions();
  }, [communityUrl]);

  return (
    <section className="bg-[#ECECEC] px-6 py-20 text-[#202020]">
      <div className="mx-auto max-w-5xl">
        <div className="flex flex-col items-center text-center mb-12">
          <div className="inline-flex items-center gap-2 px-4 py-1.5 rounded-full bg-[#BE5209]/10 text-[#BE5209] text-sm font-semibold mb-4">
            <Flame className="h-4 w-4 fill-current" />
            <span>Community Activity</span>
          </div>
          
          <h2 className="font-jakarta text-3xl font-black leading-tight md:text-4xl text-[#202020]">
            🔥 Trending Discussions
          </h2>
          
          <p className="font-jakarta mt-4 max-w-2xl text-lg font-medium text-[#202020]/75">
            See what developers are building, discussing, and sharing in the Advayam ecosystem.
          </p>
        </div>

        <div className="grid gap-4 md:grid-cols-2">
          {discussions.map((discussion) => (
            <a
              key={discussion.id}
              href={`${communityUrl}${discussion.url.startsWith("/") ? "" : "/"}${discussion.url}`}
              target="_blank"
              rel="noopener noreferrer"
              className="group relative flex flex-col justify-between rounded-2xl border-2 border-transparent bg-white p-6 shadow-sm hover:border-[#BE5209]/30 hover:shadow-md transition-all duration-300"
            >
              <div>
                <div className="flex items-center justify-between gap-2 mb-3">
                  <span className="text-xs font-bold uppercase tracking-wider text-[#BE5209] px-2.5 py-0.5 rounded-md bg-[#BE5209]/5">
                    {discussion.category}
                  </span>
                  <span className="text-xs text-[#202020]/50 font-medium">
                    {discussion.timeAgo}
                  </span>
                </div>

                <h3 className="font-jakarta text-lg font-bold leading-snug text-[#202020] group-hover:text-[#BE5209] transition-colors duration-200">
                  {discussion.title}
                </h3>
              </div>

              <div className="mt-6 flex items-center justify-between pt-4 border-t border-black/5">
                <span className="text-xs text-[#202020]/60 font-semibold">
                  by <span className="text-[#202020]/80">@{discussion.author}</span>
                </span>
                
                <div className="flex items-center gap-4 text-[#202020]/60">
                  <span className="flex items-center gap-1 text-xs">
                    <MessageSquare className="h-3.5 w-3.5" />
                    {discussion.replies}
                  </span>
                  <span className="flex items-center gap-1 text-xs">
                    <Eye className="h-3.5 w-3.5" />
                    {discussion.views}
                  </span>
                </div>
              </div>
            </a>
          ))}
        </div>

        <div className="mt-12 flex justify-center">
          <a
            href={communityUrl}
            target="_blank"
            rel="noopener noreferrer"
            className="font-jakarta inline-flex items-center gap-2 rounded-full border-2 border-[#BE5209] bg-transparent px-6 py-3 text-sm font-bold text-[#BE5209] transition hover:bg-[#BE5209] hover:text-white"
          >
            View all discussions
            <ArrowRight className="h-4 w-4" />
          </a>
        </div>
      </div>
    </section>
  );
}
