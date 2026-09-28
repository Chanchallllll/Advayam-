import type { ReactNode } from "react";

export default function SocialLoginButton({
  icon,
  label,
  ariaLabel,
  onClick,
  className,
}: {
  icon: ReactNode;
  label?: string;
  ariaLabel?: string;
  onClick: () => void;
  className?: string;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      aria-label={ariaLabel || label || "Social login"}
      className={
        className ||
        "flex h-12 w-full items-center justify-center gap-4 rounded-xl border border-[#202020]/15 bg-white font-jakarta text-sm font-black text-[#202020] transition hover:bg-[#f7f7f7]"
      }
    >
      {icon}
      {label && <span>{label}</span>}
    </button>
  );
}