"use client";

import React, { createContext, useContext, useState, useEffect } from "react";
import { apiFetch } from "@/lib/api";

import { setCookie, getCookie, eraseCookie } from "@/lib/cookies";

interface User {
  id: string;
  name: string;
  email: string;
  role: string;
}

interface AuthContextType {
  user: User | null;
  token: string | null;
  isLoading: boolean;
  login: (token: string, user: User, remember?: boolean) => void;
  logout: () => void;
}

const AuthContext = createContext<AuthContextType | undefined>(undefined);

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [user, setUser] = useState<User | null>(null);
  const [token, setToken] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(true);

  // Helper to determine the cookie domain dynamically (omitted on localhost unless configured)
  const getCookieDomain = () => {
    if (typeof window === "undefined") return undefined;
    if (process.env.NEXT_PUBLIC_COOKIE_DOMAIN) {
      return process.env.NEXT_PUBLIC_COOKIE_DOMAIN;
    }
    const hostname = window.location.hostname;
    if (hostname === "localhost" || hostname === "127.0.0.1") {
      return undefined;
    }
    return undefined;
  };

  const logout = () => {
    setToken(null);
    setUser(null);
    localStorage.removeItem("advayam-token");
    localStorage.removeItem("advayam-user");
    sessionStorage.removeItem("advayam-token");
    sessionStorage.removeItem("advayam-user");
    
    // Clear cookies for SSO
    const domain = getCookieDomain();
    eraseCookie("advayam-token", domain);
    eraseCookie("advayam-user", domain);
    
    if (typeof window !== "undefined") {
      window.location.href = "/auth/login";
    }
  };

  useEffect(() => {
    async function loadSession() {
      try {
        // Try reading from cookie first (SSO source of truth)
        let savedToken = getCookie("advayam-token");
        let savedUserStr = getCookie("advayam-user");
        let savedUser = savedUserStr ? JSON.parse(savedUserStr) : null;

        // Fallback to local / session storage if cookies not set yet
        if (!savedToken || !savedUser) {
          savedToken = localStorage.getItem("advayam-token");
          const userStr = localStorage.getItem("advayam-user");
          savedUser = userStr ? JSON.parse(userStr) : null;
        }
        if (!savedToken || !savedUser) {
          savedToken = sessionStorage.getItem("advayam-token");
          const userStr = sessionStorage.getItem("advayam-user");
          savedUser = userStr ? JSON.parse(userStr) : null;
        }

        if (savedToken && savedUser) {
          setToken(savedToken);
          setUser(savedUser);

          // Verify token and fetch fresh profile from backend
          try {
            await apiFetch("/auth/profile");
          } catch (e) {
            console.error("Session verification failed, logging out:", e);
            logout();
          }
        }
      } catch (err) {
        console.error("Error loading session:", err);
      } finally {
        setIsLoading(false);
      }
    }
    loadSession();
  }, []);

  const login = (accessToken: string, loggedInUser: User, remember: boolean = true) => {
    setToken(accessToken);
    setUser(loggedInUser);
    
    const domain = getCookieDomain();
    
    if (remember) {
      localStorage.setItem("advayam-token", accessToken);
      localStorage.setItem("advayam-user", JSON.stringify(loggedInUser));
      sessionStorage.removeItem("advayam-token");
      sessionStorage.removeItem("advayam-user");
      
      // Save cookie with 7 days expiration (matching backend JWT lifetime)
      setCookie("advayam-token", accessToken, 7, domain);
      setCookie("advayam-user", JSON.stringify(loggedInUser), 7, domain);
    } else {
      sessionStorage.setItem("advayam-token", accessToken);
      sessionStorage.setItem("advayam-user", JSON.stringify(loggedInUser));
      localStorage.removeItem("advayam-token");
      localStorage.removeItem("advayam-user");
      
      // Save session cookie (expires when browser session ends, i.e. no days param)
      setCookie("advayam-token", accessToken, undefined, domain);
      setCookie("advayam-user", JSON.stringify(loggedInUser), undefined, domain);
    }
  };

  return (
    <AuthContext.Provider value={{ user, token, isLoading, login, logout }}>
      {children}
    </AuthContext.Provider>
  );
}

export function useAuth() {
  const context = useContext(AuthContext);
  if (context === undefined) {
    throw new Error("useAuth must be used within an AuthProvider");
  }
  return context;
}
