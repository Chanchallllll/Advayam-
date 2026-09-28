"use client";

import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import RegisterStepper from "./RegisterStepper";
import {
  getRegisterSession,
  isStrongPassword,
  saveRegisterSession,
} from "@/lib/validation";
import { useAuth } from "@/context/AuthContext";
import { apiFetch } from "@/lib/api";

export default function ProfileDetailsForm() {
  const router = useRouter();
  const { login } = useAuth();

  const [allowed, setAllowed] = useState(false);
  const [error, setError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

  const [profile, setProfile] = useState({
    username: "",
    email: "",
    password: "",
    confirmPassword: "",
  });

  useEffect(() => {
    const params = new URLSearchParams(window.location.search);
    const preview = params.get("preview") === "true";

    const session = getRegisterSession();
    const savedEmail = localStorage.getItem("advayam-register-email");

    if (preview || session?.emailVerified) {
      setAllowed(true);
      if (savedEmail && !profile.email) {
        setProfile((prev) => ({ ...prev, email: savedEmail }));
      }
      return;
    }

    router.replace("/auth/register");
  }, [router, profile.email]);

  async function submitProfile() {
    setError("");

    if (
      !profile.username ||
      !profile.email ||
      !profile.password ||
      !profile.confirmPassword
    ) {
      setError("Please fill all required fields.");
      return;
    }

    if (!isStrongPassword(profile.password)) {
      setError(
        "Password must contain 8+ characters, uppercase, lowercase, number and special character."
      );
      return;
    }

    if (profile.password !== profile.confirmPassword) {
      setError("Passwords do not match.");
      return;
    }

    const token = localStorage.getItem("advayam-register-token") || "";
    if (!token) {
      setError("Email verification session expired. Please register again.");
      return;
    }

    setIsLoading(true);
    try {
      const response = await apiFetch("/auth/complete-profile", {
        method: "POST",
        body: JSON.stringify({
          email: profile.email,
          name: profile.username,
          password: profile.password,
          token,
        }),
      });

      // Login immediately with returned token and user object
      login(response.access_token, response.user, true);

      const role =
        (localStorage.getItem("advayam-register-role") as "developer" | "user") ||
        "developer";

      saveRegisterSession({
        role,
        emailVerified: true,
        profileCompleted: true,
      });

      // Clear register temp keys
      localStorage.removeItem("advayam-register-token");
      localStorage.removeItem("advayam-register-email");
      localStorage.removeItem("advayam-register-role");

      router.push("/auth/register/complete");
    } catch (err: any) {
      setError(err.message || "Failed to create account. Please try again.");
    } finally {
      setIsLoading(false);
    }
  }

  if (!allowed) return null;

  return (
    <div className="mx-auto w-full max-w-[720px]">
      <RegisterStepper currentStep={2} />

      <div className="mt-12 text-center">
        <h1 className="font-jakarta text-4xl font-black text-[#202020]">
          Profile details
        </h1>

        <p className="mt-3 font-jakarta text-sm font-medium text-[#202020]/60">
          Complete your Advayam profile
        </p>
      </div>

      <div className="mt-8 grid grid-cols-2 gap-4 max-sm:grid-cols-1">
        {Object.entries(profile).map(([key, value]) => (
          <Field
            key={key}
            label={toLabel(key)}
            type={key.toLowerCase().includes("password") ? "password" : "text"}
            value={value}
            onChange={(next) =>
              setProfile((prev) => ({
                ...prev,
                [key]: next,
              }))
            }
          />
        ))}
      </div>

      {error && (
        <p className="mt-5 rounded-xl border border-red-200 bg-red-50 px-4 py-3 font-jakarta text-sm font-bold text-red-600">
          {error}
        </p>
      )}

      <button
        type="button"
        disabled={isLoading}
        onClick={submitProfile}
        className="mt-8 h-14 w-full rounded-xl bg-[#d95b00] font-jakarta text-sm font-black text-white hover:bg-[#bd4f00] disabled:opacity-50"
      >
        {isLoading ? "Creating Account..." : "Create Account"}
      </button>
    </div>
  );
}

function Field({
  label,
  value,
  onChange,
  type = "text",
}: {
  label: string;
  value: string;
  onChange: (value: string) => void;
  type?: string;
}) {
  return (
    <label>
      <span className="font-jakarta text-sm font-black">{label}</span>

      <input
        type={type}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        className="mt-2 h-12 w-full rounded-xl border border-[#202020]/15 bg-white px-4 font-jakarta text-sm outline-none focus:border-[#d95b00]"
      />
    </label>
  );
}

function toLabel(key: string) {
  return key.replace(/([A-Z])/g, " $1").replace(/^./, (s) => s.toUpperCase());
}