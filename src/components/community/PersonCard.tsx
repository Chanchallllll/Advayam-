import type { CommunityPerson } from "@/data/community";

export default function PersonCard({
  person,
  disableHover = false,
}: {
  person: CommunityPerson;
  disableHover?: boolean;
}) {
  return (
    <article
      className={`group flex flex-col items-center justify-center rounded-2xl border border-white/10 bg-white/[0.05] p-6 shadow-sm transition-all duration-300 ${
        disableHover ? "" : "hover:-translate-y-1 hover:border-[#BE5209] hover:shadow-md"
      }`}
    >
      <h3 className="text-center font-jakarta text-base font-extrabold text-white group-hover:text-[#BE5209] transition-colors">
        {person.name}
      </h3>

      <p className="mt-1.5 text-center font-jakarta text-xs font-semibold text-white/60">
        {person.role}
      </p>
    </article>
  );
}