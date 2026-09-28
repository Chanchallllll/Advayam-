"use client";

import Link from "next/link";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import type { UserRole } from "@/types/auth";
import { isValidEmail, isValidOTP, saveRegisterSession } from "@/lib/validation";
import RegisterStepper from "./RegisterStepper";
import RoleSelector from "./RoleSelector";
import OTPInput from "./OTPInput";
import { apiFetch } from "@/lib/api";

export default function RegisterForm() {
  const router = useRouter();

  const [role, setRole] = useState<UserRole>("developer");
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
      await apiFetch("/auth/send-otp", {
        method: "POST",
        body: JSON.stringify({ email, role }),
      });

      setEmailSubmitted(true);
      setCountdown(25);

      localStorage.setItem("advayam-register-role", role);
      localStorage.setItem("advayam-register-email", email);
    } catch (err: any) {
      setError(err.message || "Failed to send verification code. Please try again.");
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
      const response = await apiFetch("/auth/verify-otp", {
        method: "POST",
        body: JSON.stringify({ email, otp }),
      });

      // Save token returned from OTP verification
      localStorage.setItem("advayam-register-token", response.token);

      saveRegisterSession({
        role,
        emailVerified: true,
        profileCompleted: false,
      });

      router.push("/auth/register/profile");
    } catch (err: any) {
      setError(err.message || "Invalid or expired verification code.");
    } finally {
      setIsVerifying(false);
    }
  }

  return (
    <div className="mx-auto w-full max-w-[620px]">
      <RegisterStepper currentStep={1} />

      <div className="mt-12 text-center">
        <h1 className="font-jakarta text-4xl font-black text-white">
          Create your account
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-white/60">
          Join as a Developer or User
        </p>
      </div>

      <div className="mt-8">
        <RoleSelector role={role} setRole={setRole} />

        <label className="mt-8 block font-jakarta text-sm font-black">
          Email address
        </label>

        <div className="mt-3 flex gap-3">
          <input
            type="email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            placeholder="Enter your email"
            className="w-full rounded-xl bg-white font-jakarta text-base text-[#202020] outline-none placeholder:text-[#202020]/50"
          />

          <button
            type="button"
            disabled={isSending}
            onClick={submitEmail}
            className="h-14 rounded-xl bg-[#d95b00] px-6 font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50"
          >
            {isSending ? "Sending..." : "Submit"}
          </button>
        </div>

        {emailSubmitted && (
          <div className="mt-8">
            <h2 className="font-jakarta text-lg font-black">
              Verify your email
            </h2>

            <p className="mt-2 font-jakarta text-sm font-medium text-white/60">
              We’ve sent a 6-digit OTP to your email
            </p>

            <label className="mt-6 block font-jakarta text-sm font-black">
              Enter OTP
            </label>

            <div className="mt-3">
              <OTPInput value={otp} onChange={setOtp} />
            </div>

            <p className="mt-5 font-jakarta text-sm font-medium text-white/60">
              Didn’t receive the code?{" "}
              <button
                type="button"
                disabled={countdown > 0}
                onClick={submitEmail}
                className="font-black text-[#d95b00] disabled:opacity-50"
              >
                Resend OTP{" "}
                {countdown > 0
                  ? `(00:${String(countdown).padStart(2, "0")})`
                  : ""}
              </button>
            </p>
          </div>
        )}

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

        <p className="mt-8 text-center font-jakarta text-sm font-medium text-white/70">
          Already have an account?{" "}
          <Link href="/auth/login" className="font-black text-[#d95b00]">
            Login
          </Link>
        </p>
      </div>
    </div>
  );
}