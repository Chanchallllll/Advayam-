"use client";

import Footer from "@/components/Footer";
import Header from "@/components/Header";
import { useAuth } from "@/context/AuthContext";
import { Loader } from "lucide-react";
import { useEffect, useState } from "react";

export default function DashboardPage() {
  const { user, isLoading } = useAuth();
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
  }, []);

  useEffect(() => {
    if (user && user.role === "admin") {
      window.location.href = "/admin";
    }
  }, [user]);

  if (isLoading || !mounted) {
    return (
      <main className="flex min-h-screen flex-col bg-black text-white">
        <Header />
        <div className="flex-1 flex items-center justify-center pt-40">
          <Loader className="h-8 w-8 animate-spin text-[#BE5209]" />
        </div>
        <Footer />
      </main>
    );
  }

  if (!user) {
    return (
      <main className="flex min-h-screen flex-col bg-black text-white">
        <Header />
        <section className="flex-1 flex flex-col items-center justify-center px-6 pt-40 pb-20">
          <div className="max-w-md text-center p-8 bg-white border-2 border-black rounded-[2rem] shadow-sm">
            <h1 className="text-3xl font-black">Please Sign In</h1>
            <p className="mt-4 font-jakarta text-sm font-medium text-slate-600">
              You must be logged in to view your dashboard.
            </p>
            <a
              href="/auth/login"
              className="mt-6 inline-flex h-12 items-center justify-center rounded-xl bg-[#BE5209] px-6 text-sm font-semibold text-white hover:bg-black transition"
            >
              Go to Login
            </a>
          </div>
        </section>
        <Footer />
      </main>
    );
  }

  const isDeveloper = user.role === "developer";

  return (
    <main className="bg-black text-white min-h-screen flex flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40 flex-1">
        <div className="mx-auto max-w-8xl">
          <h1 className="text-5xl md:text-6xl font-black font-jakarta">Welcome to Advayam IDE</h1>

          {isDeveloper ? (
            <div className="mt-8 rounded-[2rem] border-2 border-dashed border-[#BE5209] bg-orange-50/50 p-12 text-center">
              <h2 className="text-3xl font-black text-[#BE5209]">Welcome Developer</h2>
              <p className="mt-4 text-base font-medium text-slate-600">
                Content coming soon! We are preparing the developer workspace, software SDKs, and IDE tools for you.
              </p>
            </div>
          ) : (
            <>
              <p className="mt-6 font-jakarta text-sm font-medium text-slate-600">
                Manage your Advayam projects, orders, downloads, and learning progress.
              </p>
              <div className="mt-12 grid gap-6 md:grid-cols-3">
                {[
                  "My Projects",
                  "Orders",
                  "Downloads",
                  "Learning Progress",
                  "Saved Docs",
                  "Account Settings",
                ].map((i) => (
                  <div
                    key={i}
                    className="rounded-[2rem] border-2 border-black p-8 font-black bg-white hover:bg-[#DBDBDB] transition cursor-pointer"
                  >
                    {i}
                  </div>
                ))}
              </div>
            </>
          )}
        </div>
      </section>
      <Footer />
    </main>
  );
}
