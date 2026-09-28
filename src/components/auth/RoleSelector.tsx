import type { UserRole } from "@/types/auth";

export default function RoleSelector({
  role,
  setRole,
}: {
  role: UserRole;
  setRole: (role: UserRole) => void;
}) {
  return (
    <div className="grid h-14 grid-cols-2 rounded-xl border border-[#202020]/15 bg-white p-1">
      <button
        type="button"
        onClick={() => setRole("developer")}
        className={`rounded-lg font-jakarta text-sm font-black transition ${
          role === "developer"
            ? "bg-[#d95b00] text-white"
            : "text-[#202020]"
        }`}
      >
        As Developer
      </button>

      <button
        type="button"
        onClick={() => setRole("user")}
        className={`rounded-lg font-jakarta text-sm font-black transition ${
          role === "user" ? "bg-[#d95b00] text-white" : "text-[#202020]"
        }`}
      >
        As User
      </button>
    </div>
  );
}