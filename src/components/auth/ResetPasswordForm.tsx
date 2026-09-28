"use client";

import Link from "next/link";
import { Eye, EyeOff } from "lucide-react";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { isStrongPassword } from "@/lib/validation";
import { apiFetch } from "@/lib/api";

export default function ResetPasswordForm() {
  const router = useRouter();

  const [allowed, setAllowed] = useState(false);
  const [password, setPassword] = useState("");
  const [confirmPassword, setConfirmPassword] = useState("");

  const [showPassword, setShowPassword] = useState(false);
  const [showConfirmPassword, setShowConfirmPassword] = useState(false);
  const [complete, setComplete] = useState(false);
  const [error, setError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    const verified = localStorage.getItem("advayam-reset-verified");

    if (verified === "true") {
      setAllowed(true);
      return;
    }

    router.replace("/auth/forgot-password");
  }, [router]);

  async function resetPassword() {
    setError("");

    if (!password || !confirmPassword) {
      setError("Enter and confirm your new password.");
      return;
    }

    if (!isStrongPassword(password)) {
      setError(
        "Password must contain 8+ characters, uppercase, lowercase, number and special character."
      );
      return;
    }

    if (password !== confirmPassword) {
      setError("Passwords do not match.");
      return;
    }

    const email = localStorage.getItem("advayam-reset-email") || "";
    const token = localStorage.getItem("advayam-reset-token") || "";

    if (!email || !token) {
      setError("Password reset session expired. Please start over.");
      return;
    }

    setIsLoading(true);
    try {
      await apiFetch("/auth/reset-password-otp", {
        method: "POST",
        body: JSON.stringify({ email, newPassword: password, token }),
      });

      localStorage.removeItem("advayam-reset-verified");
      localStorage.removeItem("advayam-reset-otp-sent");
      localStorage.removeItem("advayam-reset-email");
      localStorage.removeItem("advayam-reset-token");
      
      setComplete(true);
    } catch (err: any) {
      setError(err.message || "Failed to reset password. Please try again.");
    } finally {
      setIsLoading(false);
    }
  }

  if (!allowed) return null;

  if (complete) {
    return (
      <div className="mx-auto w-full max-w-[620px] rounded-3xl border border-[#202020]/10 bg-white p-10 text-center shadow-sm">
        <h1 className="font-jakarta text-4xl font-black text-[#202020]">
          Password Reset Complete
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-[#202020]/60">
          You can now login with your new password.
        </p>

        <Link
          href="/auth/login"
          className="mt-8 inline-flex h-14 w-full items-center justify-center rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white"
        >
          Go to Login
        </Link>
      </div>
    );
  }

  return (
    <div className="mx-auto w-full max-w-[620px]">
      <div className="text-center">
        <h1 className="font-jakarta text-4xl font-black text-[#202020]">
          Reset Password
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-[#202020]/60">
          Create a new secure password for your Advayam account.
        </p>
      </div>

      <div className="mt-10">
        <label className="block font-jakarta text-sm font-black">
          New Password
        </label>

        <div className="mt-3 flex h-14 items-center rounded-xl border border-[#202020]/15 bg-white px-5 focus-within:border-[#d95b00]">
          <input
            type={showPassword ? "text" : "password"}
            placeholder="New Password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            className="w-full bg-transparent font-jakarta text-sm outline-none"
          />

          <button
            type="button"
            onClick={() => setShowPassword(!showPassword)}
            className="text-[#202020]/50"
          >
            {showPassword ? (
              <EyeOff className="h-5 w-5" />
            ) : (
              <Eye className="h-5 w-5" />
            )}
          </button>
        </div>

        <label className="mt-6 block font-jakarta text-sm font-black">
          Confirm Password
        </label>

        <div className="mt-3 flex h-14 items-center rounded-xl border border-[#202020]/15 bg-white px-5 focus-within:border-[#d95b00]">
          <input
            type={showConfirmPassword ? "text" : "password"}
            placeholder="Confirm Password"
            value={confirmPassword}
            onChange={(e) => setConfirmPassword(e.target.value)}
            className="w-full bg-transparent font-jakarta text-sm outline-none"
          />

          <button
            type="button"
            onClick={() => setShowConfirmPassword(!showConfirmPassword)}
            className="text-[#202020]/50"
          >
            {showConfirmPassword ? (
              <EyeOff className="h-5 w-5" />
            ) : (
              <Eye className="h-5 w-5" />
            )}
          </button>
        </div>

        {error && (
          <p className="mt-5 rounded-xl border border-red-200 bg-red-50 px-4 py-3 font-jakarta text-sm font-bold text-red-600">
            {error}
          </p>
        )}

        <button
          type="button"
          disabled={isLoading}
          onClick={resetPassword}
          className="mt-8 h-14 w-full rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50"
        >
          {isLoading ? "Resetting Password..." : "Reset Password"}
        </button>
      </div>
    </div>
  );
}