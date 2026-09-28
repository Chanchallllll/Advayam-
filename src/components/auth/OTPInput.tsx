"use client";

import { useRef } from "react";

export default function OTPInput({
  value,
  onChange,
}: {
  value: string;
  onChange: (value: string) => void;
}) {
  const refs = useRef<Array<HTMLInputElement | null>>([]);

  const digits = Array.from({ length: 6 }, (_, i) => value[i] || "");

  function updateDigit(index: number, digit: string) {
    if (!/^\d?$/.test(digit)) return;

    const next = [...digits];
    next[index] = digit;

    onChange(next.join(""));

    if (digit && index < 5) {
      refs.current[index + 1]?.focus();
    }
  }

  function handlePaste(text: string) {
    const pasted = text.replace(/\D/g, "").slice(0, 6);
    onChange(pasted);
    refs.current[Math.min(pasted.length, 5)]?.focus();
  }

  return (
    <div className="flex items-center gap-7 max-sm:gap-3">
      {digits.map((digit, index) => (
        <input
          key={index}
          ref={(el) => {
            refs.current[index] = el;
          }}
          value={digit}
          maxLength={1}
          inputMode="numeric"
          onChange={(e) => updateDigit(index, e.target.value)}
          onPaste={(e) => {
            e.preventDefault();
            handlePaste(e.clipboardData.getData("text"));
          }}
          onKeyDown={(e) => {
            if (e.key === "Backspace" && !digits[index] && index > 0) {
              refs.current[index - 1]?.focus();
            }
          }}
          className="h-[58px] w-[64px] rounded-xl border border-[#202020]/15 bg-white text-center font-jakarta text-xl font-black text-[#202020] outline-none focus:border-[#d95b00] focus:ring-1 focus:ring-[#d95b00] max-sm:h-11 max-sm:w-11"
        />
      ))}
    </div>
  );
}