/**
 * Utility functions for client-side cookie management.
 * Used for cross-subdomain Single Sign-On (SSO).
 */

export function setCookie(name: string, value: string, days?: number, domain?: string) {
  let expires = "";
  if (days) {
    const date = new Date();
    date.setTime(date.getTime() + days * 24 * 60 * 60 * 1000);
    expires = "; expires=" + date.toUTCString();
  }
  
  // Build domain parameter if provided
  const domainStr = domain ? `; domain=${domain}` : "";
  
  // In production, enforce Secure and SameSite=Lax (or None if required)
  const isSecure = typeof window !== "undefined" && window.location.protocol === "https:";
  const secureStr = isSecure ? "; Secure" : "";

  document.cookie = `${name}=${encodeURIComponent(value || "")}${expires}; path=/${domainStr}; SameSite=Lax${secureStr}`;
}

export function getCookie(name: string): string | null {
  const nameEQ = name + "=";
  if (typeof document === "undefined") return null;
  
  const ca = document.cookie.split(";");
  for (let i = 0; i < ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) === " ") c = c.substring(1, c.length);
    if (c.indexOf(nameEQ) === 0) {
      return decodeURIComponent(c.substring(nameEQ.length, c.length));
    }
  }
  return null;
}

export function eraseCookie(name: string, domain?: string) {
  const domainStr = domain ? `; domain=${domain}` : "";
  document.cookie = `${name}=; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;${domainStr}; SameSite=Lax`;
}
