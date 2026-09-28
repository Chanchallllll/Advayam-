"use client";

import Link from "next/link";
import { Eye, EyeOff } from "lucide-react";
import { useEffect, useState } from "react";
import { FaGithub } from "react-icons/fa";
import { FcGoogle } from "react-icons/fc";
import type { UserRole } from "@/types/auth";
import RoleSelector from "./RoleSelector";
import SocialLoginButton from "./SocialLoginButton";
import { isValidEmail } from "@/lib/validation";
import { useAuth } from "@/context/AuthContext";
import { apiFetch } from "@/lib/api";

export default function LoginForm() {
  const { login } = useAuth();
  const [role, setRole] = useState<UserRole>("developer");
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");

  const [remember, setRemember] = useState(false);
  const [showPassword, setShowPassword] = useState(false);
  const [error, setError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    const saved = localStorage.getItem("advayam-remember-login");

    if (!saved) return;

    try {
      const parsed = JSON.parse(saved) as {
        email?: string;
        password?: string;
        role?: UserRole;
      };

      setEmail(parsed.email || "");
      setPassword(parsed.password || "");
      setRole(parsed.role || "developer");
      setRemember(true);
    } catch {
      localStorage.removeItem("advayam-remember-login");
    }
  }, []);

  async function handleLogin() {
    setError("");

    if (!email.trim() || !password.trim()) {
      setError("Email address and password are required.");
      return;
    }

    if (!isValidEmail(email)) {
      setError("Enter a valid email address.");
      return;
    }

    setIsLoading(true);
    try {
      const response = await apiFetch("/auth/login", {
        method: "POST",
        body: JSON.stringify({ email, password }),
      });

      login(response.access_token, response.user, remember);

      if (remember) {
        localStorage.setItem(
          "advayam-remember-login",
          JSON.stringify({
            email,
            password,
            role,
          })
        );
      } else {
        localStorage.removeItem("advayam-remember-login");
      }

      if (response.user.role === "admin") {
        window.location.href = "/admin";
      } else {
        window.location.href = "/dashboard";
      }
    } catch (err: any) {
      setError(err.message || "Invalid email or password");
    } finally {
      setIsLoading(false);
    }
  }

  function continueWithGoogle() {
    const backendUrl = process.env.NEXT_PUBLIC_API_URL || "http://localhost:5000";
    window.location.href = `${backendUrl}/auth/google`;
  }

  function continueWithGithub() {
    const backendUrl = process.env.NEXT_PUBLIC_API_URL || "http://localhost:5000";
    window.location.href = `${backendUrl}/auth/github`;
  }

  const loginDisabled = !email.trim() || !password.trim();

  return (
     <div className="min-h-screen w-full bg-black px-4 py-12">
      <div className="mx-auto w-full max-w-[600px] rounded-3xl border border-slate-200 bg-white p-8 sm:p-10 shadow-xl shadow-black/20">
      <div className="text-center">
        <h1 className="font-jakarta text-3xl font-extrabold text-[#202020]">
          Welcome back
        </h1>

        <p className="mt-2 font-jakarta text-xs sm:text-sm font-semibold text-[#202020]/60">
          Login to continue to your account
        </p>
      </div>

      <div className="mt-7">
        <RoleSelector role={role} setRole={setRole} />

        <label className="mt-6 block font-jakarta text-xs font-bold text-slate-800 uppercase tracking-wider">
          Email address
        </label>

        <input
          type="email"
          placeholder="Enter your email"
          value={email}
          onChange={(e) => setEmail(e.target.value)}
          className="mt-2 h-12 w-full rounded-xl border border-slate-200 bg-slate-50/70 px-4 font-jakarta text-sm outline-none focus:bg-white focus:border-[#EA580C] transition-all"
        />

        <div className="mt-5 flex items-center justify-between">
          <label className="font-jakarta text-xs font-bold text-slate-800 uppercase tracking-wider">
            Password
          </label>

          <Link
            href="/auth/forgot-password"
            className="font-jakarta text-xs font-bold text-[#EA580C] hover:underline"
          >
            Forgot password?
          </Link>
        </div>

        <div className="mt-2 flex h-12 items-center rounded-xl border border-slate-200 bg-slate-50/70 px-4 focus-within:bg-white focus-within:border-[#EA580C] transition-all">
          <input
            type={showPassword ? "text" : "password"}
            placeholder="Enter your password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            className="w-full bg-transparent font-jakarta text-sm outline-none"
          />

          <button
            type="button"
            onClick={() => setShowPassword(!showPassword)}
            className="text-slate-400 hover:text-slate-600 transition"
            aria-label={showPassword ? "Hide password" : "Show password"}
          >
            {showPassword ? (
              <EyeOff className="h-4 w-4" />
            ) : (
              <Eye className="h-4 w-4" />
            )}
          </button>
        </div>

        <label className="mt-4 flex items-center gap-2.5 font-jakarta text-xs font-semibold text-slate-600">
          <input
            type="checkbox"
            checked={remember}
            onChange={(e) => setRemember(e.target.checked)}
            className="h-4 w-4 rounded border-slate-300 accent-[#EA580C]"
          />
          Remember me
        </label>

        {error && (
          <p className="mt-4 rounded-xl border border-red-200 bg-red-50 px-4 py-3 font-jakarta text-xs font-bold text-red-600">
            {error}
          </p>
        )}

        <button
          type="button"
          disabled={loginDisabled || isLoading}
          onClick={handleLogin}
          className="mt-6 h-12 w-full rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] font-jakarta text-sm font-bold text-white shadow-md shadow-orange-500/20 transition-all hover:scale-[1.01] active:scale-[0.99] disabled:cursor-not-allowed disabled:opacity-50"
        >
          {isLoading ? "Logging in..." : "Login"}
        </button>

        <div className="my-6 flex items-center gap-3">
          <div className="h-px flex-1 bg-slate-200" />
          <span className="font-jakarta text-xs font-bold text-slate-400 uppercase tracking-wider">
            or continue with
          </span>
          <div className="h-px flex-1 bg-slate-200" />
        </div>

        {/* Compact Icon-Only Google & GitHub Buttons Close Together */}
        <div className="flex items-center justify-center gap-3">
          <SocialLoginButton
            icon={<FcGoogle className="h-6 w-6" />}
            ariaLabel="Sign in with Google"
            onClick={continueWithGoogle}
            className="flex h-12 w-14 items-center justify-center rounded-xl bg-slate-100/90 hover:bg-slate-200/90 transition-all border-0 shadow-none hover:scale-105 active:scale-95"
          />

          <SocialLoginButton
            icon={<FaGithub className="h-6 w-6 text-slate-900" />}
            ariaLabel="Sign in with GitHub"
            onClick={continueWithGithub}
            className="flex h-12 w-14 items-center justify-center rounded-xl bg-slate-100/90 hover:bg-slate-200/90 transition-all border-0 shadow-none hover:scale-105 active:scale-95"
          />
        </div>

        <p className="mt-7 text-center font-jakarta text-xs font-semibold text-slate-600">
          Don’t have an account?{" "}
          <Link href="/auth/register" className="font-bold text-[#EA580C] hover:underline">
            Register
          </Link>
        </p>
      </div>
    </div>
  </div>
  );
}