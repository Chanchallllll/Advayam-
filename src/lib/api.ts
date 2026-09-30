import { getCookie } from "./cookies";

export const API_BASE_URL = (
  process.env.NEXT_PUBLIC_API_URL ||
  "https://advayam-official-website-1.onrender.com"
).replace(/\/+$/, "");

export async function apiFetch<T = any>(
  path: string,
  options: RequestInit = {}
): Promise<T> {
  const url = `${API_BASE_URL}/${path.replace(/^\/+/, "")}`;

  // Get token from cookie (SSO) or localStorage/sessionStorage
  let token: string | null = null;

  if (typeof window !== "undefined") {
    token =
      getCookie("advayam-token") ||
      localStorage.getItem("advayam-token") ||
      sessionStorage.getItem("advayam-token");
  }

  const headers = new Headers(options.headers);

  if (
    !headers.has("Content-Type") &&
    !(options.body instanceof FormData)
  ) {
    headers.set("Content-Type", "application/json");
  }

  if (token) {
    headers.set("Authorization", `Bearer ${token}`);
  }

  const response = await fetch(url, {
    ...options,
    headers,
  });

  if (!response.ok) {
    let errorMessage = "An error occurred";

    try {
      const data = await response.json();

      if (data.fieldErrors) {
        errorMessage = Object.entries(data.fieldErrors)
          .map(
            ([field, errors]) =>
              `${field}: ${(errors as string[]).join(", ")}`
          )
          .join("; ");
      } else {
        errorMessage = data.message || errorMessage;

        if (
          typeof errorMessage === "object" &&
          errorMessage !== null
        ) {
          errorMessage = Object.values(errorMessage)
            .flat()
            .join(", ");
        }
      }
    } catch {
      errorMessage = response.statusText || errorMessage;
    }

    throw new Error(errorMessage);
  }

  // Some successful API responses may have no JSON body.
  try {
    const text = await response.text();

    if (!text) {
      return {} as T;
    }

    return JSON.parse(text) as T;
  } catch {
    return {} as T;
  }
}