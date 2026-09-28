"use client";

import { useEffect, Suspense } from "react";
import { useSearchParams, useRouter } from "next/navigation";
import { useAuth } from "@/context/AuthContext";

function CallbackHandler() {
  const searchParams = useSearchParams();
  const router = useRouter();
  const { login } = useAuth();

  useEffect(() => {
    const token = searchParams.get("token");
    const userStr = searchParams.get("user");
    const error = searchParams.get("error");

    if (error) {
      router.push(`/auth/login?error=${encodeURIComponent(error)}`);
      return;
    }

    if (token && userStr) {
      try {
        const user = JSON.parse(decodeURIComponent(userStr));
        login(token, user, true);

        if (user.role === "admin") {
          router.push("/admin");
        } else {
          router.push("/dashboard");
        }
      } catch (err) {
        console.error("Failed to parse OAuth user profile:", err);
        router.push("/auth/login?error=Invalid session response");
      }
    } else {
      router.push("/auth/login");
    }
  }, [searchParams, router, login]);

  return (
    <div className="flex min-h-screen flex-col items-center justify-center bg-white p-6 text-center font-jakarta">
      <div className="h-12 w-12 animate-spin rounded-full border-4 border-[#BE5209] border-t-transparent mb-4" />
      <h2 className="text-2xl font-bold text-[#202020]">Completing Authentication...</h2>
      <p className="mt-2 text-sm text-slate-500">Please wait while we log you into your Advayam account.</p>
    </div>
  );
}

export default function AuthCallbackPage() {
  return (
    <Suspense fallback={
      <div className="flex min-h-screen items-center justify-center font-jakarta">
        Loading...
      </div>
    }>
      <CallbackHandler />
    </Suspense>
  );
}
