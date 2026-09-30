import type { DocumentationDropdown } from "@/data/documentation";
import { ChevronDown } from "lucide-react";

type DocumentationSectionProps = {
  id: string;
  number: string;
  title: string;
  lead: string;
  points?: string[];
  dropdowns?: DocumentationDropdown[];
  note: string;
  openDropdownId?: string | null;
  onDropdownToggle?: (dropdownId: string) => void;
};

export default function DocumentationSection({
  id,
  number,
  title,
  lead,
  points,
  dropdowns,
  note,
  openDropdownId,
  onDropdownToggle,
}: DocumentationSectionProps) {
  const selectedDropdown = dropdowns?.find(
    (dropdown) => openDropdownId === `${id}:${dropdown.title}`,
  );

  return (
    <section
      id={id}
      className="scroll-mt-32"
    >
      <h3 className="font-jakarta text-xl font-black text-white sm:text-2xl">
        <span className="text-[#EA580C]">
          {number}
        </span>{" "}
        {title}
      </h3>

      <p className="font-jakarta mt-4 text-sm font-medium leading-7 text-white/80 sm:text-base">
        {lead}
      </p>

      {dropdowns ? (
        <div className="mt-5 space-y-4">
          <div className="rounded-2xl border border-white/10 bg-white/[0.04] p-4">
            <div className="flex flex-wrap gap-3">
              {dropdowns.map((dropdown) => {
                const isSelected = selectedDropdown?.title === dropdown.title;

                return (
                  <button
                    key={dropdown.title}
                    type="button"
                    aria-expanded={isSelected}
                    onClick={() =>
                      onDropdownToggle?.(`${id}:${dropdown.title}`)
                    }
                    className={`flex items-center gap-2 rounded-xl border px-4 py-3 font-jakarta text-base font-bold transition-colors ${
                      isSelected
                        ? "border-[#EA580C] bg-[#EA580C] text-white"
                        : "border-white/10 bg-white/[0.04] text-white hover:border-white/30 hover:bg-white/[0.08]"
                    }`}
                  >
                    {dropdown.title}
                    <ChevronDown
                      className={`h-5 w-5 transition-transform ${
                        isSelected ? "rotate-180" : ""
                      }`}
                    />
                  </button>
                );
              })}
            </div>
          </div>

          {selectedDropdown && (
            <div className="rounded-2xl border border-white/10 bg-white/[0.04] p-4">
              {selectedDropdown.lead && (
                <p className="whitespace-pre-line font-jakarta text-sm font-medium leading-7 text-white/80">
                  {selectedDropdown.lead}
                </p>
              )}
              <ul className="font-jakarta mt-3 list-disc pl-5 text-sm font-medium leading-7 text-white/70 sm:text-base">
                {selectedDropdown.points.map((point) => (
                  <li key={point}>{point}</li>
                ))}
              </ul>
            </div>
          )}
        </div>
      ) : (
        <ul className="font-jakarta mt-2 list-disc pl-7 text-sm font-medium leading-7 text-white/70 sm:text-base">
          {points?.map((point) => (
            <li key={point}>{point}</li>
          ))}
        </ul>
      )}

      <p className="font-jakarta mt-3 text-sm font-medium leading-7 text-white/60 sm:text-base">
        {note}
      </p>
    </section>
  );
}