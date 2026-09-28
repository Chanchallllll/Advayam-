import type { DocumentationDropdown } from "@/data/documentation";

type DocumentationSectionProps = {
  id: string;
  number: string;
  title: string;
  lead: string;
  points?: string[];
  dropdowns?: DocumentationDropdown[];
  note: string;
};

export default function DocumentationSection({
  id,
  number,
  title,
  lead,
  points,
  dropdowns,
  note,
}: DocumentationSectionProps) {
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
        <div className="mt-5 grid gap-4 md:grid-cols-2">
          {dropdowns.map((dropdown) => (
            <details
              key={dropdown.title}
              name={`${id}-dropdown`}
              className="group rounded-2xl border border-white/10 bg-white/[0.04] p-4 open:bg-white/[0.07]"
            >
              <summary className="flex cursor-pointer list-none items-center justify-between font-jakarta text-base font-bold text-white marker:hidden">
                {dropdown.title}
                <span className="text-xl text-[#EA580C] transition-transform group-open:rotate-45">+</span>
              </summary>
              {dropdown.lead && (
                <p className="mt-4 whitespace-pre-line font-jakarta text-sm font-medium leading-7 text-white/80">
                  {dropdown.lead}
                </p>
              )}
              <ul className="font-jakarta mt-3 list-disc pl-5 text-sm font-medium leading-7 text-white/70 sm:text-base">
                {dropdown.points.map((point) => (
                  <li key={point}>{point}</li>
                ))}
              </ul>
            </details>
          ))}
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