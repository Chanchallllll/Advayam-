import type { RegisterStep } from "@/types/auth";

const steps = [
  { number: 1, label: "Verify Email" },
  { number: 2, label: "Profile Details" },
  { number: 3, label: "Complete" },
];

export default function RegisterStepper({
  currentStep,
}: {
  currentStep: RegisterStep;
}) {
  return (
    <div className="mx-auto flex w-full max-w-[580px] items-center justify-between">
      {steps.map((step, index) => {
        const active = currentStep >= step.number;

        return (
          <div key={step.number} className="flex flex-1 items-center">
            <div className="flex items-center gap-3">
              <span
                className={`flex h-8 w-8 items-center justify-center rounded-full font-jakarta text-sm font-black ${
                  active
                    ? "bg-[#d95b00] text-white"
                    : "bg-white/10 text-white/50"
                }`}
              >
                {step.number}
              </span>

              <span
                className={`font-jakarta text-sm font-bold ${
                  active ? "text-[#d95b00]" : "text-white/50"
                }`}
              >
                {step.label}
              </span>
            </div>

            {index < steps.length - 1 && (
              <div className="mx-4 h-px flex-1 bg-white/15" />
            )}
          </div>
        );
      })}
    </div>
  );
}