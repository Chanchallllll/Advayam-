export function isValidEmail(email: string) {
  return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);
}

export function isValidOTP(otp: string) {
  return /^\d{6}$/.test(otp);
}

export function validatePassword(password: string) {
  return {
    minLength: password.length >= 8,
    uppercase: /[A-Z]/.test(password),
    lowercase: /[a-z]/.test(password),
    number: /\d/.test(password),
    special: /[^A-Za-z0-9]/.test(password),
  };
}

export function isStrongPassword(password: string) {
  return Object.values(validatePassword(password)).every(Boolean);
}

export function saveRegisterSession(session: {
  role: "developer" | "user";
  emailVerified: boolean;
  profileCompleted: boolean;
}) {
  localStorage.setItem("advayam-register-session", JSON.stringify(session));
}

export function getRegisterSession() {
  const saved = localStorage.getItem("advayam-register-session");
  if (!saved) return null;

  try {
    return JSON.parse(saved);
  } catch {
    return null;
  }
}