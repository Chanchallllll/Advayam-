"use client";

import Link from "next/link";
import { CheckCircle2 } from "lucide-react";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import RegisterStepper from "./RegisterStepper";
import { getRegisterSession } from "@/lib/validation";

export default function RegisterComplete() {
  const router = useRouter();
  const [allowed, setAllowed] = useState(false);

  useEffect(() => {
    const params = new URLSearchParams(window.location.search);
    const preview = params.get("preview") === "true";

    const session = getRegisterSession();

    if (preview || session?.profileCompleted) {
      setAllowed(true);
      return;
    }

    router.replace("/auth/register");
  }, [router]);

  if (!allowed) return null;

  return (
    <div className="mx-auto w-full max-w-[620px]">
      <RegisterStepper currentStep={3} />

      <div className="mt-12 rounded-3xl border border-[#202020]/10 bg-white p-10 text-center shadow-sm">
        <CheckCircle2 className="mx-auto h-16 w-16 text-[#d95b00]" />

        <h1 className="mt-6 font-jakarta text-4xl font-black text-[#202020]">
          Account Created
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-[#202020]/60">
          Welcome to the Advayam ecosystem.
        </p>

        <Link
          href="/dashboard"
          className="mt-8 inline-flex h-14 w-full items-center justify-center rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white"
        >
          Go to Dashboard
        </Link>
      </div>
    </div>
  );
}