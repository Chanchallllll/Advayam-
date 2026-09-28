"use client";

import Link from "next/link";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { isValidEmail, isValidOTP } from "@/lib/validation";
import OTPInput from "./OTPInput";
import { apiFetch } from "@/lib/api";

export default function ForgotPasswordForm() {
  const router = useRouter();

  const [email, setEmail] = useState("");
  const [otp, setOtp] = useState("");
  const [emailSubmitted, setEmailSubmitted] = useState(false);
  const [countdown, setCountdown] = useState(25);
  const [error, setError] = useState("");
  const [isSending, setIsSending] = useState(false);
  const [isVerifying, setIsVerifying] = useState(false);

  useEffect(() => {
    if (!emailSubmitted || countdown <= 0) return;

    const timer = setInterval(() => {
      setCountdown((value) => value - 1);
    }, 1000);

    return () => clearInterval(timer);
  }, [emailSubmitted, countdown]);

  async function submitEmail() {
    setError("");

    if (!isValidEmail(email)) {
      setError("Enter a valid email address.");
      return;
    }

    setIsSending(true);
    try {
      await apiFetch("/auth/send-forgot-password-otp", {
        method: "POST",
        body: JSON.stringify({ email }),
      });

      localStorage.setItem("advayam-reset-email", email);
      localStorage.setItem("advayam-reset-otp-sent", "true");

      setEmailSubmitted(true);
      setCountdown(25);
    } catch (err: any) {
      setError(err.message || "Failed to send code. Please try again.");
    } finally {
      setIsSending(false);
    }
  }

  async function verifyOtp() {
    setError("");

    if (!emailSubmitted) {
      setError("Submit your email address first.");
      return;
    }

    if (!isValidOTP(otp)) {
      setError("Enter the 6-digit OTP.");
      return;
    }

    setIsVerifying(true);
    try {
      const response = await apiFetch("/auth/verify-forgot-password-otp", {
        method: "POST",
        body: JSON.stringify({ email, otp }),
      });

      // Save token returned from OTP verification
      localStorage.setItem("advayam-reset-token", response.token);
      localStorage.setItem("advayam-reset-verified", "true");

      router.push("/auth/forgot-password/reset");
    } catch (err: any) {
      setError(err.message || "Incorrect verification code.");
    } finally {
      setIsVerifying(false);
    }
  }

  return (
    <div className="mx-auto w-full max-w-[620px]">
      <div className="text-center">
        <h1 className="font-jakarta text-4xl font-black text-[#202020]">
          Forgot Password?
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-[#202020]/60">
          Enter your email and verify the OTP to reset your password.
        </p>
      </div>

      <div className="mt-10">
        <label className="block font-jakarta text-sm font-black">
          Email address
        </label>

        <div className="mt-3 flex gap-3 max-sm:flex-col">
          <input
            type="email"
            placeholder="Email Address"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            className="h-14 flex-1 rounded-xl border border-[#202020]/15 bg-white px-5 font-jakarta text-sm outline-none focus:border-[#d95b00]"
          />

          <button
            type="button"
            disabled={isSending}
            onClick={submitEmail}
            className="h-14 rounded-xl bg-[#d95b00] px-6 font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50"
          >
            {isSending ? "Sending..." : "Send OTP"}
          </button>
        </div>

        <div className="mt-9">
          <h2 className="font-jakarta text-lg font-black">Verify your email</h2>

          <p className="mt-2 font-jakarta text-sm font-medium text-[#202020]/60">
            We’ve sent a 6-digit OTP to{" "}
            <a
              href="https://mail.google.com"
              target="_blank"
              rel="noreferrer"
              className="font-black text-[#202020] underline underline-offset-4"
            >
              your email
            </a>
          </p>

          <label className="mt-6 block font-jakarta text-sm font-black">
            Enter OTP
          </label>

          <div className="mt-3">
            <OTPInput value={otp} onChange={setOtp} />
          </div>

          <p className="mt-5 font-jakarta text-sm font-medium text-[#202020]/60">
            Didn’t receive the code?{" "}
            <button
              type="button"
              disabled={!emailSubmitted || countdown > 0}
              onClick={() => setCountdown(25)}
              className="font-black text-[#d95b00] disabled:opacity-50"
            >
              Resend OTP{" "}
              {emailSubmitted && countdown > 0
                ? `(00:${String(countdown).padStart(2, "0")})`
                : ""}
            </button>
          </p>
        </div>

        {error && (
          <p className="mt-5 rounded-xl border border-red-200 bg-red-50 px-4 py-3 font-jakarta text-sm font-bold text-red-600">
            {error}
          </p>
        )}

        <button
          type="button"
          disabled={isVerifying || !emailSubmitted}
          onClick={verifyOtp}
          className="mt-8 h-14 w-full rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50"
        >
          {isVerifying ? "Verifying..." : "Verify & Continue"}
        </button>

        <p className="mt-8 text-center font-jakarta text-sm font-medium text-[#202020]/70">
          Remember your password?{" "}
          <Link href="/auth/login" className="font-black text-[#d95b00]">
            Login
          </Link>
        </p>
      </div>
    </div>
  );
}